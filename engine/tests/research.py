"""Independent finite-space probability calculations and research invariants."""
import ctypes as C,itertools,math,random,time
import numpy as np
from native import build,fn,scalar,array
lib=build()
parse=fn(lib,'ParseCsv',(C.c_void_p,C.c_uint64))
def load(rows):
 data=('round,n1,n2,n3,n4,n5,n6\n'+''.join(str(i+1)+','+','.join(map(str,r))+'\n' for i,r in enumerate(rows))).encode()
 assert parse(data,len(data))==1
fit=fn(lib,'JointFit',(C.c_uint32,C.c_uint32))
enumerate_=fn(lib,'JointEnumerate')
logp=fn(lib,'JointLogProbability',(C.c_uint64,),C.c_double)
forecast=fn(lib,'JointForecastLog',(C.c_uint64,),C.c_double)
rng=random.Random(372)
scalar(lib,'joint_universe').value=8
space=list(itertools.combinations(range(8),6))
def idx(c):return list(c)+[45+i*45+j for i,j in itertools.combinations(c,2)]+[2070+math.comb(k,3)+math.comb(j,2)+i for i,j,k in itertools.combinations(c,3)]
indices=[idx(c) for c in space];masks=[sum(1<<i for i in c) for c in space]
coef=array(lib,'joint_coef',16260)
for i in set(itertools.chain.from_iterable(indices)):coef[i]=rng.uniform(-.02,.02)
assert enumerate_()==1
energy=np.array([sum(coef[i] for i in ix) for ix in indices]);prob=np.exp(energy);prob/=prob.sum()
assert np.allclose([math.exp(logp(m)) for m in masks],prob,atol=1e-13)
mom=np.zeros(16260)
for ix,p in zip(indices,prob):mom[ix]+=p
assert np.allclose(array(lib,'joint_moment',16260),mom,atol=1e-13)
assert math.isclose(sum(math.exp(forecast(m)) for m in masks),1,abs_tol=1e-13)
# Balanced complete finite space must shrink exactly to the uniform model.
rows=[[i+1 for i in c] for c in space]*3
load(rows);assert fit(len(rows),0)==1
assert max(abs(x) for x in coef)==0
assert np.allclose([math.exp(logp(m)) for m in masks],1/28)
# Trained coefficients respect bounds and future rows cannot affect a prefix.
rows=[sorted(rng.sample(range(1,9),6)) for _ in range(72)]
load(rows);assert fit(60,0)==1
before=list(coef),list(array(lib,'joint_moment',16260))
load(rows[:60]+[[1,2,3,4,5,6]]*12);assert fit(60,0)==1
assert before==(list(coef),list(array(lib,'joint_moment',16260)))
for start,end,cap in [(0,45,.05),(45,2070,.02),(2070,16260,.01)]:assert max(abs(coef[i]) for i in range(start,end))<=cap
# Nested outer row t is unchanged when that row or later truth changes fitting.
select=fn(lib,'JointSelectSetting',(C.c_uint32,))
assert select(64)==1
saved=list(coef);chosen=scalar(lib,'research_chosen').value
load(rows[:64]+[[1,2,3,4,5,6]]*8);assert select(64)==1
assert saved==list(coef) and scalar(lib,'research_chosen').value==chosen
assert fn(lib,'JointNestedValidation')()==1
assert scalar(lib,'research_outer_count').value==8
outer=array(lib,'research_outer_rows',8*48,C.c_ubyte)
# E-process is the product of already-normalized forecast / uniform ratios.
import struct
recs=[struct.unpack_from('<IIddddd',bytes(outer),48*i) for i in range(8)]
cumulative=0.
for training,choice,loss,uniform,loge,brier,_ in recs:
 cumulative+=uniform-loss
 assert math.isclose(cumulative,loge,abs_tol=1e-12)
 assert 0<=brier<=1
assert fn(lib,'JointUncertainty')()==1
assert 8<=scalar(lib,'research_world_count').value<=32
assert scalar(lib,'research_mc_error',C.c_double).value>=0
assert abs(sum(array(lib,'joint_moment',8))-6)<1e-12
# Cancellation fails closed, including an exact-space normalization pass.
scalar(lib,'cancel_flag').value=1
assert enumerate_()==0 and fit(60,0)==0
scalar(lib,'cancel_flag').value=0
# Full 45-ball domain: all zero coefficients give exact binomial support and p.
scalar(lib,'joint_universe').value=45
for i in range(16260):coef[i]=0
start=time.monotonic();assert enumerate_()==1
assert scalar(lib,'joint_enumerated',C.c_uint64).value==8145060
assert abs(sum(array(lib,'joint_moment',45))-6)<1e-12
assert math.isclose(math.exp(logp(63)),1/8145060,rel_tol=1e-12)
print(f'PASS exact 8145060 support ({time.monotonic()-start:.2f}s), NumPy joint probabilities/moments, shrinkage, bounds, nested prefix isolation, log-e replay, adaptive uncertainty and cancellation')

# Full pipeline ablation at a zero-holdout origin: 13 actual complete executions.
rows=[sorted(rng.sample(range(1,46),6)) for _ in range(60)]
load(rows)
start=time.monotonic()
assert fn(lib,'AnalyzeResearch')()==1
assert scalar(lib,'research_ablation_count').value==13
assert scalar(lib,'research_ready').value==1
assert scalar(lib,'research_outer_count').value==0
assert fn(lib,'BuildReport')()>0
assert b'FULL PIPELINE ABLATION' in bytes(array(lib,'report_buffer',2097152,C.c_ubyte))
print(f'PASS full research suite on 60 rows ({time.monotonic()-start:.1f}s)')
