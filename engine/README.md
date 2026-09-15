# Fortuna ORACLE Engine 1.5.0

양쪽 기본 ORACLE 경로에 다중 기간·관계 보정과 순차 검증을 추가했습니다.
[수식·계산량·API·검증 명세](docs/DEPTH_VALIDATION.md)

일반·심층 기본 경로의 [열 가지 통합 추론 기능](docs/INTEGRATED_INFERENCE.md)을
`integrated.inc`에서 구현합니다. 네 모형의 유한 계수 사후분포를 시간순으로
결합하고 강건화한 확률을 `GenerateIntegrated`로 추첨합니다.

Febius Fortuna의 분석·검증·번호 생성 엔진입니다. **기존 `NokMyo/fortuna` 저장소의 `engine/` 폴더에서 독립적으로 관리합니다.** 계산 로직은 x64 어셈블리이며 GUI에 의존하지 않습니다.

## 구성

- `src/`: 수학, 데이터, 후보 평가, 생성, 백테스트, 보고서, 봉인, 공개 API
- `fortuna_oracle.s`, `modules.txt`: 어셈블리 진입점과 모듈 목록
- `docs/`: [공개 API](docs/ENGINE_API.md), [실제 구현 범위](docs/IMPLEMENTATION.md), 전체 연구 설계
- `tests/`: 동일 어셈블리의 수학 검증과 Windows API 검사
- `sdk/`: 호출 규격 헤더와 어셈블리 사용 예제
- `data/`: 입력 형식과 가상 이력 예제

## 독립 빌드

Windows x64 Visual Studio 개발자 명령 프롬프트에서 LLVM clang과 Windows SDK를 준비합니다.

```bat
cd engine
build.bat
```

`build/FortunaOracleStatic.lib`를 생성합니다. 루트 앱 빌드는 이 라이브러리를 EXE에 포함하므로 사용자는 별도 DLL 없이 실행합니다. 개발용 DLL과 예제가 필요한 경우에만 `build.bat sdk`를 실행합니다.

Linux 교차 빌드는 `sh scripts/build-linux.sh`로 정적 라이브러리를 만듭니다. `sdk` 인수를 추가하면 개발용 DLL도 생성합니다. `python tests/native.py`와 `python tests/engine.py`는 GCC와 Python/NumPy가 필요합니다. Windows DLL 검사는 `build.bat sdk` 실행 후 `python tests/dll-windows.py`로 실행합니다.

엔진 버전은 `VERSION`, 공개 호출 규격은 ABI 1.0으로 관리합니다. 엔진 내부 함수와 상태는 공개하지 않습니다. 앱 연결 코드는 상위 `src/engine_bridge.inc`에 있습니다. 개발·검증용 Python과 헤더는 실행 로직에 포함되지 않습니다.

## 정밀 연구 확장

`joint.inc`, `research.inc`, `forecast.inc`가 공동 확률모형, 축소, 탐색 누락 감사, 완전한 성분 제거 재검증, 중첩 검증, 적응적 섭동과 사전 예측 기록을 구현합니다. [계산 규격과 제한](docs/RESEARCH.md)을 기준으로 사용하세요. 공개 API는 기존 ABI 1.0 구조체를 유지하고 연구용 함수를 추가했습니다. 일반 분석과 연구 분석은 같은 엔진 잠금을 사용합니다.

`FortunaOracleGenerateUniform`은 완료된 분석을 보존하면서 균등 무작위 추첨합니다. `Generate`는 표준 검증 정책, `GenerateResearch`는 완료된 심층 공동 모형을 사용합니다.
