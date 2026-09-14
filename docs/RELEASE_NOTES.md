# Febius Fortuna 1.4.0

- 세 가지 독립 버튼: 예상 번호 추첨(균등 무작위), ORACLE기반 예상 번호 추첨(일반 분석), ORACLE기반 심층 분석 추첨(정밀 연구).
- 1·5·10게임 선택을 모든 추첨 방식에 공통 적용.
- 심층 분석 완료 후 실제 공동 모형 추첨을 호출하도록 연결 수정.
- 랜덤 추첨 후에도 완료된 ORACLE 분석을 재사용하여 불필요한 재계산 방지.
- 일반 분석의 균등 전환과 심층 모형의 미검증 상태를 구분하여 표시.
- `--open CSV`로 데이터를 불러오며 GUI 시작.
- Fortuna ORACLE Engine 1.2.0: 분석 상태를 보존하는 GenerateUniform API 추가. ABI 1.0 호환 유지.
- 앱과 엔진 실행 로직은 x64 어셈블리. 같은 저장소의 engine/ 폴더와 단일 EXE 배포 유지.

Windows x64용 ZIP을 풀고 FebiusFortuna.exe를 실행합니다. Python 및 별도 엔진 DLL 설치는 필요하지 않습니다.
