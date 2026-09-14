"""Measured compute time, not a promised one-hour duration. Optional manual run."""
import ctypes as C
import random
import time
from native import build,fn,scalar

lib=build();parse=fn(lib,'ParseCsv',(C.c_void_p,C.c_uint64))
rng=random.Random(901)
raw=('round,n1,n2,n3,n4,n5,n6\n'+''.join(str(i+1)+','+','.join(map(str,sorted(rng.sample(range(1,46),6))))+'\n' for i in range(60))).encode()
assert parse(raw,len(raw))==1
for mode in (0,1):
    start=time.monotonic()
    assert fn(lib,'IntegratedAnalyze',(C.c_uint32,))(mode)==1
    print(f'Integrated layer only, 60 rows, profile={mode}: {time.monotonic()-start:.3f}s; mask={scalar(lib,"ti_completed").value}',flush=True)
