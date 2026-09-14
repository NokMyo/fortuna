"""Independent assertions for the Advanced Evidence Layer assembly routines."""
import ctypes as C
import itertools, math, struct
import native

lib=native.build()
fn=native.fn
scalar=native.scalar

quad=fn(lib,'AdvancedQuadIndex',(C.c_uint32,C.c_uint32,C.c_uint32,C.c_uint32),C.c_uint64)
for a,b,c,d in itertools.combinations(range(45),4):
    expected=math.comb(d,4)+math.comb(c,3)+math.comb(b,2)+a
    assert quad(a,b,c,d)==expected,(a,b,c,d,quad(a,b,c,d),expected)

counts=(C.c_uint32*148995).in_dll(lib,'advanced_quad_counts')
C.memset(C.addressof(counts),0,C.sizeof(counts))
add=fn(lib,'AdvancedAddQuadMask',(C.c_uint64,),C.c_uint64)
mask=sum(1<<i for i in range(6))
assert add(mask)==1
assert sum(counts)==15
for q in itertools.combinations(range(6),4):
    assert counts[quad(*q)]==1

perm=fn(lib,'AdvancedPermuteMask',(C.c_uint64,C.c_uint32,C.c_uint32),C.c_uint64)
base=sum(1<<i for i in (0,4,9,17,28,44))
for rep in range(128):
    for block in range(4):
        out=perm(base,rep,block)
        assert out.bit_count()==6
        assert out>>45==0

# Synthetic prefix history: only the mask field is needed by AEL walk-forward.
history=(C.c_ubyte*(8192*48)).in_dll(lib,'history')
C.memset(C.addressof(history),0,C.sizeof(history))
for i in range(64):
    nums=sorted({(i*7+j*8)%45 for j in range(6)})
    while len(nums)<6:
        x=(nums[-1]+1)%45 if nums else 0
        if x not in nums: nums.append(x); nums.sort()
    m=sum(1<<n for n in nums[:6])
    struct.pack_into('<Q',history,i*48+40,m)
scalar(lib,'history_count').value=64
scalar(lib,'cancel_flag').value=0
assert fn(lib,'AdvancedValidateQuad',(),C.c_uint64)()==1
assert scalar(lib,'advanced_quad_train_count').value==64
assert sum(counts)==64*15
support=C.c_double.in_dll(lib,'advanced_quad_support').value
assert 0.0<=support<=1.0

# Four-block stability has a deterministic upper bound test.
rows=(C.c_ubyte*(8192*64)).in_dll(lib,'validation_rows')
C.memset(C.addressof(rows),0,C.sizeof(rows))
for i in range(32):
    struct.pack_into('<d',rows,i*64+24,1.0)
    struct.pack_into('<d',rows,i*64+40,0.05)
scalar(lib,'validation_count').value=32
assert fn(lib,'AdvancedTemporalStability',(),C.c_uint64)()==1
stability=C.c_double.in_dll(lib,'advanced_temporal_stability').value
assert math.isclose(stability,1.0,abs_tol=1e-12)

# Null stress test must stay a bounded probability/support transform.
for i in range(32):
    pred=sum(1<<n for n in ((i+0)%45,(i+3)%45,(i+8)%45,(i+14)%45,(i+23)%45,(i+35)%45))
    truth=pred
    struct.pack_into('<Q',rows,i*64+8,pred)
    struct.pack_into('<Q',rows,i*64+16,truth)
scalar(lib,'advanced_mode').value=0
assert fn(lib,'AdvancedSyntheticNull',(),C.c_uint64)()==1
p=C.c_double.in_dll(lib,'advanced_null_p').value
ns=C.c_double.in_dll(lib,'advanced_null_support').value
assert 0.0<=p<=1.0 and 0.0<=ns<=1.0

# Hierarchical uncertainty can only shrink learned evidence and move rho toward uniform.
weights=(C.c_double*12).in_dll(lib,'model_weights')
qvalues=(C.c_double*12).in_dll(lib,'model_qvalue')
unc=(C.c_double*12).in_dll(lib,'advanced_model_uncertainty')
for i in range(11):
    weights[i]=0.1+i*0.01
    qvalues[i]=0.01+i*0.005
before=list(weights[:11])
scalar(lib,'validation_count').value=120
C.c_double.in_dll(lib,'advanced_null_support').value=0.8
C.c_double.in_dll(lib,'advanced_temporal_stability').value=0.75
C.c_double.in_dll(lib,'evidence').value=1.0
C.c_double.in_dll(lib,'rho').value=0.25
assert fn(lib,'AdvancedShrinkWeights',(),C.c_uint64)()==1
assert all(0.0<=weights[i]<=before[i]+1e-15 for i in range(11))
assert all(0.0<=unc[i]<=1.0 for i in range(11))
assert 0.0<=C.c_double.in_dll(lib,'evidence').value<=1.0
assert 0.25<=C.c_double.in_dll(lib,'rho').value<=1.0

# Fourth-order contribution is separately capped, with deep mode allowed a wider bound.
C.c_double.in_dll(lib,'advanced_quad_support').value=1.0
C.c_double.in_dll(lib,'advanced_null_support').value=1.0
C.c_double.in_dll(lib,'advanced_temporal_stability').value=1.0
scalar(lib,'advanced_mode').value=0
assert fn(lib,'AdvancedSetQuadWeight',(),C.c_uint64)()==1
normal_cap=C.c_double.in_dll(lib,'advanced_quad_weight').value
scalar(lib,'advanced_mode').value=1
assert fn(lib,'AdvancedSetQuadWeight',(),C.c_uint64)()==1
deep_cap=C.c_double.in_dll(lib,'advanced_quad_weight').value
assert math.isclose(normal_cap,0.08,abs_tol=1e-12)
assert math.isclose(deep_cap,0.15,abs_tol=1e-12)

print('PASS AEL quad model, prefix walk-forward, temporal/null validation, uncertainty shrinkage and caps')
