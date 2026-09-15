# Febius Fortuna 1.7.0

일반·심층 ORACLE에 다중 기간 추정, 번호 관계 보정, 정규화 오차 검사,
시간순 성능 검정과 블록 재표본 검증을 추가했습니다.
[새 수식과 검증 범위](engine/docs/DEPTH_VALIDATION.md)

**Windows 클래식 로또 6/45 번호 생성기 · ORACLE Field Architecture**

[Windows 릴리스](https://github.com/NokMyo/fortuna/releases) · [사용 설명서](docs/USER_GUIDE.md) · [열 가지 통합 수학 기능](engine/docs/INTEGRATED_INFERENCE.md) · [Advanced Evidence Layer](engine/docs/ADVANCED_EVIDENCE_LAYER.md)

**엔진 전용 폴더:** [`engine/`](engine/)에 Fortuna ORACLE Engine 1.5.0의 소스·문서·테스트·SDK를 모았습니다. 앱은 공개 API로 연결하며, 빌드할 때 엔진을 실행 파일에 포함합니다. [SDK와 호출 규격](engine/docs/ENGINE_API.md)

일반·심층 ORACLE 모두 고차 확률모형, 계수 불확실성 적분, 다단계 확률 탐색,
변화점, 가상 이력 재학습, 텐서, 동적 상태, 시간순 결합, 강건 최적화,
정규화 상수 경계 계산을 기본 실행합니다. 심층은 같은 기능을 더 넓게 계산합니다.
계수 적분은 경험적 베이지안 유한 격자이고 동적 상태는 주변 확률 근사입니다.
실제 범위·검증·계산 비용은 [통합 명세](engine/docs/INTEGRATED_INFERENCE.md)에 명시합니다.
같은 방식으로 다시 추첨하면 완료된 분석을 재사용합니다. 두 ORACLE 방식 모두
게임 간 독립 추출이므로 같은 게임이 반복될 수 있습니다.

프로그램 전체 실행 로직은 **x86-64 어셈블리어**입니다. C/C++ 런타임 없이 Win32 API를 직접 호출합니다. Febius Downrush의 유틸리티 디자인 방향에 맞춰 회색 기본 창, 네이티브 메뉴, 사각 버튼과 명확한 번호 표시를 사용합니다.

## 실행

Windows 10 1607 이상 또는 Windows 11, x64 환경에서 `FebiusFortuna.exe`를 실행합니다. **`FebiusFortuna.exe` 하나로 실행하며 별도 엔진 DLL이 필요 없습니다.** 설치나 관리자 권한, 별도 런타임이 필요 없습니다. ZIP에는 설명서와 **가상 데이터** 예제가 포함됩니다. 실제 추첨 기록은 포함하지 않습니다.

1. **1·5·10게임**을 선택하고 **예상 번호 추첨**을 누르면 데이터 없이 균등 무작위 추첨합니다.
2. 과거 회차 CSV를 불러옵니다. 분석에는 연속된 60회차 이상이 필요합니다.
3. **ORACLE기반 예상 번호 추첨**은 일반 분석, **ORACLE기반 심층 분석 추첨**은 장시간 정밀 연구를 수행한 후 번호를 추첨합니다.
4. 같은 ORACLE 버튼을 다시 누르면 완료된 분석을 재사용합니다. 랜덤 버튼은 항상 균등 무작위입니다.
5. 번호 복사·저장, 분석 보고서 내보내기, 회차별 공식 번호 봉인을 사용할 수 있습니다.

## ORACLE

기본 ORACLE은 8,145,060개 조합을 전수 탐색하고 Bayesian 번호 잔차, 다중 EWMA, 이산 hazard, pair/triple 관계, spectral lattice, Mahalanobis 구조 공간, entropy, 이웃 조합 및 가상 이력 안정성을 함께 계산합니다.

검증은 회차 순서대로 과거만 학습하는 rolling/prefix 방식입니다. 균등 무작위 기준선, 모델별 BH 다중검정, 상관관계 중복 억제, 별도로 유보한 마지막 30회차 확인을 거쳐야 가중 정책을 승격합니다. 근거가 부족하면 균등 무작위 생성으로 돌아갑니다. 120회차 미만에서는 정책 승격을 허용하지 않습니다.

### Advanced Evidence Layer 1.3

일반 ORACLE과 심층 ORACLE 모두 기존 검증 뒤에 AEL을 적용합니다.

- 4개 번호 상호작용을 148,995개 4-subset 공간에서 prefix-only로 집계하고, 하위 triple 신호와 계층적 shrinkage로 결합합니다.
- 새 4차 신호 자체도 walk-forward matched-null 검증을 거쳐 지지도가 없는 경우 가중치가 0이 됩니다.
- 최근 prefix 검증 구간을 네 개의 시간 블록으로 분리해 Brier와 평균 적중 안정성을 다시 확인합니다.
- 일반 ORACLE은 4,096개, 심층 ORACLE은 8,192개의 구조보존 synthetic-null 세계로 기존 forecast가 가짜 패턴에서도 쉽게 살아나는지 스트레스 테스트합니다.
- 이미 검증된 모델 가중치도 표본수, BH q-value, null 생존도, 시간 안정성을 이용해 다시 shrinkage하며 불확실성이 커질수록 균등분포 혼합 비율을 높입니다.
- 4,096 fast 후보장을 최대 8,192개까지 다시 탐색하고 2,048개 advanced stage를 거친 뒤 512개 최종 field를 만듭니다.
- 최상위 후보는 두 번호를 동시에 바꾸는 완전한 2-swap 이웃 `C(6,2)×C(39,2)=11,115`개를 검사합니다. 일반은 상위 8개, 심층은 상위 32개에 적용합니다.
- 후보 점수가 두 개 모델에 과도하게 의존하는지 leave-two-model contribution stability를 계산해 최종 thermal score를 보정합니다.

AEL은 기존 검증 문턱을 우회하지 않습니다. 기반 ORACLE의 근거가 약하면 evidence를 더 줄이고 균등 혼합을 늘리는 방향으로만 작동합니다.

ORACLE 지수와 AEL 점수는 내부 분석 지표이며 당첨확률이 아닙니다. 공정한 추첨에서 모든 조합의 실제 1등 확률은 **1 / 8,145,060**입니다.

## 저장소 구조

- `src/`, `resources/`: 클래식 Windows 화면과 앱 기능
- `engine/src/`: ORACLE 계산·검증·생성 엔진의 어셈블리 소스
- `engine/docs/`, `engine/tests/`, `engine/sdk/`: 엔진 설계, 검증, 공개 API와 사용 예제
- `engine/build.bat`, `engine/scripts/build-linux.sh`: 앱 없이 엔진만 빌드하는 진입점
- `docs/`, `scripts/`: 앱 사용 설명서와 통합 빌드·검증·배포

모두 **같은 `NokMyo/fortuna` 저장소**에서 관리합니다. 추가 저장소나 소스 다운로드 단계는 없습니다. [폴더 간 책임과 연결 방식](docs/ARCHITECTURE.md)

## 정밀 연구 분석

**ORACLE기반 심층 분석 추첨**은 기존 정밀 연구·AEL 검증을 수행한 뒤 열 가지 통합 추론 기능을 실행합니다. 실제 추첨은 네 모형의 계수 사후분포를 결합한 통합 분포와 균등 분포의 혼합을 사용합니다. 기존 공동모형 사전 예측 기록은 별도 분석 도구로 유지되며 새 통합 분포의 기록과는 구분합니다.

심층 버튼은 공동 모형과 균등 분포의 혼합에서 직접 추첨하며, 일반 ORACLE 버튼은 검증 정책을 사용합니다. 연구 모형의 실제 예측 우위는 아직 입증되지 않았습니다. 연구는 계산량이 크며 취소할 수 있습니다. [수식·경계·사용법](engine/docs/RESEARCH.md)

```bat
build\FebiusFortuna.exe --research history.csv report.txt
```

**전체 조합 정밀 연구 (장시간)** / `--research-full`은 깊은 점수도 모든 조합에 적용합니다. **다음 회차 확률분포 기록 / 사전 예측 기록 검증**으로 분포 전체를 추첨 전에 저장하고 나중에 평가할 수 있습니다. 로컬 기록은 외부 공증이 아닙니다.

## 개발 및 검증

Windows x64 Visual Studio 개발자 명령 프롬프트에서 LLVM clang, Windows SDK, MSVC linker를 준비한 뒤 실행합니다.

```bat
build.bat
build\FebiusFortuna.exe --self-test
build\FebiusFortuna.exe --analyze engine\data\SYNTHETIC-example.csv build\report.txt
```

Linux에서는 MinGW binutils/headers/libraries와 GCC가 있으면 `sh scripts/build-linux.sh`로 교차 빌드합니다. `python engine/tests/native.py`, `python engine/tests/advanced.py`, `python engine/tests/engine.py`, `python engine/tests/research.py`는 동일 어셈블리 코어와 AEL을 독립적인 Python/NumPy 기준과 대조합니다. Python과 PowerShell은 개발·검증 도구이며 실행 파일에 포함되지 않습니다.

GitHub Actions는 실제 Windows 빌드·자체 검사·한글 경로·GUI 제어·화면 캡처와 Linux 수학 검증을 통과한 동일 실행 파일을 패키징합니다. `main`에서 모든 작업이 성공하면 `VERSION`에 해당하는 릴리스를 생성하고, 그보다 낮은 기존 GitHub Release는 정리합니다.

## 설계 문서

- [USER_GUIDE.md](docs/USER_GUIDE.md): 사용법과 문제 해결
- [IMPLEMENTATION.md](engine/docs/IMPLEMENTATION.md): 기존 ORACLE 수식, 검증 경계, 구현 여부
- [ADVANCED_EVIDENCE_LAYER.md](engine/docs/ADVANCED_EVIDENCE_LAYER.md): AEL 1.3의 4차 모델, null stress, shrinkage, 후보 심화
- [ORACLE.md](engine/docs/ORACLE.md), [ORACLE_FIELD_ARCHITECTURE.md](engine/docs/ORACLE_FIELD_ARCHITECTURE.md): 전체 설계와 장기 연구 명세
- [FORMAT.md](engine/data/FORMAT.md): 입력 데이터 계약

**제품 버전 1.7.0은 설계 문서의 모든 장기 연구 항목 완료를 의미하지 않습니다.** 자동 데이터 공급, 외부 공증, GPU 가속, 통합 분포의 영구 기록, 연속 전체 사후분포 적분과 증명된 구간 연산은 포함하지 않습니다. 열 기능의 실제 구현 범위는 통합 명세에 명시합니다.

## 추첨 방식과 구현 언어

**예상 번호 추첨 / ORACLE기반 예상 번호 추첨 / ORACLE기반 심층 분석 추첨**을 별도 버튼으로 제공합니다. 앱과 엔진의 실행 로직은 x64 어셈블리만 사용합니다. [세 가지 추첨 방식과 어셈블리 규칙](docs/DRAW_MODES.md)을 참고하세요.
