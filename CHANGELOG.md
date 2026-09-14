# 1.5.0 / engine 1.3.0

- 일반·심층 ORACLE에 Advanced Evidence Layer(AEL)를 적용.
- prefix-only 4차 조합 상호작용과 계층적 shrinkage·불확실성 전파 추가.
- 4구간 시간 안정성 및 4,096/8,192 synthetic-null 스트레스 검증 추가.
- 빠른 후보장을 4,096에서 최대 8,192로 확장하고 2,048개 고급 재평가 단계 및 2-swap 전수 이웃 탐색 추가.
- 상위 두 모델 제거 민감도 기반 후보 안정성 보정과 deep joint sampling의 검증된 AEL 결합 추가.

# 1.4.2 / engine 1.2.0

- 보안 취약점 수정.

# 1.4.1 / engine 1.2.0

- 버그 수정 및 최적화.

# 1.4.0 / engine 1.2.0

- 랜덤·일반 ORACLE·심층 ORACLE 세 가지 추첨 버튼과 공통 게임 수 선택.
- 심층 결과의 실제 공동 모형 추첨 연결, 모형을 보존하는 독립 균등 API와 분석 재사용.
- 실제 Windows GUI 경로·재사용 검증, 어셈블리 실행 로직 규칙과 사용법 문서화.

# 1.3.0 / engine 1.1.0

- 여섯 연구 확장을 공동 모형, 연구 오케스트레이션, 사전 예측 기록 모듈에 구현.
- 확률모형·기록·연구 생성 API와 클래식 분석 메뉴, CLI 추가.
- 수학 기준 대조와 미래 데이터 차단, 전체 재검증, 기록 손상·중복 평가 검사를 추가.
- 연구 모형과 운영 번호 생성의 범위 및 미입증 예측력을 문서화.

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
