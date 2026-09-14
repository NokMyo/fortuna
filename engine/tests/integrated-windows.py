"""Production Windows default routes: real 45C6, no authentication bypass."""
import ctypes as C
import math
import pathlib
import random
import struct
import threading
import time

root=pathlib.Path(__file__).resolve().parents[1]
dll=C.WinDLL(str(root/'build/FortunaOracle.dll'))
def api(name,args=()):
    f=getattr(dll,'FortunaOracle'+name);f.argtypes=list(args);f.restype=C.c_int32;return f
init=api('Initialize');assert init()==0
load=api('LoadCsv',(C.c_void_p,C.c_uint64))
normal=api('AnalyzeAdvanced')
deep=api('AnalyzeResearchAdvanced',(C.c_uint32,))
generate=api('GenerateIntegrated',(C.c_uint32,C.c_void_p,C.c_uint32))
uniform=api('GenerateUniform',(C.c_uint32,C.c_void_p,C.c_uint32))
snapshot=api('GetIntegratedSnapshot',(C.c_void_p,C.c_uint32))
prob=api('GetIntegratedProbability',(C.c_uint64,C.c_void_p))
report=api('GetReport',(C.c_void_p,C.c_uint32))
out=(C.c_uint64*10)();state=(C.c_ubyte*64)();p=C.c_double()
assert generate(1,out,10)==-4
assert snapshot(state,63)==-1 and prob(0,C.byref(p))==-1
rng=random.Random(8392)
raw=('round,n1,n2,n3,n4,n5,n6\n'+''.join(str(i+1)+','+','.join(map(str,sorted(rng.sample(range(1,46),6))))+'\n' for i in range(60))).encode()
assert load(raw,len(raw))==0
for mode in (0,1):
    start=time.monotonic()
    assert (normal() if mode==0 else deep(0))==0
    assert snapshot(state,64)==0
    fields=struct.unpack('<8I4d',bytes(state))
    assert fields[:6]==(64,1,mode,1023,0,2 if mode==0 else 8),fields
    assert all(math.isfinite(x) for x in fields[8:])
    assert generate(10,out,10)==0
    assert all(x.bit_count()==6 and x>>45==0 for x in out)
    assert prob(out[0],C.byref(p))==0 and 0<p.value<1
    if mode==0: assert math.isclose(p.value,1/8145060,rel_tol=1e-12)
    before=bytes(state)
    assert uniform(10,out,10)==0 and snapshot(state,64)==0 and bytes(state)==before
    assert generate(1,out,10)==0
    size=report(None,0);assert 0<size<2097152
    buf=C.create_string_buffer(size);assert report(buf,size)==size
    assert b'INTEGRATED TEN-FUNCTION INFERENCE' in buf.raw
    print(f'PASS Windows integrated mode={mode} default 45C6, ten-feature snapshot, actual generation/probability/report and cache ({time.monotonic()-start:.2f}s)',flush=True)
# Cancellation must prevent reuse; do not wait for another complete heavy analysis.
result=[];thread=threading.Thread(target=lambda:result.append(normal()));thread.start()
for _ in range(10000):
    if snapshot(state,64)==-2:break
    time.sleep(.001)
else:raise AssertionError('Analysis did not lock')
assert api('Cancel')()==0
thread.join(30);assert not thread.is_alive() and result==[-6],result
assert generate(1,out,10)==-4
assert load(raw,len(raw))==0 and generate(1,out,10)==-4
print('PASS integrated cancellation, busy isolation and CSV invalidation')
