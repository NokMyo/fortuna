# Febius Fortuna 1.7.1 / Engine 1.5.1

Windows 백신의 휴리스틱 오탐 가능성을 낮추기 위한 바이너리 정리 업데이트입니다.

- 비공식 `SystemFunction036/RtlGenRandom` 호출을 문서화된 Windows CSPRNG `BCryptGenRandom`으로 교체
- 릴리스 EXE/DLL에 PE 체크섬 생성 및 기존 ASLR·High Entropy VA·NX 호환 플래그 검증 강화
- Febius Account HTTP User-Agent를 현재 앱 버전으로 정리
- CI에서 legacy RNG import 부재, BCRYPT import, PE 보안 플래그와 체크섬을 자동 검사

ORACLE 분석 수식, 계정·라이선스 정책, 저장 데이터 형식은 변경하지 않았습니다.
이 업데이트는 특정 백신의 탐지 해제를 보장하지 않으며 코드 서명을 추가한 버전은 아닙니다.
