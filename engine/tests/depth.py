"""Independent numerical references and finite-null checks for depth assembly."""
import ctypes as C
import itertools
import math
import random
import numpy as np
from native import build, fn, scalar, array

lib=build()
def vec(name,n): return np.ctypeslib.as_array(array(lib,name,n))
def val(name): return scalar(lib,name,C.c_double).value
parse=fn(lib,'ParseCsv',(C.c_void_p,C.c_uint64))
fit=fn(lib,'TiFit',(C.c_uint32,))
scalar(lib,'joint_universe').value=8
space=list(itertools.combinations(range(8),6))
rng=random.Random(82942)
rows=[space[rng.randrange(28)] for _ in range(80)]
rows[:24]=[space[0]]*24
def load(data):
    raw=('round,n1,n2,n3,n4,n5,n6\n'+''.join(str(i+1)+','+','.join(str(j+1) for j in s)+'\n' for i,s in enumerate(data))).encode()
    assert parse(raw,len(raw))==1
load(rows)
assert fit(60)==1

# A separate vectorized discounted beta/composite-score implementation.
counts=np.zeros((4,8));mass=np.zeros(4);scores=np.zeros(4)
decay=np.exp(-np.log(2)/np.array([16,64,256,np.inf]))
kalman=np.full(8,.75);variance=np.full(8,.1875)
for row in rows[:60]:
    y=np.zeros(8);y[list(row)]=1
    predictive=(counts+60*.75)/(mass[:,None]+60)
    scores+=(y*np.log(predictive)+(1-y)*np.log1p(-predictive)).mean(axis=1)
    counts=decay[:,None]*counts+y
    mass=decay*mass+1
    gain=(variance+.0001)/(variance+.0001+.1875)
    kalman+=gain*(y-kalman);variance=gain*.1875
w=np.exp(scores-scores.max());w/=w.sum()
rates=.5*(w@((counts+60*.75)/(mass[:,None]+60))+kalman)
assert np.allclose(vec('depth_scores',4),scores,atol=2e-13)
assert np.allclose(vec('depth_scale_weights',4),w,atol=2e-13)
assert np.allclose(vec('depth_raw_rates',8),rates,atol=2e-13)
coef=vec('ti_coef',4*16260).reshape(4,16260)
graph=np.zeros((8,8))
for i,j in itertools.combinations(range(8),2):graph[i,j]=graph[j,i]=abs(coef[0,45+i*45+j])
lap=np.diag(graph.sum(axis=1))-graph
x=np.linalg.solve(np.eye(8)+lap,rates-.75)
assert np.allclose(vec('depth_rates',8),x+.75,atol=2e-13)
assert np.isclose(x.sum(),(rates-.75).sum(),atol=2e-13)
assert x@lap@x <= (rates-.75)@lap@(rates-.75)+1e-13
assert val('depth_graph_residual')<1e-10
assert np.allclose(coef[3,:8],np.clip(.5*x,-.05,.05),atol=2e-13)

# Analytic finite-space Fisher curvature and first partition derivative.
indices=[list(s)+[45+i*45+j for i,j in itertools.combinations(s,2)]+
         [2070+math.comb(k,3)+math.comb(j,2)+i for i,j,k in itertools.combinations(s,3)] for s in space]
means=vec('depth_means',20).reshape(4,5)
fisher=vec('depth_variances',20).reshape(4,5)
for f in range(4):
    energy=np.array([coef[f,ix].sum() for ix in indices])
    for a,scale in enumerate([0,.5,1]):
        q=np.exp(scale*energy);q/=q.sum()
        mean=q@energy;var=q@((energy-mean)**2)
        assert math.isclose(means[f,a],mean,abs_tol=2e-13)
        assert math.isclose(fisher[f,a],var,abs_tol=2e-13)
        h=1e-4
        logz=lambda t:np.log(np.exp(t*energy).sum())
        curvature=(logz(scale+h)-2*logz(scale)+logz(scale-h))/h**2
        assert abs(curvature-var)<3e-7

# Numerical auditing must reject corrupted normalization, NaN and curvature.
audit=fn(lib,'DepthAuditFamily',(C.c_uint32,))
post=vec('ti_post',20);old=post.copy()
for bad in (float('nan'),float('inf'),-1.,2.):
    post[0]=bad;assert audit(0)==0
post[:]=old;assert audit(0)==1
logz=vec('ti_logz',20);oldz=logz.copy();logz[0]+=1
assert audit(0)==0;logz[:]=oldz

# Compensated summation retains tiny mass lost by ordinary double addition.
fn(lib,'DepthMomentsReset')();vec('ti_z',5)[:]=0
accumulate=fn(lib,'DepthAccumulate',(C.c_uint32,C.c_double,C.c_double))
# The generic native adapter places doubles in xmm0/xmm1 (assembly internal ABI).
terms=[1.]+[1e-16]*10000
for term in terms: accumulate(0,term,.5)
assert abs(vec('ti_z',5)[0]-math.fsum(terms))<1e-15

# Future changes cannot affect fitted rates, graph, normalizers or moments.
assert fit(60)==1
before=[vec(n,k).copy() for n,k in [('depth_rates',8),('ti_logz',20),('depth_means',20),('depth_variances',20)]]
load(rows[:60]+[space[-1]]*20);assert fit(60)==1
for expected,(n,k) in zip(before,[('depth_rates',8),('ti_logz',20),('depth_means',20),('depth_variances',20)]):
    assert np.array_equal(expected,vec(n,k))

