# Fortuna ORACLE Engine

한국 로또 6/45 분석 엔진. 모든 구현은 x86-64 어셈블리어이며 GUI와 C/C++ 런타임에 의존하지 않습니다.

이 저장소는 엔진의 소스·수학 명세·검증·SDK를 관리합니다. [Febius Fortuna](https://github.com/NokMyo/fortuna)는 별도 앱 저장소에서 이 엔진을 정적으로 연결하여 **단일 EXE**로 배포합니다. 사용자는 엔진을 따로 설치하지 않습니다.

## 빌드

x64 Visual Studio 개발자 명령 프롬프트에서 LLVM clang과 Windows SDK를 준비한 후 `build.bat`를 실행합니다.

- `build/FortunaOracleStatic.lib`: 앱에 포함할 정적 라이브러리
- `build/FortunaOracle.dll` 및 `FortunaOracle.lib`: 독립 개발자용 동적 라이브러리
- `build/oracle-example.exe`: 독립 어셈블리 호출 예제

공개 함수만 전역 심볼로 내보내므로 정적 연결 시 앱의 내부 함수 이름과 충돌하지 않습니다. 호출 규격과 상태 소유권은 [ENGINE_API.md](docs/ENGINE_API.md)에 정의합니다. 공개 ABI를 유지하며 앱과 엔진의 버전을 별도로 관리합니다.

## 검증

`python tests/native.py`와 `python tests/engine.py`는 동일한 어셈블리 수학을 독립 기준과 비교합니다. Windows에서는 `python tests/dll-windows.py`와 `oracle-example.exe`로 호출 규격·범위·동시 호출·취소를 확인합니다. Actions는 두 환경의 검증을 수행합니다.

기존 설계의 연구·확장 항목과 통계적 한계는 [IMPLEMENTATION.md](docs/IMPLEMENTATION.md)에 명시합니다. 내부 점수는 당첨확률이 아닙니다.
