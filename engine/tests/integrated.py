"""Independent small-space references for every integrated inference component."""
import ctypes as C
import itertools
import math
import random
import time
import numpy as np
from native import build, fn, scalar, array

lib = build()
parse = fn(lib, 'ParseCsv', (C.c_void_p, C.c_uint64))
fit = fn(lib, 'TiFit', (C.c_uint32,))
prob = fn(lib, 'TiFamilyProbability', (C.c_uint64, C.c_uint32), C.c_double)
scalar(lib, 'joint_universe').value = 8
rng = random.Random(3205)
space = list(itertools.combinations(range(8), 6))
masks = [sum(1 << i for i in s) for s in space]
indices = [list(s) + [45+i*45+j for i,j in itertools.combinations(s,2)] +
           [2070+math.comb(k,3)+math.comb(j,2)+i for i,j,k in itertools.combinations(s,3)] for s in space]
rows = [list(space[rng.randrange(len(space))]) for _ in range(64)]
rows[:20] = [list(range(6)) for _ in range(20)]

def load(data):
    raw = ('round,n1,n2,n3,n4,n5,n6\n' + ''.join(str(i+1)+','+','.join(str(n+1) for n in s)+'\n' for i,s in enumerate(data))).encode()
    assert parse(raw,len(raw)) == 1

load(rows)
assert fit(60) == 1
coef = np.ctypeslib.as_array(array(lib, 'ti_coef', 4*16260)).reshape(4,16260)
logz = np.ctypeslib.as_array(array(lib,'ti_logz',20)).reshape(4,5)
post = np.ctypeslib.as_array(array(lib,'ti_post',20)).reshape(4,5)
scales = np.array([0.,.5,1.])
for f in range(4):
    energy = np.array([coef[f,ix].sum() for ix in indices])
    z = np.exp(energy[:,None]*scales).sum(axis=0)
    assert np.allclose(logz[f,:3],np.log(z),atol=2e-13)
    observed = sum(energy[space.index(tuple(s))] for s in rows[:60])
    logw = scales*observed-60*np.log(z)-.5*scales**2
    weights = np.exp(logw-logw.max()); weights /= weights.sum()
    assert np.allclose(post[f,:3],weights,atol=2e-12)
    expected = (np.exp(energy[:,None]*scales)/z*weights).sum(axis=1)
    actual = np.array([prob(m,f) for m in masks])
    assert np.allclose(actual,expected,atol=2e-13)
    assert abs(actual.sum()-1)<1e-12

# Exact independent beta-Bernoulli split posterior for sum parity.
parity = [sum(n+1 for n in s)%2 for s in rows[:60]]
def beta_log(seq):
    k=sum(seq);n=len(seq)
    return math.lgamma(k+1)+math.lgamma(n-k+1)-math.lgamma(n+2)
logs=np.array([beta_log(parity[:s])+beta_log(parity[s:]) for s in range(60)])
cp=np.exp(logs-logs.max());cp/=cp.sum()
assert np.allclose(array(lib,'ti_cp_prob',60),cp,atol=2e-13)
rates=np.zeros(8)
for s,w in enumerate(cp):
    for row in rows[s:60]: rates[row]+=w/(60-s)
assert np.allclose(coef[2,:8],np.clip(.5*(rates-.75),-.05,.05),atol=2e-13)

# Marginal Kalman equations, independent of the assembly loop layout.
mean=np.full(8,.75);variance=np.full(8,.75*.25)
for row in rows[:60]:
    prediction_var=variance+.0001
    gain=prediction_var/(prediction_var+.75*.25)
    observation=np.zeros(8);observation[row]=1
    mean+=gain*(observation-mean)
    variance=gain*.75*.25
assert np.allclose(array(lib,'ti_state',8),mean,atol=1e-13)
assert np.allclose(array(lib,'ti_state_var',8),variance,atol=1e-13)
assert np.allclose(coef[3,:8],np.clip(.5*(mean-.75),-.05,.05),atol=1e-13)

# Future rows must not affect any fitted basis, normalizer or posterior.
saved=(coef.copy(),logz.copy(),post.copy())
load(rows[:60]+[list(range(6))]*4)
assert fit(60)==1
for a,b in zip(saved,(coef,logz,post)): assert np.array_equal(a,b)
load(rows)

# Verify stacking + entropic robust optimization against a NumPy implementation;
# an outer row is kept completely out of both optimization phases.
scalar(lib,'ti_folds').value=4
p=np.array([[.10,.03,.02,.02,1/28],[.02,.10,.02,.01,1/28],
            [.08,.02,.03,.02,1/28],[.001,.001,.9,.001,1/28]])
