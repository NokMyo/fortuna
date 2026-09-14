# 앱과 ORACLE 엔진의 폴더 분리

모든 코드는 `NokMyo/fortuna` 저장소에 있습니다. 엔진의 소스·문서·테스트·SDK를 `engine/` 아래에 모으고, 사용자에게는 엔진이 포함된 단일 `FebiusFortuna.exe`를 배포합니다.

## 경계

앱의 `src/fortuna.s`는 화면·클립보드·파일 대화상자·작업 스레드를 관리합니다. `src/engine_bridge.inc`가 엔진 공개 API를 호출하고 결과를 앱 상태로 복사합니다. 앱은 `engine/src/` 구현 파일을 직접 include하지 않습니다.

엔진의 `engine/fortuna_oracle.s`는 `engine/src/`만 조립합니다. 엔진은 창이나 대화상자를 만들지 않으며 앱 내부 기호를 참조하지 않습니다. 전역 공개 기호는 `FortunaOracle` API로 제한하고 내부 함수와 상태는 모듈 안에 둡니다.

루트 `build.bat`는 엔진 정적 라이브러리를 먼저 만들고 앱과 연결합니다. 엔진만 빌드하려면 `engine/build.bat`를 실행합니다. 외부 프로그램 연동용 DLL은 `engine/build.bat sdk`로 별도 생성할 수 있으며 일반 앱 패키지에는 포함하지 않습니다.

## 버전과 검증

앱 버전은 루트 `VERSION`, 엔진 버전은 `engine/VERSION`으로 관리합니다. API 구조체와 동시 호출 계약은 [엔진 API 문서](../engine/docs/ENGINE_API.md)에 정의합니다. 정적 연결에서도 엔진은 프로세스 안에서 하나의 상태를 사용합니다.

GitHub Actions는 엔진 수학 검증, 개발용 DLL의 공개 API 검사, 그리고 DLL 없이 EXE만 복사한 폴더에서 실제 Windows 앱 검사를 실행합니다. 모든 검증을 통과한 main 빌드만 새 버전의 릴리스를 게시합니다. 기존 릴리스는 덮어쓰지 않습니다.
