"""Exercise the DLL directly, without launching or loading the Fortuna GUI."""
import ctypes as C
import pathlib,threading,time,random
root=pathlib.Path(__file__).resolve().parents[1]
dll=C.WinDLL(str(root/'build/FortunaOracle.dll'))
def api(name,args=(),result=C.c_int32):
 f=getattr(dll,'FortunaOracle'+name);f.argtypes=list(args);f.restype=result;return f
init=api('Initialize');load=api('LoadCsv',(C.c_void_p,C.c_uint64));set_range=api('SetBacktestRange',(C.c_uint32,C.c_uint32))
analyze=api('Analyze');cancel=api('Cancel')
generate=api('Generate',(C.c_uint32,C.c_void_p,C.c_uint32))
snapshot=api('GetSnapshot',(C.c_void_p,C.c_uint32));report=api('GetReport',(C.c_void_p,C.c_uint32))
progress=api('GetProgress',(C.c_void_p,C.c_uint32))
assert api('GetAbiVersion')()==0x10000
assert api('GetEngineVersion')()==0x10600
out=(C.c_uint64*10)()
assert generate(1,out,10)==-4
assert init()==0 and init()==0
assert generate(0,out,10)==-1
assert generate(11,out,10)==-1
assert generate(5,out,4)==-1
assert generate(1,None,10)==-1
assert generate(5,out,10)==0
assert len(set(out[:5]))==5 and all(x.bit_count()==6 and x>>45==0 for x in out[:5])
s=(C.c_ubyte*272)();assert snapshot(s,271)==-1;assert snapshot(s,272)==0
assert int.from_bytes(bytes(s[:4]),'little')==272
assert report(None,0)==-4
raw=(root/'data/SYNTHETIC-example.csv').read_bytes()
assert load(raw,len(raw))==0
assert snapshot(s,272)==0
old_hash=bytes(s[176:208])
bad=b'1,1,1,2,3,4,5\n';assert load(bad,len(bad))==-3
assert snapshot(s,272)==0 and bytes(s[176:208])==old_hash
assert analyze()==0
size=report(None,0);assert 0<size<2097152
small=C.create_string_buffer(b'CANARY',7)
assert report(small,7)==size and small.raw==b'CANARY\0'
buf=C.create_string_buffer(size);assert report(buf,size)==size
assert b'Candidate-field SHA-256' in buf.raw
# Serialize operations, expose safe progress/cancel while analysis holds the lock.
rng=random.Random(77)
raw=('round,n1,n2,n3,n4,n5,n6\n'+''.join(str(i)+','+','.join(map(str,sorted(rng.sample(range(1,46),6))))+'\n' for i in range(1,121))).encode()
assert load(raw,len(raw))==0
assert set_range(61,119)==-1
assert set_range(61,120)==0
assert set_range(0,0)==0
result=[];thread=threading.Thread(target=lambda:result.append(analyze()));thread.start()
state=(C.c_uint32*4)();busy=False
for _ in range(1000):
 if snapshot(s,272)==-2:
  busy=True;break
 time.sleep(.001)
assert busy
assert progress(state,16)==0
assert generate(1,out,10)==-2
assert init()==-2
assert cancel()==0
thread.join(30);assert not thread.is_alive() and result==[-6],result
assert snapshot(s,272)==0
assert int.from_bytes(bytes(s[12:16]),'little')&1==0
assert C.c_double.from_buffer(s,160).value==1
assert generate(1,out,10)==0
# No API leaks pointers to private memory; every result is copied into caller buffers.
print('PASS standalone DLL ABI, bounds, transactional state, report size negotiation, busy isolation, cancellation and generation')
