"""Independent invariants against the shipped assembly; no model reimplementation."""
import ctypes as C
import hashlib, math, random
import numpy as np
from native import build,fn,scalar,array
lib=build()
parse=fn(lib,'ParseCsv',(C.c_void_p,C.c_uint64))
rebuild=fn(lib,'RebuildStats',(C.c_uint32,))
prepare=fn(lib,'PrepareModels')
def csv(rows):
    return ('round,n1,n2,n3,n4,n5,n6\n'+''.join(f'{i+1},'+','.join(map(str,row))+'\n' for i,row in enumerate(rows))).encode()
def load(rows):
    raw=csv(rows);assert parse(raw,len(raw))==1
rng=random.Random(77)
rows=[sorted(rng.sample(range(1,46),6)) for _ in range(120)]
load(rows)
# Transactional parsing: every failure preserves the committed dataset.
hash_before=bytes(array(lib,'hash_dataset',32,C.c_ubyte))
for raw in [b'1,1,1,3,4,5,6\n',b'1,0,2,3,4,5,6',b'1,1,2,3,4,5,46',
            b'1,1,2,3,4,5,6,6',b'1,1,2,3,4,5,6,7,8',b'1,1,2,3,4,5,6x',
            b'1,2025-02-29,1,2,3,4,5,6',b'2147483648,1,2,3,4,5,6',
            b'1,1,2,3,4,5,6\n3,1,2,3,4,5,6',b'1,-1,2,3,4,5,6']:
    assert parse(raw,len(raw))==0,raw
    assert scalar(lib,'history_count').value==120
    assert bytes(array(lib,'hash_dataset',32,C.c_ubyte))==hash_before
# Canonical sorting yields the same hash regardless of row or ball order.
rev=b'round,n1,n2,n3,n4,n5,n6\r\n'+b''.join((str(i+1)+','+','.join(map(str,reversed(rows[i])))+'\r\n').encode() for i in range(119,-1,-1))
assert parse(rev,len(rev))==1
assert bytes(array(lib,'hash_dataset',32,C.c_ubyte))==hash_before
# Analysis range activation is stronger than fold selection: the active history
# itself is sliced, so final fitting and next-round target cannot read later rows.
activate=fn(lib,'ActivateHistoryRange',(C.c_uint32,C.c_uint32))
assert activate(0,59)==0
assert activate(1,59)==1
assert scalar(lib,'history_count').value==59
assert fn(lib,'ResolveBacktestRange')()==0
assert activate(1,60)==1
assert scalar(lib,'history_count').value==60
assert scalar(lib,'last_round').value==60
assert scalar(lib,'backtest_start_round').value==1
assert scalar(lib,'backtest_end_round').value==60
assert fn(lib,'ResolveBacktestRange')()==1
assert scalar(lib,'backtest_start_index').value==60
assert scalar(lib,'backtest_end_index').value==60
assert activate(31,120)==1
assert scalar(lib,'history_count').value==90
assert scalar(lib,'last_round').value==120
assert fn(lib,'ResolveBacktestRange')()==1
assert scalar(lib,'backtest_start_index').value==60
assert scalar(lib,'backtest_end_index').value==90
assert activate(1,120)==1
assert scalar(lib,'history_count').value==120
assert scalar(lib,'validation_stop').value==90
rebuild(60)
assert sum(array(lib,'freq_total',45,C.c_uint32))==360
assert sum(array(lib,'triple_counts',14190,C.c_uint32))==1200
assert list(array(lib,'freq_total',45,C.c_uint32))==[sum(n in row for row in rows[:60]) for n in range(1,46)]
assert math.isclose(array(lib,'feature_mean',8)[0],np.mean([sum(row) for row in rows[:60]]),rel_tol=1e-13)
# Number-theoretic feature must count primes including 2 and 43, excluding 1.
extract=fn(lib,'ExtractFeatures',(C.c_uint64,))
extract(sum(1<<(n-1) for n in [1,2,3,5,41,43]));assert array(lib,'feature_buf',8)[5]==5
# Real assembly matrix inversion versus NumPy.
inverse=fn(lib,'MatrixInverse',(C.c_void_p,C.c_uint32,C.c_void_p))
m=np.array([[4.,1.,.5],[1.,3.,.25],[.5,.25,2.]])
out=np.zeros((3,3));assert inverse(m.ctypes.data,3,out.ctypes.data)==1
assert np.allclose(out,np.linalg.inv(m),atol=1e-11)
q=(C.c_double*4)();p=(C.c_double*4)(.001,.01,.05,.2)
fn(lib,'BHCorrection',(C.c_void_p,C.c_uint32,C.c_void_p))(p,4,q)
assert np.allclose(list(q),[.004,.02,1/15,.2])
# Prefix invariance includes counterfactual-world rebuilding, a leakage risk.
def field_prefix():
    rebuild(60);assert prepare()==1
    fn(lib,'BuildNormalization')();assert fn(lib,'ScanField')()==1
    assert scalar(lib,'scan_count').value==8145060
    assert fn(lib,'BuildGeometry')()==1
    assert fn(lib,'BuildWorlds')()==1
    fn(lib,'BuildThermal')()
    masks=list(array(lib,'final_masks',512,C.c_uint64))
    assert len(set(masks))==512
    assert all(m.bit_count()==6 and m>>45==0 for m in masks)
    probs=list(array(lib,'final_prob',512))
    assert all(math.isfinite(p) and p>=0 for p in probs)
    assert abs(sum(probs)-1)<1e-12
    assert abs(sum(array(lib,'mixture_marginals',45))-6)<1e-12
    assert scalar(lib,'draw_count').value==60
    return masks,probs,list(array(lib,'world_summary',512*4))
