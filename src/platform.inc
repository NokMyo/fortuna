# Febius Fortuna — x86-64 Windows native application
# No C/C++ runtime. UI, CSV parser, statistics, exhaustive scan and sampler are assembly.

.equ WS_OVERLAPPEDWINDOW, 0x00CF0000
.equ WS_VISIBLE,          0x10000000
.equ WS_CHILD,            0x40000000
.equ WS_BORDER,           0x00800000
.equ SS_CENTER,           0x00000001
.equ BS_PUSHBUTTON,       0x00000000
.equ ES_MULTILINE,        0x00000004
.equ ES_READONLY,         0x00000800
.equ ES_AUTOVSCROLL,      0x00000040
.equ WS_VSCROLL,          0x00200000
.equ WM_CREATE,           0x0001
.equ WM_DESTROY,          0x0002
.equ WM_COMMAND,          0x0111
.equ WM_SETFONT,          0x0030
.equ WM_APP_DONE,         0x8001
.equ SW_SHOWDEFAULT,      10
.equ COLOR_WINDOW_BRUSH,  6
.equ IDC_ARROW,           32512
.equ ID_LOAD,             1001
.equ ID_ANALYZE,          1002
.equ ID_RECALC,           1003
.equ GENERIC_READ,        0x80000000
.equ FILE_SHARE_READ,     0x00000001
.equ OPEN_EXISTING,       3
.equ FILE_ATTRIBUTE_NORMAL, 0x80
.equ INVALID_HANDLE_VALUE, -1
.equ HEAP_ZERO_MEMORY,    0x00000008
.equ OFN_FILEMUSTEXIST,   0x00001000
.equ OFN_PATHMUSTEXIST,   0x00000800
.equ OFN_EXPLORER,        0x00080000
.equ TOP_FAST,            4096
.equ TOP_FINAL,           512
.equ EXPECTED_Q16,        8738
.equ SHORT_DECAY_Q16,     60094
.equ MID_DECAY_Q16,       64136
.equ LONG_DECAY_Q16,      65182
.equ SHORT_ADD_Q16,       5442
.equ MID_ADD_Q16,         1400
.equ LONG_ADD_Q16,        354

.extern GetModuleHandleA
.extern RegisterClassExA
.extern CreateWindowExA
.extern DefWindowProcA
.extern ShowWindow
.extern UpdateWindow
.extern GetMessageA
.extern TranslateMessage
.extern DispatchMessageA
.extern PostQuitMessage
.extern LoadCursorA
.extern CreateFontA
.extern DeleteObject
.extern SendMessageA
.extern SetWindowTextA
.extern EnableWindow
.extern wsprintfA
.extern GetOpenFileNameA
.extern CreateFileA
.extern GetFileSizeEx
.extern ReadFile
.extern CloseHandle
.extern GetProcessHeap
.extern HeapAlloc
.extern HeapFree
.extern CreateThread
.extern PostMessageA
.extern SystemFunction036
.extern ExitProcess