# Independent causal expert-mixture/e-process calculation.
seq=fn(lib,'DepthSequential')
p=np.array([[.03,.05,.01,.06,1/28],[.06,.02,.03,.01,1/28],
            [.02,.04,.07,.01,1/28],[.01,.01,.03,.08,1/28]])
scalar(lib,'ti_folds').value=4
scalar(lib,'ti_space_double',C.c_double).value=28
vec('ti_rows',80)[:20]=p.ravel();vec('ti_weights',5)[:]=.2
w=np.full(5,.2);loge=np.zeros(5);gains=[];peak=0;cusum=0;maxcusum=0;saved=[]
for row in p:
    saved.append(w.copy());mix=row@w
    loge[:4]+=np.log(row[:4]*28/1.000001)
    gain=np.log((.5+.5*mix*28)/1.000001);gains.append(gain)
    loge[4]+=gain;peak=max(peak,loge.max())
    cusum=max(0,cusum-gain);maxcusum=max(maxcusum,cusum)
    w=.98*w*row/mix+.02/5
assert seq()==1
assert np.allclose(vec('depth_loge',5),loge,atol=1e-13)
assert np.allclose(vec('depth_predict_weights',20).reshape(4,5),saved,atol=1e-13)
assert np.allclose(vec('depth_gains',4),gains,atol=1e-13)
assert math.isclose(val('depth_anytime_p'),min(1,5*np.exp(-peak)),abs_tol=1e-13)
assert math.isclose(val('depth_cusum'),maxcusum,abs_tol=1e-13)
lastmix=p[-1].mean()
ablation=max(abs(np.log((lastmix-.2*p[-1,i])/.8/lastmix)) for i in range(4))
assert math.isclose(val('depth_ablation'),ablation,abs_tol=1e-13)
before=vec('depth_predict_weights',20).copy()
vec('ti_rows',80)[15:20]=[.8,.01,.01,.01,1/28]
assert seq()==1 and np.array_equal(before,vec('depth_predict_weights',20))
vec('ti_rows',80)[0]=float('nan');assert seq()==0

# Exhaustive null experiment: all 2^10 fair binary sequences. Each expert is a
# normalized distribution. Expected e <=1 and family/time rejection rate <=.05.
scalar(lib,'ti_folds').value=10
scalar(lib,'ti_space_double',C.c_double).value=2
pred=np.array([.95,.75,.25,.05,.5]);total_e=0;rejects=0
for bits in itertools.product([0,1],repeat=10):
    q=np.array([pred if bit else 1-pred for bit in bits])
    vec('ti_rows',80)[:50]=q.ravel()
    assert seq()==1
    total_e+=math.exp(val('depth_online_loge'))
    rejects+=val('depth_anytime_p')<=.05
assert math.isclose(total_e/1024,1/1.000001**10,abs_tol=1e-12)
assert rejects/1024<=.05

# Stationary bootstrap variance has a closed-form conditional reference.
boot=fn(lib,'DepthBootstrap')
scalar(lib,'ti_folds').value=16;scalar(lib,'ti_profile').value=1
scalar(lib,'rng_mode').value=1;scalar(lib,'rng_state',C.c_uint64).value=74382
g=np.sin(np.arange(16)*.7)*.2+np.arange(16)*.005
vec('depth_gains',16)[:]=g
assert boot()==1 and scalar(lib,'depth_boot_done').value==4096
b=vec('depth_bootstrap',4096)
centered=g-g.mean();gamma=np.array([centered@np.roll(centered,k)/16 for k in range(16)])
expected_var=(16*gamma[0]+2*sum((16-k)*.75**k*gamma[k] for k in range(1,16)))/16**2
assert abs(b.mean()-g.mean())<5*math.sqrt(expected_var/4096)
assert abs(b.var()/expected_var-1)<.10
assert val('depth_boot_low')==b[4096//40] and val('depth_boot_high')==b[4095-4096//40]
vec('depth_gains',16)[:]=.1;assert boot()==1
assert math.isclose(val('depth_boot_low'),.1,abs_tol=1e-14)
scalar(lib,'ti_folds').value=7;assert boot()==1
assert scalar(lib,'depth_boot_done').value==0

# Both complete product layers run expanded folds and bootstrap by default;
# cancellation restores all history and makes integrated state unavailable.
load(rows)
scalar(lib,'joint_universe').value=8
analyze=fn(lib,'IntegratedAnalyze',(C.c_uint32,))
history=array(lib,'history',8192*48,C.c_ubyte);original=bytes(history)
for mode in (0,1):
    assert analyze(mode)==1 and bytes(history)==original
    assert scalar(lib,'ti_folds').value==(8 if mode==0 else 16)
    assert scalar(lib,'depth_boot_done').value==(512 if mode==0 else 4096)
    assert val('depth_graph_residual')<1e-10
    assert 0<val('depth_anytime_p')<=1
    if mode==0:assert val('ti_alpha')==0
scalar(lib,'cancel_flag').value=1
assert analyze(0)==0 and scalar(lib,'ti_ready').value==0 and bytes(history)==original
print('PASS multiscale/graph NumPy reference, Fisher curvature, compensation, corruption rejection, prefix isolation, exact finite-null e-process control, stationary-bootstrap moments and both default pipelines')
