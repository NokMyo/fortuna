# Febius Fortuna 1.4.1

**Windows 클래식 로또 6/45 번호 생성기 · ORACLE Field Architecture**

[Windows 릴리스](https://github.com/NokMyo/fortuna/releases/tag/v1.4.1) · [사용 설명서](docs/USER_GUIDE.md) · [실제 구현 명세](engine/docs/IMPLEMENTATION.md)

**엔진 전용 폴더:** [`engine/`](engine/)에 Fortuna ORACLE Engine 1.2.0의 소스·문서·테스트·SDK를 모았습니다. 앱은 공개 API로 연결하며, 빌드할 때 엔진을 실행 파일에 포함합니다. [SDK와 호출 규격](engine/docs/ENGINE_API.md)

프로그램 전체 로직은 **x86-64 어셈블리어**입니다. C/C++ 런타임 없이 Win32 API를 직접 호출합니다. Febius Downrush의 유틸리티 디자인 방향에 맞춰 회색 기본 창, 네이티브 메뉴, 사각 버튼과 명확한 번호 표시를 사용합니다.

## 실행

Windows 10 1607 이상 또는 Windows 11, x64 환경에서 `FebiusFortuna.exe`를 실행합니다. **`FebiusFortuna.exe` 하나로 실행하며 별도 엔진 DLL이 필요 없습니다.** 설치나 관리자 권한, 별도 런타임이 필요 없습니다. ZIP에는 설명서와 **가상 데이터** 예제가 포함됩니다. 실제 추첨 기록은 포함하지 않습니다.

1. **1·5·10게임**을 선택하고 **예상 번호 추첨**을 누르면 데이터 없이 균등 무작위 추첨합니다.
2. 과거 회차 CSV를 불러옵니다. 분석에는 연속된 60회차 이상이 필요합니다.
3. **ORACLE기반 예상 번호 추첨**은 일반 분석, **ORACLE기반 심층 분석 추첨**은 장시간 정밀 연구를 수행한 후 번호를 추첨합니다.
4. 같은 ORACLE 버튼을 다시 누르면 완료된 분석을 재사용합니다. 랜덤 버튼은 항상 균등 무작위입니다.
5. 번호 복사·저장, 분석 보고서 내보내기, 회차별 공식 번호 봉인을 사용할 수 있습니다.

## ORACLE

8,145,060개 조합을 전수 탐색해 4,096개를 정밀 평가하고 512개 후보장을 구성합니다. Bayesian 번호 잔차, 다중 EWMA, 이산 hazard, pair/triple 관계, spectral lattice, Mahalanobis 구조 공간, entropy, 234개 이웃 조합 및 32개 가상 이력의 안정성을 함께 계산합니다.

검증은 회차 순서대로 과거만 학습합니다. 512개 균등 무작위 기준선, 모델별 BH 다중검정, 상관관계 중복 억제, 별도로 유보한 마지막 30회차 확인을 거쳐야 가중 정책을 승격합니다. 근거가 부족하면 균등 무작위 생성으로 돌아갑니다. 120회차 미만에서는 정책 승격을 허용하지 않습니다.

ORACLE 지수는 내부 점수의 균등 표본 대비 상대 위치이며 당첨확률이 아닙니다. 공정한 추첨에서 모든 조합의 실제 1등 확률은 **1 / 8,145,060**입니다.

## 저장소 구조

- `src/`, `resources/`: 클래식 Windows 화면과 앱 기능
- `engine/src/`: ORACLE 계산·검증·생성 엔진의 어셈블리 소스
- `engine/docs/`, `engine/tests/`, `engine/sdk/`: 엔진 설계, 검증, 공개 API와 사용 예제
- `engine/build.bat`, `engine/scripts/build-linux.sh`: 앱 없이 엔진만 빌드하는 진입점
- `docs/`, `scripts/`: 앱 사용 설명서와 통합 빌드·검증·배포

모두 **같은 `NokMyo/fortuna` 저장소**에서 관리합니다. 추가 저장소나 소스 다운로드 단계는 없습니다. [폴더 간 책임과 연결 방식](docs/ARCHITECTURE.md)

## 정밀 연구 분석 (엔진 1.2.0)

**ORACLE기반 심층 분석 추첨**에 여섯 확장을 추가했습니다. 45C6 전체 정규화 공동 확률모형, 단계별 경험적 베이지안 축소, 후보 누락 검사와 8,192개 확장, 12개 성분의 완전한 제거 재검증, 중첩 시간순 로그 손실 평가와 사전 예측 기록, 적응적 불확실성 분석입니다.

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

Linux에서는 MinGW binutils/headers/libraries와 GCC가 있으면 `sh scripts/build-linux.sh`로 교차 빌드합니다. `python engine/tests/native.py`, `python engine/tests/engine.py`, `python engine/tests/research.py`는 동일 어셈블리 코어를 독립적인 Python/NumPy 기준과 대조합니다. Python과 PowerShell은 개발·검증 도구이며 실행 파일에 포함되지 않습니다.

GitHub Actions는 실제 Windows 빌드·자체 검사·한글 경로·GUI 제어·화면 캡처와 Linux 수학 검증을 통과한 동일 실행 파일을 패키징합니다. `main`에서 모든 작업이 성공하면 `VERSION`에 해당하는 릴리스를 생성하고, 그보다 낮은 기존 GitHub Release는 정리합니다.

## 설계 문서

- [USER_GUIDE.md](docs/USER_GUIDE.md): 사용법과 문제 해결
- [IMPLEMENTATION.md](engine/docs/IMPLEMENTATION.md): 1.0.0의 수식, 검증 경계, 구현 여부
- [ORACLE.md](engine/docs/ORACLE.md), [ORACLE_FIELD_ARCHITECTURE.md](engine/docs/ORACLE_FIELD_ARCHITECTURE.md): 전체 설계와 장기 연구 명세
- [FORMAT.md](engine/data/FORMAT.md): 입력 데이터 계약

**제품 버전 1.4.1은 설계 문서의 모든 장기 연구 항목 완료를 의미하지 않습니다.** 자동 데이터 공급, 고차 모형, AVX/GPU 가속, 외부 타임스탬프 등 미구현 항목은 구현 명세에 명시합니다.

## 추첨 방식과 구현 언어

**예상 번호 추첨 / ORACLE기반 예상 번호 추첨 / ORACLE기반 심층 분석 추첨**을 별도 버튼으로 제공합니다. 앱과 엔진의 실행 로직은 x64 어셈블리만 사용합니다. [세 가지 추첨 방식과 어셈블리 규칙](docs/DRAW_MODES.md)을 참고하세요.
