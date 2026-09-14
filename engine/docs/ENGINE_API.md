# Fortuna ORACLE Engine 1.1.0 — Windows x64 SDK

`FortunaOracle.dll` is a standalone, pure x86-64 assembly analysis library. It does not load Febius Fortuna, create windows, open dialogs or depend on a C/C++ runtime. Its only runtime dependencies are Windows Kernel32 and Advapi32. The GUI is a separate client of this public ABI.

The same API is also available in `FortunaOracleStatic.lib` for embedding in a single application executable. When using the header for static linkage define `FORTUNA_ORACLE_STATIC`. No DLL is then required at runtime.

## Distribution and versioning

- Application: **Febius Fortuna 1.3.0** (`FebiusFortuna.exe`).
- Engine: **Fortuna ORACLE Engine 1.1.0** (`FortunaOracle.dll`).
- Binary interface: **ABI 1.0**, `0x00010000`.

The application embeds the static engine in its EXE; its ZIP requires no engine DLL. Engine source, documentation, tests and SDK live under `engine/` in the same Fortuna repository. Run `engine/build.bat sdk` to build a developer DLL, import library and standalone assembly client.

Engine/application releases are separately identified. Version integers encode major in bits 16–31, minor in 8–15 and patch in 0–7. Clients must check the exact supported ABI before using structures; ABI 1.0 structures are immutable. Future incompatible layouts require a different ABI. Do not replace DLLs based only on matching filenames.

## ABI and ownership

All entry points use the standard Microsoft Windows x64 calling convention and exact undecorated names in `FortunaOracle.def`. `sdk/include/fortuna_oracle.h` declares the ABI for clients in other languages; it is not an engine implementation. The library never returns a private-state pointer or a memory allocation that the caller must free. Buffers and filenames belong to the caller and must remain valid throughout a call.

There is **one engine state per linked engine module per process**. Multiple consumers in the same process share that state. This version has no multi-context handles. Use separate processes for isolated concurrent analyses. Do not unload the DLL while any call or client thread is active.

Initialize before normal operations. Initialization is idempotent and performs lightweight hash/combinatorial/OS-entropy checks without loading data. It does not reset an already initialized engine. Mutating operations and snapshots use a private exclusive SRW lock; concurrent calls return `FO_BUSY` instead of racing or blocking. `Cancel`, `GetProgress` and version queries are callable while analysis runs. Run blocking analysis on a caller-owned worker thread.

## Functions

| Function | Contract |
|---|---|
| `GetAbiVersion()` / `GetEngineVersion()` | Return the respective packed version without initialization. All names have the `FortunaOracle` prefix. |
| `Initialize()` | Return 0 on success. Required before stateful operations. |
| `LoadCsv(utf8, uint64 bytes)` | Transactional import from caller memory, at most 16 MiB. No trailing NUL required. |
| `LoadCsvFileW(path)` | Transactional import through a NUL-terminated UTF-16 filename. |
| `Analyze()` | Synchronous analysis using the loaded history. At least 60 rows. Resets cancellation on entry; requests apply to the active analysis. |
| `Cancel()` | Request cooperative cancellation. No waiting. A cancelled `Analyze` returns −6. |
| `Generate(count, masks, capacity)` | Count 1–10; capacity measured in uint64 masks, not bytes. On success fills exactly count 45-bit masks. Six bits are set in each mask; bit 0 represents number 1. Generation works uniformly without history. |
| `GetSnapshot(out, bytes)` | Copy the fixed 272-byte ABI structure. Requires at least 272 bytes. No live internal pointers. |
| `GetProgress(out, bytes)` | Copy four uint32 counters (16 bytes): stage, percent, completed learning folds, history count. Values are approximate independently sampled counters, not an atomic multi-field snapshot. Stage-local percent can restart for each validation origin. |
| `GetReport(out, bytes)` | Positive result is required UTF-8 buffer size **including NUL**. Query with NULL,0. Too-small buffers are left untouched and required size is returned. Negative values are errors. Requires a completed candidate field. |
| `SealW(path)` | Append or reuse a sealed selection at an explicit caller-supplied ledger path. Returns 0 appended, 1 reused, negative error. Path length 1–1022 UTF-16 code units. Read the resulting selection via snapshot. No implicit AppData location inside the engine. |
| `UseUniform()` | Disable the active candidate field and switch to uniform selection. History remains loaded; re-analyze to restore the field. |
| `SelfTest()` | Destructive diagnostic: replaces the process-local dataset and selection with fixtures. Tests an isolated temporary ledger, never the application's user ledger. Returns 0, a negative API error, or a positive 10+ failure stage. Prefer a dedicated diagnostic process. |