x=field_prefix()
load(rows[:60]+[sorted(rng.sample(range(1,46),6)) for _ in range(60)])
y=field_prefix();assert x==y,'future rows leaked into prefix models/worlds'
# Full walk-forward, final restoration and reproducible policy hash.
load(rows[:90]);assert fn(lib,'AnalyzeOracle')()==1
assert scalar(lib,'validation_count').value==30
assert scalar(lib,'draw_count').value==90
assert scalar(lib,'rho',C.c_double).value==1.0
assert scalar(lib,'confirmation_count').value==0
first=bytes(array(lib,'hash_field',32,C.c_ubyte))
assert fn(lib,'AnalyzeOracle')()==1
assert bytes(array(lib,'hash_field',32,C.c_ubyte))==first
# Exercise reserved-holdout rejection separately with a deliberately concentrated
# model; confirmation must reset the policy rather than promote it on training.
load(rows)
scalar(lib,'validation_stop').value=90
scalar(lib,'rng_mode').value=1
scalar(lib,'evidence',C.c_double).value=1
scalar(lib,'rho',C.c_double).value=.5
array(lib,'model_weights',12)[0]=1
scalar(lib,'validation_baseline_brier',C.c_double).value=(6/45)*(1-6/45)
assert fn(lib,'ConfirmPolicy')()==1
assert scalar(lib,'confirmation_count').value==30
assert scalar(lib,'confirmation_status').value==2
assert scalar(lib,'rho',C.c_double).value==1
# A deliberately non-lottery sequence must exercise the positive gate as well.
# This verifies policy plumbing, not predictive performance on real draws.
load([[1,2,3,4,5,6] for _ in range(120)])
assert fn(lib,'AnalyzeOracle')()==1
assert scalar(lib,'validation_count').value==30
assert scalar(lib,'confirmation_count').value==30
assert scalar(lib,'confirmation_status').value==1
assert .5 <= scalar(lib,'rho',C.c_double).value < 1
assert abs(sum(array(lib,'model_weights',12))-1)<1e-12
# OS entropy path and diversified bundles, plus cancellation fail-closed.
scalar(lib,'rng_mode').value=0
assert fn(lib,'GenerateTickets',(C.c_uint32,))(10)==1
masks=list(array(lib,'session_masks',10,C.c_uint64))
assert len(set(masks))==10 and all(m.bit_count()==6 and m>>45==0 for m in masks)
size=fn(lib,'BuildReport')();assert 0<size<2097152
report=bytes(array(lib,'report_buffer',size,C.c_ubyte)).decode('utf-8-sig')
assert 'Candidate-field SHA-256' in report and 'Independent policy confirmation' in report and 'Backtest targets' in report
scalar(lib,'cancel_flag').value=1
assert fn(lib,'AnalyzeOracle')()==0
assert scalar(lib,'field_ready').value==0 and scalar(lib,'rho',C.c_double).value==1
print('PASS independent parser, statistics, linalg, FDR, 8,145,060 scan, worlds, prefix isolation, rolling validation, confirmation, reproducibility, sampling, report and cancellation')
