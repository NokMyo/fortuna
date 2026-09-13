"""Capture native exception addresses/registers when a Windows CI check fails."""
import ctypes as C
from ctypes import wintypes as W
import pathlib,sys,struct
k=C.WinDLL('kernel32',use_last_error=True)
class STARTUP(C.Structure):
    _fields_=[('cb',W.DWORD),('reserved',W.LPWSTR),('desktop',W.LPWSTR),('title',W.LPWSTR),('x',W.DWORD),('y',W.DWORD),('cx',W.DWORD),('cy',W.DWORD),('xc',W.DWORD),('yc',W.DWORD),('fill',W.DWORD),('flags',W.DWORD),('show',W.WORD),('reserved_size',W.WORD),('reserved_ptr',C.c_void_p),('stdin',W.HANDLE),('stdout',W.HANDLE),('stderr',W.HANDLE)]
class PROCESS(C.Structure):
    _fields_=[('process',W.HANDLE),('thread',W.HANDLE),('pid',W.DWORD),('tid',W.DWORD)]
k.CreateProcessW.argtypes=[W.LPCWSTR,W.LPWSTR,C.c_void_p,C.c_void_p,W.BOOL,W.DWORD,C.c_void_p,W.LPCWSTR,C.POINTER(STARTUP),C.POINTER(PROCESS)]
k.WaitForDebugEvent.argtypes=[C.c_void_p,W.DWORD]
k.ContinueDebugEvent.argtypes=[W.DWORD,W.DWORD,W.DWORD]
k.GetThreadContext.argtypes=[W.HANDLE,C.c_void_p]
k.ReadProcessMemory.argtypes=[W.HANDLE,C.c_void_p,C.c_void_p,C.c_size_t,C.c_void_p]
k.TerminateProcess.argtypes=[W.HANDLE,W.UINT]
si=STARTUP();si.cb=C.sizeof(si);pi=PROCESS()
exe=str(pathlib.Path(sys.argv[1]).resolve())
cmd=C.create_unicode_buffer('"'+exe+'" '+(' '.join(sys.argv[2:]) or '--self-test'))
assert k.CreateProcessW(None,cmd,None,None,False,2,None,None,C.byref(si),C.byref(pi)),C.get_last_error()
event=C.create_string_buffer(176);base=0
while k.WaitForDebugEvent(event,180000):
    code,pid,tid=struct.unpack_from('<III',event.raw)
    if code==3:base=struct.unpack_from('<Q',event.raw,40)[0]
    if code==1:
        exception=struct.unpack_from('<I',event.raw,16)[0]
        addr=struct.unpack_from('<Q',event.raw,32)[0]
        if exception not in (0x80000003,0x80000004):
            print(f'Native exception {exception:#x}, address {addr:#x}, image RVA {addr-base:#x}',flush=True)
            buf=C.create_string_buffer(1250);ptr=(C.addressof(buf)+15)&~15
            C.c_uint32.from_address(ptr+48).value=0x100003
            if k.GetThreadContext(pi.thread,ptr):
                registers={n:C.c_uint64.from_address(ptr+120+8*i).value for i,n in enumerate(['rax','rcx','rdx','rbx','rsp','rbp','rsi','rdi','r8','r9','r10','r11','r12','r13','r14','r15','rip'])}
                print({n:hex(v) for n,v in registers.items()},flush=True)
            fault=struct.unpack_from('<QQ',event.raw,48)
            print('Exception information:',[hex(x) for x in fault],flush=True)
            codebytes=C.create_string_buffer(32)
            if k.ReadProcessMemory(pi.process,addr,codebytes,32,None): print('Instruction bytes:',codebytes.raw.hex(),flush=True)
            mapfile=pathlib.Path('build/fortuna.map')
            if mapfile.exists():
                symbols=[]
                for line in mapfile.read_text().splitlines():
                    parts=line.split()
                    if len(parts)>2 and ':' in parts[0]:
                        try: symbols.append((int(parts[2],16)-0x140000000,parts[1]))
                        except ValueError: pass
                near=sorted((v,n) for v,n in symbols if v<=addr-base)
                print('Nearest symbols:',near[-5:],flush=True)
            k.TerminateProcess(pi.process,1)
            break
    if code==5:
        print('Debuggee exit:',struct.unpack_from('<I',event.raw,16)[0],flush=True);break
    k.ContinueDebugEvent(pid,tid,0x10002 if code!=1 or exception in (0x80000003,0x80000004) else 0x80010001)
