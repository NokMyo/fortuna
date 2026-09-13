# Febius Fortuna

**분석적 무작위 생성기 — Fortuna ORACLE**

Fortuna는 한국 로또 6/45의 과거 추첨 데이터를 분석해 전체 **8,145,060개 조합**을 전수 탐색하고, 자체 ORACLE 점수 지형에서 번호를 다시 샘플링하는 Febius 실험 제품입니다.

프로그램 본체는 **x86-64 어셈블리어만으로 작성**합니다. C/C++ 런타임은 사용하지 않으며 Win32 API를 직접 호출합니다.

현재 `ORACLE ASM 0.1`은 CSV 검증/파싱, 번호별 장·중·단기 통계, 45×45 번호쌍 그래프, 구조 분포, 최근 회차 중복 억제, 사람 선택 패턴 회피, 814만 조합 전수 1차 탐색, 4,096개 정밀 평가, 512개 최종 후보군, 가중 재계산, 적응형 회의주의(skepticism), 번호 persistence까지 포함합니다.

## 빌드

Windows x64에서 LLVM `clang`과 `lld-link`, Windows SDK가 PATH에 있는 개발자 명령 프롬프트를 사용합니다.

```bat
build.bat
```

결과물은 `build\FebiusFortuna.exe`입니다. `main` push마다 GitHub Actions에서도 Windows x64 빌드를 검증하고 실행 파일 artifact를 생성합니다.

## 데이터

앱에서 `Load CSV`를 눌러 과거 회차 파일을 불러옵니다. 지원 형식은 `data/FORMAT.md`에 정의되어 있습니다. 특정 비공식 웹 API에 엔진을 묶지 않기 위해 데이터 공급 계층과 분석 코어를 분리했습니다.

## Recalculate

`Analyze`는 해당 데이터셋을 기준으로 ORACLE 확률 지형을 한 번 만듭니다. `Recalculate`는 분석을 다시 학습하는 버튼이 아니라 **같은 지형에서 새로운 조합을 샘플링**하는 버튼입니다. 따라서 결과 숫자는 바뀌어도 후보군의 통계적 성격은 유지됩니다.

## 중요한 전제

정상적인 무작위 6/45 추첨에서는 모든 6개 조합의 실제 1등 당첨확률이 동일합니다. ORACLE Index, score, persistence는 Fortuna 내부 모델의 상대 지표이며 실제 당첨확률이나 당첨 보장을 뜻하지 않습니다.

수학 및 공학 설계는 [`docs/ORACLE.md`](docs/ORACLE.md)를 참고하세요.
