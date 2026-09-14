# 1.2.0

- 같은 저장소의 `engine/`에 엔진 소스·문서·테스트·SDK와 독립 빌드 진입점을 통합.
- 엔진 공개 API만 노출하고 내부 기호를 숨겨 앱과 정적으로 연결.
- 일반 배포를 엔진 포함 단일 EXE로 변경하고 DLL 없는 실제 Windows 실행 검증 추가.

# Changelog

## 1.1.0

- Extract all ORACLE analysis, randomness, reports and sealing into the independent pure-assembly FortunaOracle.dll 1.0.0.
- Make the native GUI a client of a versioned public ABI; no engine globals or analysis code are embedded in the GUI.
- Add caller-owned snapshots, buffer size negotiation, concurrent-call isolation, progress and cancellation.
- Ship a standalone engine SDK with import library, ABI header, export list and assembly example.
- Add direct DLL and separate-client Windows tests, retain core regression tests, and package both required runtime files together.

## 1.0.0

- Replace the prototype with a modular, pure x64 assembly ORACLE reference engine and classic Korean native Windows UI.
- Add transactional canonical CSV validation, SHA-256 identities, OS randomness without silent fallback, Bayesian and temporal statistics, censored hazard, pair/triple/spectral/structural models.
- Scan all 8,145,060 combinations, deeply evaluate 4,096, and construct a 512-candidate field with exhaustive 1-swap geometry and 32 counterfactual worlds.
- Add prefix-only rolling validation, 512 matched null baselines, BH model gates, and a separate last-30 confirmation for learned-policy promotion; otherwise use uniform sampling.
- Add stable entropy-targeted sampling, diversified 1/5/10-game output, clipboard/text export, detailed audit reports and deterministic local prediction seals.
- Add assembly self-tests, independent numerical/property tests, native Windows UI checks, PE resources/manifest, and verified release packaging.
- Document exact reference approximations and remaining research/product extensions instead of marking the entire master roadmap complete.