np.ctypeslib.as_array(array(lib,'ti_rows',16*5))[:20]=p.ravel()
scalar(lib,'ti_space_double',C.c_double).value=28
w=np.full(5,.2)
for iteration in range(80):
    mix=p[:3]@w
    q=np.ones(3) if iteration<40 else 1/(28*mix)
    q/=q.sum()
    g=(p[:3]*(q/mix)[:,None]).sum(axis=0)
    w=np.maximum(w*np.exp(.02*g),1e-12);w/=w.sum()
fn(lib,'TiStack')()
assert np.allclose(array(lib,'ti_weights',5),w,atol=1e-12)
np.ctypeslib.as_array(array(lib,'ti_rows',16*5))[15:20]=[.99,.001,.001,.001,.001]
fn(lib,'TiStack')()
assert np.allclose(array(lib,'ti_weights',5),w,atol=1e-12)

# Tensor exactness on a representable constant rank-one tensor.
coef[:]=0
for ix in indices:
    for j in ix[21:]: coef[0,j]=.002
fn(lib,'TiTensor')()
for ix in indices: assert np.allclose(coef[1,ix[21:]],.002,atol=1e-10)

# Real pipeline for both profiles, including full synthetic refits and restoration.
bound=fn(lib,'TiBounds')
scalar(lib,'ti_space',C.c_uint64).value=28
energies=np.linspace(-.7,.7,28)
terms=np.exp(energies)
last_width=math.inf
for count in (0,7,14,21,28):
    scalar(lib,'ti_enum_count',C.c_uint64).value=count
    array(lib,'ti_z',5)[2]=terms[:count].sum()
    bound()
    lo=scalar(lib,'ti_z_lower',C.c_double).value
    hi=scalar(lib,'ti_z_upper',C.c_double).value
    assert lo<=terms.sum()<=hi and hi-lo<last_width
    last_width=hi-lo

analyze=fn(lib,'IntegratedAnalyze',(C.c_uint32,))
sample=fn(lib,'IntegratedSample')
actual_probability=fn(lib,'IntegratedProbability',(C.c_uint64,),C.c_double)
history=array(lib,'history',8192*48,C.c_ubyte)
for mode in (0,1):
    load(rows)
    before=bytes(history)
    oldcoef=list(array(lib,'joint_coef',16260))
    start=time.monotonic()
    assert analyze(mode)==1
    assert bytes(history)==before
    assert list(array(lib,'joint_coef',16260))==oldcoef
    assert scalar(lib,'ti_ready').value==1
    assert scalar(lib,'ti_completed').value==1023
    assert scalar(lib,'ti_null_done').value==(2 if mode==0 else 8)
    assert 0<scalar(lib,'ti_null_p',C.c_double).value<=1
    lo=scalar(lib,'ti_z_lower',C.c_double).value
    hi=scalar(lib,'ti_z_upper',C.c_double).value
    exact=math.exp(array(lib,'ti_logz',20)[17])
    assert lo<=exact<=hi
    assert math.isfinite(scalar(lib,'ti_ais_error',C.c_double).value)
    # Deep samples must follow the actual posterior mixture, not the old model.
    if mode:
        expected=np.zeros(28)
        w=np.array(array(lib,'ti_weights',5))
        for i,m in enumerate(masks): expected[i]=.5/28+.5*(sum(w[f]*prob(m,f) for f in range(4))+w[4]/28)
        assert np.allclose([actual_probability(m) for m in masks],expected,atol=2e-13)
        counts=np.zeros(28)
        for _ in range(12000):
            m=sample(); assert m in masks
            counts[masks.index(m)]+=1
        assert ((counts-12000*expected)**2/(12000*expected)).sum()<85
    print(f'PASS integrated profile {mode}: all ten components, prefix safety, null refits, restoration ({time.monotonic()-start:.2f}s)',flush=True)

# Cancellation restores history and legacy model and never marks partial success.
before=bytes(history);oldcoef=list(array(lib,'joint_coef',16260))
scalar(lib,'cancel_flag').value=1
assert analyze(0)==0
assert scalar(lib,'ti_ready').value==0
assert bytes(history)==before and list(array(lib,'joint_coef',16260))==oldcoef
scalar(lib,'cancel_flag').value=0
print('PASS NumPy normalized posterior, beta changepoints, Kalman, tensor, stacking/DRO, exact sums, sampled distribution and cancellation')
