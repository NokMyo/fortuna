Febius Fortuna 1.1.0 — 독립 Fortuna ORACLE Engine

- 계산부를 **FortunaOracle.dll 1.0.0**으로 실제 분리했습니다. 앱과 별도 파일·별도 버전으로 관리합니다.
- 앱 없이도 공개 ABI를 통해 CSV 분석, 번호 생성, 보고서와 봉인 기능을 사용할 수 있습니다.
- 앱과 엔진 모두 x86-64 어셈블리어이며 C/C++ 런타임은 사용하지 않습니다. 엔진은 GUI 라이브러리에 의존하지 않습니다.
- 독립 SDK에 DLL, import library, 호출 규격 헤더, 어셈블리 사용 예제를 제공합니다.
- 기존 클래식 화면을 유지하고 별도의 엔진 정보 창을 추가했습니다.

**일반 사용자는 앱 ZIP을 다운로드해 압축을 풀어 주세요. `FebiusFortuna.exe`와 `FortunaOracle.dll`을 같은 폴더에 두어야 합니다.** SDK ZIP은 다른 프로그램에 엔진을 연결할 개발자용입니다.

Windows 앱·독립 DLL·별도 어셈블리 클라이언트 실행 검사와 수학 회귀 검증을 거친 패키지입니다. 호출 규격은 [ENGINE_API.md](https://github.com/NokMyo/fortuna/blob/main/docs/ENGINE_API.md)에 정리했습니다. 기존 1.0.0 릴리스는 유지합니다.

이번 변경은 엔진 독립화입니다. 분석 방식의 기존 한계와 미구현 연구·확장 항목은 그대로 적용됩니다.
