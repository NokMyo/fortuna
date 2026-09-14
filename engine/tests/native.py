"""Build the actual assembly engine as ELF for independent Python assertions.
Only this ABI adapter and OS entropy adapter differ from the Windows binary.
"""
import ctypes as C
import pathlib,re,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
MODULES=['core','hash','data','stats','linalg','models','field','robust','sample','joint','research','validation','advanced','advanced_fix','report']
def build(extra=()):
    modules=MODULES+list(extra)
    source='.intel_syntax noprefix\n'+ '\n'.join((ROOT/f'src/{m}.inc').read_text() for m in modules)
    source=source.replace('.section .rdata,"dr"','.section .data')
    source+='''\n.section .text
FN SystemFunction036
    mov rdi,rcx
    mov rsi,rdx
    mov rbx,rdx
    xor edx,edx
    mov eax,318
    syscall
    cmp rax,rbx
    sete al
    movzx eax,al
    END
'''
    names=re.findall(r'^FN (\w+)',source,re.M)
    for name in names:
        source+=f'''\n.globl test_{name}
test_{name}:
    mov r9,rcx
    mov r8,rdx
    mov rcx,rdi
    mov rdx,rsi
    sub rsp,40
    call {name}
    add rsp,40
    ret
'''
    for name in re.findall(r'^(\w+):\s*\.(?:skip|double|long|quad)',source,re.M):
        source+='\n.globl '+name+'\n'
    source+='\n.section .note.GNU-stack,"",@progbits\n'
    out=ROOT/'build';out.mkdir(exist_ok=True)
    (out/'native.s').write_text(source)
    subprocess.run(['gcc','-shared','-fPIC','-Wl,-Bsymbolic',str(out/'native.s'),'-o',str(out/'libfortuna.so')],check=True)
    return C.CDLL(str(out/'libfortuna.so'))
def fn(lib,name,args=(),result=C.c_uint64):
    f=getattr(lib,'test_'+name);f.argtypes=list(args);f.restype=result;return f
def scalar(lib,name,kind=C.c_uint32):return kind.in_dll(lib,name)
def array(lib,name,n,kind=C.c_double):return (kind*n).in_dll(lib,name)
if __name__=='__main__':
    lib=build()
    import hashlib,random,math
    sha=fn(lib,'Sha256',(C.c_void_p,C.c_uint64,C.c_void_p))
    for n in [0,1,3,55,56,63,64,65,119,120,127,128,1000,400000]:
        data=random.Random(n).randbytes(n);out=C.create_string_buffer(32)
        sha(data,n,out);assert out.raw==hashlib.sha256(data).digest(),n
    choose=fn(lib,'Choose',(C.c_uint32,C.c_uint32))
    for n in range(46):
        for k in range(7):assert choose(n,k)==(math.comb(n,k) if k<=n else 0)
    log=fn(lib,'Log',(C.c_double,),C.c_double)
    exp=fn(lib,'Exp',(C.c_double,),C.c_double)
    for x in [1e-20,.001,.5,1,2,500,1e20]:assert math.isclose(log(x),math.log(x),abs_tol=1e-14,rel_tol=1e-14)
    for x in [-600,-50,-1,0,1,50,600]:assert math.isclose(exp(x),math.exp(x),rel_tol=1e-12)
    parse=fn(lib,'ParseCsv',(C.c_void_p,C.c_uint64))
    text='round,date,n1,n2,n3,n4,n5,n6,bonus\n2,2025-01-11,2,4,6,8,10,12,14\n1,2025-01-04,1,3,5,7,9,11,13\n'.encode()
    assert parse(text,len(text))==1, scalar(lib,'parse_error').value
    assert scalar(lib,'draw_count').value==2
    assert scalar(lib,'last_round').value==2
    mean=array(lib,'feature_mean',8)
    assert mean[0]==39.0,list(mean)
    assert sum(array(lib,'freq_total',45,C.c_uint32))==12
    assert sum(array(lib,'triple_counts',14190,C.c_uint32))==40
    print('PASS SHA-256, binomial, elementary math, canonical parsing, statistics')
