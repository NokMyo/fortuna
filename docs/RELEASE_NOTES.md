Windows용 Febius Fortuna 1.0.0입니다. Windows 10 1607 이상 / Windows 11 x64를 지원합니다.

- x86-64 어셈블리어로 작성한 단일 실행 파일. 별도 C/C++ 런타임이나 설치가 필요 없습니다.
- Febius 계열의 클래식 회색 Win32 화면, 한글 메뉴, 1·5·10게임 생성과 복사·저장.
- ORACLE 전수 탐색, Bayesian·시간·pair/triple·spectral·구조 분석, 이웃 및 가상 이력 안정성 계산.
- 회차별 백테스트와 별도 30회차 확인을 통과한 경우에만 가중 정책 활성화. 근거가 부족하면 균등 생성.
- 데이터·설정·후보장 SHA-256, 상세 보고서, 로컬 공식 번호 봉인.

`FebiusFortuna.exe`는 바로 실행할 수 있습니다. ZIP에는 실행 파일, 사용 설명서, 구현 명세와 **가상 CSV 예제**가 포함됩니다. `SHA256SUMS.txt`로 파일을 확인할 수 있습니다.

프로그램 1.0.0은 ORACLE reference 구현입니다. 설계 문서의 자동 데이터 공급, GMM/HMM, 고급 ablation, AVX/GPU, 외부 공증 등 모든 장기 항목을 완료한 버전은 아닙니다. 정확한 범위는 [구현 명세](https://github.com/NokMyo/fortuna/blob/main/docs/IMPLEMENTATION.md)에 정리했습니다.

오라클 지수와 내부 샘플링 확률은 실제 당첨확률이 아닙니다. 공정한 추첨에서 모든 조합의 1등 확률은 같습니다.
