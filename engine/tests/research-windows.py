"""Public research ABI and persistent pre-outcome forecast verification."""
import ctypes as C, pathlib,random,struct,math,hashlib,tempfile
root=pathlib.Path(__file__).resolve().parents[1]
dll=C.WinDLL(str(root/'build/FortunaOracle.dll'))
def api(name,args=(),restype=C.c_int32):
 f=getattr(dll,'FortunaOracle'+name);f.argtypes=list(args);f.restype=restype;return f
assert api('Initialize')()==0
load=api('LoadCsv',(C.c_void_p,C.c_uint64));research=api('AnalyzeResearch',(C.c_uint32,))
joint=api('GetJointProbability',(C.c_uint64,C.c_void_p));ledger=api('ForecastLedgerW',(C.c_wchar_p,C.c_uint32))
snapshot=api('GetResearchSnapshot',(C.c_void_p,C.c_uint32));generate=api('GenerateResearch',(C.c_uint32,C.c_void_p,C.c_uint32))
report=api('GetReport',(C.c_void_p,C.c_uint32))
pair=(C.c_double*2)();state=(C.c_ubyte*128)()
assert research(2)==-1 and joint(63,pair)==-4
assert joint(0,pair)==-1 and joint(1<<45,pair)==-1
assert snapshot(state,127)==-1
rng=random.Random(827)
rows=[sorted(rng.sample(range(1,46),6)) for _ in range(61)]
def csv(rows):return ('round,n1,n2,n3,n4,n5,n6\n'+''.join(str(i+2001)+','+','.join(map(str,r))+'\n' for i,r in enumerate(rows))).encode()
raw=csv(rows[:60]);assert load(raw,len(raw))==0
assert research(0)==0
assert snapshot(state,128)==0
assert struct.unpack_from('<IIII',state)==(128,1,13,0)
assert joint(63,pair)==0 and all(0<x<1 for x in pair)
assert math.isclose(pair[1],.5*pair[0]+.5/8145060,rel_tol=1e-12)
output=(C.c_uint64*10)();assert generate(10,output,10)==0
assert all(x.bit_count()==6 and x>>45==0 for x in output)
mask=sum(1<<(i-1) for i in rows[60]);assert joint(mask,pair)==0
expected_loge=math.log(pair[1]*8145060)
with tempfile.TemporaryDirectory() as folder:
 path=pathlib.Path(folder)/'예측 기록.fjp'
 assert ledger(str(path),1)==0
 before=path.read_bytes();assert len(before)==130256
 assert hashlib.sha256(before[:-32]).digest()==before[-32:]
 assert ledger(str(path),1)==1 and path.read_bytes()==before
 raw=csv(rows);assert load(raw,len(raw))==0
 assert joint(mask,pair)==-4
 assert ledger(str(path),0)==0
 assert snapshot(state,128)==0
 assert struct.unpack_from('<II',state,24)==(1,0)
 actual=struct.unpack_from('<d',state,48)[0]
 assert math.isclose(actual,expected_loge,abs_tol=1e-10),(actual,expected_loge)
 assert ledger(str(path),0)==0
 assert snapshot(state,128)==0 and struct.unpack_from('<I',state,24)[0]==1
 assert struct.unpack_from('<d',state,48)[0]==actual
 # Mismatched prefix and damaged records must fail without changing the file.
 changed=[rows[1]]+rows[1:];raw=csv(changed);assert load(raw,len(raw))==0
 assert ledger(str(path),0)==-3 and path.read_bytes()==before
 raw=csv(rows);assert load(raw,len(raw))==0
 path.write_bytes(before[:-1]);assert ledger(str(path),0)==-3
 path.write_bytes(before[:100]+bytes([before[100]^1])+before[101:]);assert ledger(str(path),0)==-3
 assert snapshot(state,128)==0 and struct.unpack_from('<I',state,120)[0]==0
print('PASS Windows research ABI, exact forecast mixture, research generation, complete suite, immutable pending forecast, prospective log-e, idempotent evaluation, prefix and corruption rejection')