Except version/report/self-test/seal return conventions, success is 0. Errors: −1 invalid argument/size; −2 engine busy; −3 input, analysis or file operation failed; −4 not initialized or no completed field; −5 entropy/startup check failed; −6 cancelled. Error details for rejected CSV are copied through snapshot's `parse_error` and `parse_line`. This is an in-process native ABI: callers are responsible for valid pointers and correctly sized backing memory; it is not a sandbox for untrusted plugins.

## Snapshot layout

Default Windows x64 alignment, maximum member alignment 8. Header contains compile-time size/version so a caller can verify its own definition.

| Offset | Value |
|---:|---|
| 0 / 4 / 8 / 12 | uint32 size=272, ABI version, engine version, flags (bit 0 field ready, bit 1 descriptive model ready) |
| 16 / 20 / 24 / 28 | uint32 history count, last round, parse error, parse line |
| 32 / 36 / 40 / 44 | uint32 candidate count, learning-fold count, selected index, generated session count |
| 48 | uint64 selected mask |
| 56 | six uint32 sorted selected numbers |
| 80 | ten uint64 session masks; read only `session_count` entries |
| 160 / 168 | binary64 uniform mixture and evidence budget |
| 176 / 208 / 240 | 32-byte dataset, candidate-field and configuration SHA-256 hashes |

After a successful CSV replacement, field readiness is cleared. A rejected replacement preserves the prior dataset. Hash/model fields are meaningful only for the corresponding ready flags; clients must not display a previous field hash as a new analysis result.

## Standalone assembly example

From an x64 Visual Studio developer prompt with LLVM clang, in the SDK directory:

```bat
clang --target=x86_64-pc-windows-msvc -c examples\generate.s -o generate.obj
link /nodefaultlib /entry:start /subsystem:console /out:generate.exe generate.obj FortunaOracle.lib kernel32.lib
generate.exe
```

Keep `FortunaOracle.dll` beside `generate.exe`. The sample checks ABI, initializes the library and generates five combinations without the GUI; exit code 0 means success. The `masks` buffer contains its output and demonstrates the binary calling contract.

CI runs this independent executable and a separate Python ctypes client against the real Windows DLL. It checks argument boundaries, version/layout, uniform generation, transactional import, completed analysis, report sizing/canary buffers, busy isolation, progress and cancellation. The GUI is tested independently against the same DLL. Existing numerical regression tests continue to build the actual relocated engine assembly for independent comparisons.

The engine extraction does not claim new predictive performance or complete the remaining mathematical research roadmap. All prior reference-model limitations still apply; scores are not jackpot probabilities.

## Research extension (engine 1.1.0, existing ABI 1.0 retained)

- `AnalyzeResearch(flags)`: synchronous complete suite, flags 0 audited fast selection or 1 full deep scoring. Other flags rejected. Same busy/cancel contract as Analyze. No partial research result becomes ready.
- `GetJointProbability(mask, out[2])`: valid six-bit mask in bits 0..44; writes pure model P and half-uniform forecast Q. Requires completed research for the current dataset.
- `GenerateResearch(count, masks, capacity)`: 1..10 independent Q samples, possible duplicates. Uses OS entropy. This is explicitly unvalidated research generation; the existing Generate retains the reference policy.
- `ForecastLedgerW(path, append)`: 0 audits an existing local FJP1 ledger; 1 audits and freezes the next forecast after research. Return 0 success, 1 already pending, negative failure. Missing/changed historical prefix, duplicate target, corrupt/truncated record or elapsed local deadline fails closed. Audit-only needs loaded history but does not require refitting the archived model.
- `GetResearchSnapshot(out, bytes)`: at least 128 bytes, see SDK header. Read `ready` before model diagnostics and `forecast_valid` before ledger values. Last uint32 is reserved. The original 272-byte snapshot is unchanged.

New successful CSV imports and ordinary Analyze invalidate the research-ready flag. Invalid CSV imports preserve prior research just as they preserve prior data. UseUniform disables research generation. Research analysis clears the ledger-valid flag until the ledger is audited again. The same one-state process model and private SRW lock apply to all research calls.

The report includes all ablation and outer-fold rows. Engine/client code and SDK declarations are versioned together in this repository. [Research formulas, limits and FJP1 layout](RESEARCH.md)

## Engine 1.2.0 추가 API (ABI 1.0 유지)

`FortunaOracleGenerateUniform(count, masks, capacity)`는 1~10개의 독립적인 균등 무작위 게임을 생성합니다. 일반/심층 분석의 모형·검증 결과를 보존하며 선택 번호의 내부 점수는 0으로 초기화합니다. 게임 간 같은 조합이 반복될 수 있습니다. 인자 오류 -1, busy -2, 초기화 필요 -4, 보안 난수 실패 -5를 반환하고 성공 시 0을 반환합니다. 출력 버퍼는 성공 시에만 기록합니다.
