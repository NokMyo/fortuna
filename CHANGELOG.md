# 1.9.1 / engine 1.6.0

- 로그인창 최초 상태 문구를 비우고, 입력값 검증 실패 시에만 `유효한 값을 입력하십시오.`를 표시하도록 수정.

# 1.9.0 / engine 1.6.0

- Febius Account 로그인 UI를 아이디·비밀번호·유효값 안내 중심으로 단순화.
- 일반·심층 ORACLE에 사용자 지정 causal backtest target range 추가.
- 범위 마지막 30회 독립 확인 유지, 미래 행 차단 및 범위 변경 시 캐시 무효화.
- 통합 추론과 심층 nested validation도 선택한 백테스트 범위에 맞춰 평가.
- CPU 마이크로벤치마크와 선택 회차 수를 이용한 일반/정밀 분석 예상 소요시간 UI 추가.
- 분석 보고서에 선택 백테스트 회차 범위와 confirmation fold 수 추가.

# 1.8.0 / engine 1.5.1

- 사용자 제공 Excel 이력에서 검증한 로또 6/45 1~1242회차를 애플리케이션에 기본 내장.
- GUI 시작 시 내장 이력을 자동 로드하고 다음 대상 회차 1243을 바로 표시.
- 기존 CSV 선택 기능은 외부 데이터로 현재 세션을 교체하는 보조 기능으로 유지.
- 빌드 전에 압축 데이터의 SHA-256, 회차 연속성, 번호 범위·중복·보너스를 전수 검증.
- self-test에서 최종 EXE의 내장 이력을 실제 엔진 parser로 로드해 1242행/최신 1242회차를 확인.

# 1.7.1 / engine 1.5.1

- Windows 세션 CSPRNG를 SystemFunction036/RtlGenRandom에서 BCryptGenRandom으로 교체.
- PE 체크섬 생성과 ASLR·High Entropy VA·NX 호환 상태 CI 검증 강화.
- 앱 User-Agent/버전 메타데이터 정리 및 legacy RNG import 회귀 검사 추가.
- ORACLE 분석 수식, 계정·라이선스 정책, 저장 형식은 변경 없음.

# 1.7.0 / engine 1.5.0

- 양쪽 기본 경로에 다중 기간 베타 추정·시간순 가중·관계 행렬 보정 추가.
- 보상 합산으로 정규화 상수와 미분 계산, 곡률·확률 질량·유한값 검사.
- 온라인 혼합, 순차 다중 검정, 블록 재표본 구간, 제거 민감도·성능 저하 진단.
- 검증 회차 8/16, 재표본 512/4096으로 확장하고 일반의 검증 문턱 강화.
- 독립 수식·작은 귀무 이력 전체 열거·재표본 분산·Windows 진단 API 검사 추가.

# 1.6.0 / engine 1.4.0

- 일반·심층 기본 경로에 열 가지 통합 수학 기능과 실제 추첨 분포 연결.
- 유한 격자 사후분포, 네 모형·균등 분포 결합, 실제 가상 이력 전체 재학습.
- 시간순 가중치 검증·KL 강건화·AIS와 전수 합계 경계 점검.
- 독립 수학 대조 및 Windows 기본 API 경로·취소·캐시 검사 추가.
- 기존 로그인·라이선스 검사 및 기존 기록 파일 보존. 기록 메뉴의 대상 모형 명시.

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
