# Febius Fortuna 1.1.0

**Windows 클래식 로또 6/45 번호 생성기 · ORACLE Field Architecture**

[Windows 릴리스](https://github.com/NokMyo/fortuna/releases/tag/v1.1.0) · [사용 설명서](docs/USER_GUIDE.md) · [실제 구현 명세](docs/IMPLEMENTATION.md)

**독립 엔진:** `FortunaOracle.dll` (Fortuna ORACLE Engine 1.0.0, ABI 1.0). 앱은 DLL의 공개 함수만 호출합니다. [SDK와 호출 규격](docs/ENGINE_API.md)

프로그램 전체 로직은 **x86-64 어셈블리어**입니다. C/C++ 런타임 없이 Win32 API를 직접 호출합니다. Febius Downrush의 유틸리티 디자인 방향에 맞춰 회색 기본 창, 네이티브 메뉴, 사각 버튼과 명확한 번호 표시를 사용합니다.

## 실행

Windows 10 1607 이상 또는 Windows 11, x64 환경에서 `FebiusFortuna.exe`를 실행합니다. 실행 파일과 `FortunaOracle.dll`을 같은 폴더에 두세요. 설치나 관리자 권한, 별도 런타임이 필요 없습니다. ZIP에는 설명서와 **가상 데이터** 예제가 포함됩니다. 실제 추첨 기록은 포함하지 않습니다.

1. 데이터 없이 **번호 생성 / 5게임 / 10게임**을 바로 사용합니다.
2. 과거 회차 CSV를 불러옵니다. 분석에는 연속된 60회차 이상이 필요합니다.
3. **오라클 분석**으로 모델, 전수 탐색, 안정성 분석과 백테스트를 계산합니다.
4. 완료 후 다시 생성하면 같은 분석 결과에서 새 번호를 추출합니다.
5. 번호 복사·저장, 분석 보고서 내보내기, 회차별 공식 번호 봉인을 사용할 수 있습니다.

## ORACLE

8,145,060개 조합을 전수 탐색해 4,096개를 정밀 평가하고 512개 후보장을 구성합니다. Bayesian 번호 잔차, 다중 EWMA, 이산 hazard, pair/triple 관계, spectral lattice, Mahalanobis 구조 공간, entropy, 234개 이웃 조합 및 32개 가상 이력의 안정성을 함께 계산합니다.

검증은 회차 순서대로 과거만 학습합니다. 512개 균등 무작위 기준선, 모델별 BH 다중검정, 상관관계 중복 억제, 별도로 유보한 마지막 30회차 확인을 거쳐야 가중 정책을 승격합니다. 근거가 부족하면 균등 무작위 생성으로 돌아갑니다. 120회차 미만에서는 정책 승격을 허용하지 않습니다.

ORACLE 지수는 내부 점수의 균등 표본 대비 상대 위치이며 당첨확률이 아닙니다. 공정한 추첨에서 모든 조합의 실제 1등 확률은 **1 / 8,145,060**입니다.

## 개발 및 검증

Windows x64 Visual Studio 개발자 명령 프롬프트에서 LLVM clang, Windows SDK, MSVC linker를 준비한 뒤 실행합니다.

```bat
build.bat
build\FebiusFortuna.exe --self-test
build\FebiusFortuna.exe --analyze data\SYNTHETIC-example.csv build\report.txt
```

Linux에서는 MinGW binutils/headers/libraries와 GCC가 있으면 `sh scripts/build-linux.sh`로 교차 빌드합니다. `python tests/native.py`, `python tests/engine.py`는 동일 어셈블리 코어를 독립적인 Python/NumPy 기준과 대조합니다. Python과 PowerShell은 개발·검증 도구이며 실행 파일에 포함되지 않습니다.

GitHub Actions는 실제 Windows 빌드·자체 검사·한글 경로·GUI 제어·화면 캡처와 Linux 수학 검증을 통과한 동일 실행 파일을 패키징합니다. `main`에서 모든 작업이 성공하면 `VERSION`에 해당하는 릴리스를 생성합니다. 기존 버전의 태그나 첨부 파일은 자동으로 덮어쓰지 않습니다.

## 설계 문서

- [USER_GUIDE.md](docs/USER_GUIDE.md): 사용법과 문제 해결
- [IMPLEMENTATION.md](docs/IMPLEMENTATION.md): 1.0.0의 수식, 검증 경계, 구현 여부
- [ORACLE.md](docs/ORACLE.md), [ORACLE_FIELD_ARCHITECTURE.md](docs/ORACLE_FIELD_ARCHITECTURE.md): 전체 설계와 장기 연구 명세
- [FORMAT.md](data/FORMAT.md): 입력 데이터 계약

**제품 버전 1.1.0은 설계 문서의 모든 장기 연구 항목 완료를 의미하지 않습니다.** 자동 데이터 공급, 고차 모형, AVX/GPU 가속, 외부 타임스탬프 등 미구현 항목은 구현 명세에 명시합니다.
