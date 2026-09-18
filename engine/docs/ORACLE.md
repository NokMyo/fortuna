# Febius Fortuna ORACLE — Master Engineering Specification

> 1.0.0 릴리스의 실제 구현 범위와 차이는 [IMPLEMENTATION.md](IMPLEMENTATION.md)를 기준으로 확인하세요. 이 문서의 전체 설계·연구 목표가 모두 구현되었다는 의미는 아닙니다.

> 상태: **Canonical / Master Specification**  
> 대상: Febius Fortuna 최종 ORACLE 엔진  
> 구현 원칙: **Windows x86-64 Assembly 중심, C/C++ CRT 비의존, 재현 가능하고 검증 가능한 분석적 무작위 생성기**  
> 문서 목적: 현재 구현, 최종 필수 기능, 연구·검증 기능, 성능 최적화, 향후 확장 기능을 하나의 기준 문서로 정의한다.

---

## 0. ORACLE의 정체성과 절대 원칙

Fortuna ORACLE은 로또 6/45의 미래 당첨번호를 초자연적으로 예언하는 시스템이 아니다. 공정하고 독립적인 6/45 추첨에서 가능한 모든 조합의 1등 당첨확률은 동일하다.

\[
\left|\Omega\right|={45 \choose 6}=8,145,060
\]

따라서 임의의 조합 \(X\in\Omega\)에 대해 공정 추첨의 실제 확률은

\[
P_{fair}(X)=\frac{1}{8,145,060}
\]

이다.

ORACLE이 만드는 것은 실제 당첨확률 그 자체가 아니라, 과거 추첨 데이터에서 측정 가능한 통계적 구조를 여러 독립 모델로 분석하고, 그 분석의 불확실성·과적합 가능성·귀무가설을 함께 고려하여 전체 8,145,060개 조합 위에 형성하는 **내부 분석 선호도 분포(analytical preference field)** 이다.

최종 ORACLE은 다음 원칙을 절대 위반하지 않는다.

1. **Uniform Null First** — 기본 가설은 항상 독립·균등 무작위다.
2. **No Future Leakage** — 미래 회차의 정보가 과거 예측에 단 한 비트도 들어가면 안 된다.
3. **Complexity Is Not Evidence** — 복잡한 모델이라는 이유만으로 높은 가중치를 주지 않는다.
4. **Shrink Toward Uniform** — 증거가 약하면 반드시 균등모델 쪽으로 수축한다.
5. **Uncertainty Must Survive** — 점수뿐 아니라 불확실성을 추적한다.
6. **Reproducibility** — 데이터 버전, 엔진 버전, 설정이 같으면 분석 결과를 재현할 수 있어야 한다.
7. **No Hidden Retrofitting** — 실제 추첨 후 과거 예측을 다시 계산하여 성능을 좋게 보이게 하는 행위를 금지한다.
8. **Validation Before Promotion** — 새 알고리즘은 Champion을 이기기 전까지 운영모델이 될 수 없다.
9. **Exact Enumeration When Feasible** — 6/45 전체 조합 공간은 현대 PC에서 전수 탐색 가능하므로 근사 탐색을 기본으로 사용하지 않는다.
10. **Analytical Randomness** — 최종 출력은 단일 결정값이 아니라 분석으로 만든 확률 지형에서 샘플링한다.

---

# 1. 전체 시스템 정의

하나의 회차 \(t\)에 대한 과거 데이터 집합을

\[
D_t=\{d_1,d_2,\dots,d_t\}
\]

라고 한다. 각 추첨은

\[
d_t=\{x_{t,1},x_{t,2},\dots,x_{t,6}\},\quad 1\le x_{t,1}<\cdots<x_{t,6}\le45
\]

로 표현한다.

다음 회차용 ORACLE 분석은

\[
D_t \rightarrow F_t \rightarrow \{M_1,\dots,M_K\} \rightarrow S_t(X) \rightarrow P_{oracle,t}(X) \rightarrow P_{final,t}(X)
\]

순으로 진행한다.

- \(F_t\): 데이터에서 추출한 특징 집합
- \(M_k\): 독립 또는 준독립 서브모델
- \(S_t(X)\): 후보 조합 \(X\)의 통합 ORACLE 점수
- \(P_{oracle,t}(X)\): ORACLE 분석 기반 조합분포
- \(P_{final,t}(X)\): 균등분포를 섞은 최종 샘플링 분포

최종 분포의 기본 형태는

\[
P_{final,t}(X)
=(1-\rho_t)P_{oracle,t}(X)
+\rho_t U(X)
\]

이다.

여기서

\[
U(X)=\frac1{8,145,060}
\]

이며 \(\rho_t\in[0,1]\)는 ORACLE이 자신의 분석을 얼마나 의심하는지를 나타내는 **Adaptive Skepticism coefficient**다.

---

# 2. 구현 상태 표기

이 문서의 기능은 다음 네 상태로 구분한다.

- **[CURRENT]** 현재 0.1 코드에 구현됨.
- **[REQUIRED]** 최종 ORACLE에 반드시 구현해야 함.
- **[RESEARCH]** 통계적 검증을 통과할 경우 핵심 모델로 승격할 연구 기능.
- **[EXTENSION]** 제품 확장성, 분석 편의, 성능 향상을 위한 선택 기능.

---

# 3. 데이터 계층

## 3.1 추첨 데이터 구조 [CURRENT → REQUIRED]

최소 필드:

- 회차 번호
- 추첨 날짜
- 본번호 6개
- 보너스 번호
- 데이터 공급처 ID
- 원본 레코드 해시

내부 저장 시 본번호 6개는 정렬된 6바이트 배열과 64비트 비트마스크 두 형태를 동시에 유지한다.

번호 \(n\)의 비트는

\[
B(n)=1\ll(n-1)
\]

로 정의하고 조합은

\[
Mask(X)=\bigvee_{n\in X}B(n)
\]

로 저장한다.

두 조합의 공통 번호 수는

\[
|A\cap B|=popcount(Mask(A)\land Mask(B))
\]

한 번으로 계산할 수 있다.

## 3.2 데이터 무결성 검증 [CURRENT → REQUIRED]

모든 입력 행에 대해 다음을 검사한다.

- 번호 개수 정확히 6개
- 모든 번호가 1~45 범위
- 중복 번호 없음
- 오름차순 정렬 가능 여부
- 회차 번호 중복 없음
- 날짜 역전 또는 비정상 점프 여부
- 보너스 번호가 본번호와 중복되지 않는지
- 데이터 공급처 간 동일 회차 불일치 여부

불일치 발생 시 해당 회차를 자동 채택하지 않고 `QUARANTINED` 상태로 둔다.

## 3.3 데이터 공급자 어댑터 [REQUIRED]

최종 엔진은 분석 코어와 데이터 취득 계층을 분리한다.

지원 우선순위:

1. 공식 또는 신뢰 가능한 HTTP 공급자
2. 캐시된 서명 데이터셋
3. CSV 수동 입력
4. 로컬 스냅샷

네트워크 실패가 분석 결과를 바꾸지 않도록, 분석 시점에는 항상 **고정된 Dataset Snapshot**을 먼저 생성한다.

## 3.4 Dataset Manifest [REQUIRED]

각 분석은 다음 메타데이터를 가진다.

```text
Dataset ID
Last draw number
Record count
Provider
Acquired timestamp
Canonical SHA-256
Parser version
Validation status
```

분석 ID는 최소한 다음을 해시한다.

\[
AnalysisID=SHA256(DatasetHash\|EngineVersion\|ConfigHash)
\]

---

# 4. 이론적 균등 기준선

공정한 6/45 추첨에서 한 특정 번호가 등장할 확률은

\[
p_1=\frac6{45}=\frac2{15}\approx0.133333
\]

이다.

특정 두 번호가 함께 등장할 확률은

\[
p_2=\frac{{43\choose4}}{{45\choose6}}=\frac1{66}\approx0.0151515
\]

이다.

특정 세 번호가 함께 등장할 확률은

\[
p_3=\frac{{42\choose3}}{{45\choose6}}
=\frac{6\cdot5\cdot4}{45\cdot44\cdot43}
\]

이다.

일반적으로 특정 \(r\)개 번호 집합이 한 추첨에 포함될 확률은

\[
p_r=\frac{{45-r\choose6-r}}{{45\choose6}}
\]

이다.

ORACLE은 raw count 자체보다 이 균등 귀무가설의 기대값에서 얼마나 벗어났는지를 주로 사용한다.

---

# 5. 번호 단위 특징 계층

## 5.1 장기 출현 잔차 [CURRENT]

번호 \(i\)의 전체 등장 횟수를 \(c_i\), 총 회차를 \(N\)이라 하면 기대 등장 횟수는

\[
E[c_i]=N\frac6{45}
\]

이다.

정수 hot-path용 중심화 잔차는

\[
R_i^{long}=45c_i-6N
\]

로 계산한다.

## 5.2 다중 시간척도 EWMA [CURRENT → REQUIRED]

번호별 시계열을

\[
y_{i,t}=\begin{cases}1&i\in d_t\\0&otherwise\end{cases}
\]

로 둔다.

시간척도 \(k\)의 EWMA는

\[
E_{i,t}^{(k)}=\lambda_kE_{i,t-1}^{(k)}+(1-\lambda_k)y_{i,t}
\]

로 계산한다.

최종 엔진 기본 밴드:

- ultra-short: 반감기 약 5회
- short: 약 15회
- medium: 약 50회
- long: 약 200회
- lifetime: 전체 누적

Assembly hot path에서는 Q16 또는 Q32 fixed-point로 구현한다.

반감기 \(h\)에 대한 감쇠계수는

\[
\lambda=2^{-1/h}
\]

이다.

## 5.3 Trend derivative [REQUIRED]

단순 현재 EWMA뿐 아니라 서로 다른 시간척도의 차이를 사용한다.

\[
Momentum_i=E_i^{short}-E_i^{long}
\]

\[
Acceleration_i=(E_i^{ultra}-E_i^{short})-(E_i^{short}-E_i^{medium})
\]

이를 통해 장기 평균 대비 최근 변화량을 분리한다.

## 5.4 Bayesian shrinkage [REQUIRED]

번호별 확률을 직접 raw frequency로 사용하지 않는다.

\[
p_i\sim Beta(\alpha,\beta)
\]

로 두고 prior mean이 이론적 출현확률 \(2/15\)가 되도록

\[
\frac{\alpha}{\alpha+\beta}=\frac2{15}
\]

를 만족하게 한다.

관측 후

\[
p_i|D\sim Beta(\alpha+c_i,\beta+N-c_i)
\]

를 사용한다.

Posterior mean:

\[
\hat p_i=\frac{\alpha+c_i}{\alpha+\beta+N}
\]

데이터가 적을수록 균등 기준으로 강하게 수축한다.

## 5.5 Gap / survival model [CURRENT → REQUIRED]

번호 \(i\)의 마지막 등장 이후 공백을 \(g_i\)라 한다.

단순히 “오래 안 나왔으니 나올 차례”라는 점수를 금지한다. 대신 역사적 gap 분포를 이용해 discrete hazard를 추정한다.

\[
h_i(g)=P(G_i=g\mid G_i\ge g)
\]

균등 독립모델에서는 매 회차 조건부 출현확률이 본질적으로 일정해야 하므로, 추정 hazard는 반드시 uniform hazard 쪽으로 shrink한다.

\[
\tilde h_i(g)=\eta_i(g)\hat h_i(g)+(1-\eta_i(g))\frac2{15}
\]

표본이 적을수록 \(\eta_i(g)\)를 낮춘다.

## 5.6 Gap surprise [REQUIRED]

현재 gap이 그 번호의 역사적 분포에서 얼마나 극단적인지

\[
Surprise_i=-\log P(G_i\ge g_i)
\]

로 계산하되, 예측 가중치는 낮게 유지한다. Surprise는 “다음에 나올 확률”이 아니라 현재 상태의 비정상성 지표다.

---

# 6. 번호 관계 그래프

## 6.1 Pair residual graph [CURRENT]

번호 \(i,j\)의 동시출현 횟수를 \(c_{ij}\)라 하면 귀무가설 기대값은 \(N/66\)이다.

정수 residual:

\[
R_{ij}^{pair}=66c_{ij}-N
\]

후보 조합 \(X\)의 pair score는

\[
S_{pair}(X)=\sum_{i<j,\;i,j\in X}f(R_{ij}^{pair})
\]

이다. 6개 조합은 pair가 정확히 15개이므로 계산비용이 작다.

## 6.2 Time-decayed pair graph [REQUIRED]

오래된 관계와 최근 관계를 구분하기 위해

\[
C_{ij}^{(\lambda)}=\sum_{t=1}^{N}\lambda^{N-t}\mathbf{1}[i,j\in d_t]
\]

를 계산한다.

## 6.3 PMI / normalized association [REQUIRED]

raw co-occurrence가 개별 빈도 증가 때문인지 분리한다.

\[
PMI(i,j)=\log\frac{P(i,j)}{P(i)P(j)}
\]

희소 카운트에는 additive/Bayesian smoothing을 적용한다.

## 6.4 Standardized pair residual [REQUIRED]

귀무가설에서 pair count를 Binomial 근사하면

\[
Z_{ij}=\frac{c_{ij}-Np_2}{\sqrt{Np_2(1-p_2)}}
\]

를 계산할 수 있다.

실제 모델에는 raw \(Z\)를 무제한 사용하지 않고 winsorization 또는 tanh형 soft clamp를 적용한다.

## 6.5 Graph spectral features [RESEARCH]

45×45 관계행렬 \(A\)에 대해 중심화·정규화 후 고유값/고유벡터 구조를 분석한다.

목표:

- 특정 번호 군집이 반복적으로 나타나는지
- 관계 그래프의 저차원 구조가 균등 시뮬레이션과 구별되는지
- 구조가 bootstrap 샘플에서도 안정적인지

고유구조가 random null graph에서도 동일 빈도로 나타난다면 해당 기능은 운영 점수에 반영하지 않는다.

---

# 7. Hypergraph 관계

## 7.1 Triple co-occurrence [REQUIRED]

세 번호 \((i,j,k)\)의 동시출현 횟수를 저장한다.

가능한 triple 수는

\[
{45\choose3}=14,190
\]

으로 충분히 관리 가능하다.

Triple residual은

\[
R_{ijk}=c_{ijk}-Np_3
\]

기반으로 계산하되 표본 희소성 때문에 pair보다 훨씬 강한 shrinkage를 적용한다.

## 7.2 Empirical-Bayes triple shrinkage [REQUIRED]

\[
\tilde R_{ijk}=w_{ijk}R_{ijk}
\]

\[
w_{ijk}=\frac{n_{ijk}}{n_{ijk}+\tau}
\]

형태로 관측량이 적은 triple 신호를 거의 0으로 수축한다.

## 7.3 4-way 이상 [RESEARCH]

직접 카운트 기반 4-way 이상은 표본 대비 상태공간이 너무 커 운영모델에 기본 적용하지 않는다. 필요한 경우 low-rank factorization 또는 log-linear interaction model로만 연구한다.

---

# 8. 조합 구조 특징

조합 \(X=\{x_1<\cdots<x_6\}\)를 단순 6개 숫자가 아니라 고차원 특징벡터

\[
\phi(X)\in\mathbb{R}^d
\]

로 변환한다.

최종 목표 차원은 대략 50~150개다.

## 8.1 기본 구조 특징 [CURRENT]

- 합계 \(\sum x_i\)
- 평균
- 범위 \(x_6-x_1\)
- 홀수 개수
- 저번호/고번호 개수
- 인접 연속수 개수
- 최근 회차 중복 수

## 8.2 확장 구조 특징 [REQUIRED]

- 분산, 표준편차
- 중앙값
- 사분위 범위
- 최소/최대 gap
- gap 평균/분산
- gap skewness 근사
- 1~9 / 10~19 / 20~29 / 30~39 / 40~45 구간 점유
- 동일 끝자리 충돌수
- 동일 십의자리 충돌수
- 소수 개수
- 제곱수 개수
- Fibonacci-like 특수집합 개수는 예측용이 아니라 crowd/anti-pattern 분석용으로만 사용
- 직전 1/3/5/10회 재출현량
- pair score 평균/분산/최대값
- triple score
- number-level trend 집계
- gap-surprise 집계
- crowd score
- entropy score
- historical-density score

## 8.3 Gap vector

\[
g(X)=(x_2-x_1,\dots,x_6-x_5)
\]

를 직접 특징으로 사용한다.

## 8.4 Structural histogram model [CURRENT → REQUIRED]

과거 실제 당첨 조합의 홀짝수, 저고수, 인접수, 합계, 범위 등의 empirical distribution을 기록한다.

후보 조합이 과거 분포에서 얼마나 자연스러운지를 구조 점수로 계산하되, 이 점수는 **실제 미래 확률의 증거로 취급하지 않는다.**

---

# 9. 정보이론 계층

## 9.1 Bucket entropy [CURRENT → REQUIRED]

번호 영역을 \(K\)개 구간으로 나누고 각 구간 점유율을 \(p_k\)라 할 때

\[
H(X)=-\sum_{k=1}^{K}p_k\log p_k
\]

를 계산한다.

현재 0.1의 cheap collision proxy는 Stage A에서 유지하고, 정밀 단계에서는 실제 entropy를 계산한다.

## 9.2 Entropy typicality [REQUIRED]

entropy가 높을수록 무조건 좋은 것이 아니다. 과거 추첨 entropy 분포의 중심과 폭을 추정하고

\[
Z_H(X)=\frac{H(X)-\mu_H}{\sigma_H}
\]

로 표준화한다.

극단적으로 낮거나 높은 entropy 모두 과도한 구조로 판단할 수 있다.

## 9.3 Maximum entropy principle [REQUIRED]

모델이 설명하지 못하는 자유도에는 임의 규칙을 추가하지 않는다. 제약조건으로 설명되지 않는 부분은 최대 엔트로피, 즉 가장 덜 가정적인 분포를 유지한다.

---

# 10. Historical density / Typicality / Novelty

## 10.1 Robust feature normalization [REQUIRED]

각 특징 \(f\)는 평균·표준편차만 쓰지 않고 median/MAD 기반 robust normalization을 우선 사용한다.

\[
Z_f=\frac{f-median(f)}{1.4826\cdot MAD(f)}
\]

## 10.2 Mahalanobis structural distance [REQUIRED]

상관된 구조특징을 동시에 고려하여

\[
D_M(X)=\sqrt{(\phi(X)-\mu)^T\Sigma^{-1}(\phi(X)-\mu)}
\]

를 계산한다.

공분산 행렬은 shrinkage covariance를 사용하여 불안정한 역행렬을 피한다.

## 10.3 Gaussian Mixture density [RESEARCH]

과거 조합 구조가 단일 구름이 아니라 여러 모드로 구성될 가능성을 모델링한다.

\[
p(\phi)=\sum_{k=1}^{K}\pi_k\mathcal N(\phi\mid\mu_k,\Sigma_k)
\]

모델 선택은 BIC/AIC뿐 아니라 walk-forward out-of-sample 성능으로 결정한다.

## 10.4 Typicality–Novelty balance [REQUIRED]

너무 평균적인 조합만 추천하지 않도록

\[
S_{TN}(X)=a\cdot Typicality(X)+b\cdot Novelty(X)-c\cdot Extremeness(X)
\]

구조로 균형을 잡는다.

---

# 11. 최근 상태 / Regime 계층

## 11.1 Window comparison [REQUIRED]

최근 \(w\)회와 장기 분포의 차이를 측정한다.

예:

\[
D_{KL}(P_{recent}\|P_{long})
\]

또는 Jensen–Shannon divergence를 사용한다.

## 11.2 Hidden Markov Model [RESEARCH]

관측되지 않는 구조 상태를

\[
Z_t\in\{1,\dots,K\}
\]

로 둔다.

상태 전이:

\[
P(Z_t=j\mid Z_{t-1}=i)=A_{ij}
\]

관측값은 합계, 범위, 홀짝, gap entropy 등이다.

HMM이 균등 시뮬레이션에서도 동일한 “상태”를 만들어내는지 반드시 비교한다.

## 11.3 Bayesian Change Point Detection [RESEARCH]

데이터 생성과정의 구조가 달라졌는지를

\[
P(Change_t\mid D)
\]

로 추정한다.

실제 장비/규칙 변경과 일치하지 않는 change point는 우연한 변동일 가능성이 높으므로 높은 임계값을 사용한다.

---

# 12. Crowd Avoidance 모델

이 계층은 **당첨확률을 높이는 모델이 아니다.** 당첨됐을 때 다른 사람이 같은 조합을 골랐을 가능성을 낮추는 방향의 독립 목적함수다.

## 12.1 기본 crowd 특징 [CURRENT]

- 1~31 생일 범위 과집중
- 5, 10, 15 등 둥근 수 과다
- 같은 끝자리 과다
- 산술적 패턴

## 12.2 확장 crowd model [REQUIRED]

가능하면 실제 공개 선택성향 연구/판매 데이터가 확보될 때만 데이터 기반으로 교정한다.

추정 목표:

\[
C(X)=P(Human\ chooses\ X)
\]

Crowd avoidance score:

\[
S_{crowd}(X)=-\log(C(X)+\epsilon)
\]

## 12.3 Grid-pattern detector [EXTENSION]

실제 로또 용지의 번호 배치에서 인간이 직선, 대각선, 사각형, 대칭 등을 선택하는 패턴을 별도 탐지한다.

---

# 13. Anti-pattern 모델

후보가 지나치게 인공적이거나 사람들이 의도적으로 고르기 쉬운 규칙을 가지는지 탐지한다.

필수 탐지:

- 완전 연속열
- 등차수열
- 반복 간격
- 동일 끝자리 군집
- 십의자리 한 구간 과집중
- 지나친 홀수/짝수 편향
- 지나친 저/고번호 편향
- 단순 배수 패턴
- 시각적 그리드 패턴

단, anti-pattern 점수는 실제 당첨확률이 낮다는 뜻으로 해석하면 안 된다.

---

# 14. Null Hypothesis Engine

ORACLE의 핵심 안전장치다.

귀무가설:

\[
H_0:\text{각 회차는 독립이며 45개 중 6개가 균등하게 추출된다.}
\]

모든 발견된 패턴은 반드시 다음 질문을 거친다.

> “완전히 균등한 가상 로또에서도 이 정도 패턴이 얼마나 자주 생기는가?”

## 14.1 Monte Carlo null generation [REQUIRED]

수백만~수억 개의 synthetic fair-history를 생성하여 각 통계량의 귀무분포를 근사한다.

\[
T_{obs}=T(D)
\]

\[
p=\frac{1+\sum_b\mathbf1[T(D_b^*)\ge T_{obs}]}{B+1}
\]

## 14.2 Exact tests where possible [REQUIRED]

번호 빈도, pair count 등 정확한 이항/초기하 분포가 가능한 영역은 Monte Carlo보다 exact 또는 analytic tail probability를 우선한다.

## 14.3 False Discovery Rate [REQUIRED]

45개 번호, 990개 pair, 14,190개 triple을 동시에 보면 우연한 “유의미한 패턴”이 대량 발생한다.

Benjamini–Hochberg 방식:

정렬된 p-value를

\[
p_{(1)}\le\cdots\le p_{(m)}
\]

라 할 때

\[
p_{(k)}\le\frac{k}{m}q
\]

를 만족하는 최대 \(k\)까지만 발견으로 인정한다.

운영 기본 FDR 목표 \(q\)는 보수적으로 설정한다.

---

# 15. 서브모델 구성

최종 ORACLE은 하나의 거대한 불투명 공식 대신 여러 서브모델의 ensemble로 구성한다.

최종 후보 서브모델:

1. Long Frequency Residual Model
2. Multi-scale Trend Model
3. Momentum/Acceleration Model
4. Bayesian Number Posterior Model
5. Gap/Survival Model
6. Pair Graph Model
7. Time-decayed Pair Model
8. Triple Hypergraph Model
9. Structural Distribution Model
10. Entropy Model
11. Historical Density Model
12. Typicality–Novelty Model
13. Recent Regime Model
14. Crowd Avoidance Model
15. Anti-pattern Model
16. Recent-overlap Model
17. Robustness Model
18. Uniform Null Model

Uniform Model은 반드시 ensemble 안에 존재한다.

---

# 16. 모델 출력 정규화

각 모델 \(M_k\)는 raw score \(r_k(X)\)를 출력한다.

모델마다 단위가 다르므로 robust standardization 후 사용한다.

\[
z_k(X)=\frac{r_k(X)-median(r_k)}{1.4826MAD(r_k)+\epsilon}
\]

극단값은

\[
\tilde z_k=c\tanh(z_k/c)
\]

같은 soft clipping으로 제한한다.

---

# 17. 모델 상관관계와 중복 신호 제거

## 17.1 Model correlation matrix [REQUIRED]

전체 후보 또는 validation sample에서

\[
C_{ij}=Corr(M_i,M_j)
\]

를 계산한다.

두 모델이 사실상 같은 신호를 보는 경우 둘 모두 높은 가중치를 받지 못하게 한다.

## 17.2 Redundancy penalty [REQUIRED]

모델 \(i\)의 독립성 계수를

\[
I_i=\frac1{1+\sum_{j\ne i}|C_{ij}|w_j}
\]

형태로 정의할 수 있다.

최종 유효 가중치:

\[
w_i^{eff}=w_iI_i
\]

## 17.3 PCA / factor grouping [RESEARCH]

모델 출력 공간을 factor로 분해하여 실질적으로 동일한 정보군을 묶고 factor 단위로 가중치를 부여하는 방식도 연구한다.

---

# 18. Ensemble weighting

## 18.1 기본 통합식 [REQUIRED]

\[
S_{ensemble}(X)=\sum_{k=1}^{K}w_k^{eff}\tilde z_k(X)
\]

단,

\[
w_k\ge0,\qquad \sum_kw_k=1
\]

를 기본 제약으로 한다.

## 18.2 Weight learning [REQUIRED]

가중치는 사람이 감으로 고정하지 않는다. Walk-forward validation에서 다음 다목적 손실을 최소화한다.

\[
L=\alpha L_{rank}+\beta L_{calibration}+\gamma L_{stability}+\delta L_{null}+\eta L_{complexity}
\]

- \(L_{rank}\): 실제 당첨번호가 후보지형에서 차지한 순위 관련 손실
- \(L_{calibration}\): 높은 내부 점수가 실제 out-of-sample 결과와 일관되는지
- \(L_{stability}\): 작은 perturbation에 순위가 과도하게 흔들리는지
- \(L_{null}\): 랜덤 기준선보다 실제로 나은지
- \(L_{complexity}\): 필요 이상 복잡한 모델에 대한 패널티

## 18.3 Bayesian Model Averaging [RESEARCH → TARGET]

최종적으로는 단일 고정 weight보다

\[
P(X|D)=\sum_mP(X|M_m,D)P(M_m|D)
\]

형태의 Bayesian model averaging을 목표로 한다.

---

# 19. Adaptive Skepticism

## 19.1 현재 구조 [CURRENT]

관측된 전체 번호빈도의 균등 기준선 편차가 약할수록 uniform random fallback 비율을 높인다.

## 19.2 최종 구조 [REQUIRED]

Skepticism \(\rho_t\)는 하나의 통계량이 아니라 다음을 종합한다.

- out-of-sample ORACLE 대 random 성능 차이
- calibration error
- bootstrap rank instability
- 모델 간 disagreement
- FDR 통과 신호 수
- 데이터 양
- 최근 regime uncertainty
- champion validation confidence

예시:

\[
\rho_t=\sigma(a_0+a_1E_{cal}+a_2U_{boot}+a_3D_{model}-a_4S_{validated})
\]

여기서 \(\sigma\)는 logistic function이다.

강한 증거가 없으면 \(\rho_t\to1\), 즉 균등랜덤에 가까워진다.

---

# 20. 전체 8,145,060 조합 탐색

## 20.1 Stage A — Exact Fast Scan [CURRENT]

모든 조합을 정확히 열거한다.

\[
\forall X\in\Omega
\]

가벼운 fixed-point 특징만 사용하여 fast score를 계산하고 상위 4,096개를 min-heap으로 유지한다.

## 20.2 Stage B — Deep Evaluation [CURRENT → REQUIRED]

4,096개 survivor에 대해 무거운 특징을 계산하고 상위 512개 candidate field를 생성한다.

## 20.3 Stage C — Precision/Robust Evaluation [REQUIRED]

최종 버전에서는 Stage B 상위 수천 개를 대상으로

- bootstrap stability
- weight perturbation
- crowd model
- exact historical density
- hypergraph
- ensemble disagreement
- rank confidence interval

을 계산한다.

## 20.4 Stage D — Candidate Field Construction [REQUIRED]

단순 top-N이 아니라 서로 지나치게 유사한 후보가 후보장을 독점하지 못하도록 diversity-aware selection을 적용한다.

---

# 21. 후보 다양성 최적화

두 조합의 Jaccard similarity:

\[
J(A,B)=\frac{|A\cap B|}{|A\cup B|}
\]

여러 조합 세트 \(\mathcal S\)를 만들 때

\[
Objective(\mathcal S)=\sum_{X\in\mathcal S}S(X)-\lambda\sum_{A\ne B}J(A,B)
\]

를 최대화한다.

대안으로 공통 번호 개수 자체

\[
O(A,B)=|A\cap B|
\]

에 강한 비선형 패널티를 줄 수 있다.

---

# 22. 최종 점수와 확률 지형

## 22.1 Robust final score [REQUIRED]

\[
S_{raw}(X)=S_{ensemble}(X)+S_{crowd}(X)-P_{overfit}(X)
\]

여기에 robustness factor를 곱한다.

\[
S_{robust}(X)=S_{raw}(X)\cdot R(X)
\]

단, 곱셈이 점수 부호에 이상한 효과를 내지 않도록 실제 구현에서는 affine/positive transform 후 결합한다.

## 22.2 MAD normalization [REQUIRED]

\[
Z(X)=\frac{S(X)-median(S)}{1.4826MAD(S)+\epsilon}
\]

## 22.3 Temperature distribution [REQUIRED]

\[
P_{oracle}(X)=\frac{\exp(\gamma Z(X)/T)}{\sum_Y\exp(\gamma Z(Y)/T)}
\]

- 낮은 \(T\): 상위 후보 집중
- 높은 \(T\): 다양한 후보

온도는 임의 UI 슬라이더가 아니라 검증된 기본값을 사용하고, Expert Mode에서만 조절 가능하게 한다.

## 22.4 Uniform mixture [CURRENT → REQUIRED]

\[
P_{final}(X)=(1-\rho)P_{oracle}(X)+\rho U(X)
\]

이 식은 최종 ORACLE 철학의 핵심이다.

---

# 23. Recalculate의 정확한 정의

`Recalculate`는 모델을 다시 학습하거나 데이터를 바꾸는 기능이 아니다.

동일한 회차와 동일한 분석 스냅샷에서

\[
X^{(1)},X^{(2)},\dots\sim P_{final,t}
\]

처럼 새로운 샘플을 뽑는다.

따라서 여러 번 돌려도 완전히 독립적인 생난수가 아니라 **같은 분석 지형의 성격을 공유하는 서로 다른 조합**이 나오는 것이 정상이다.

---

# 24. Official Pick과 Session Picks [REQUIRED]

제품에는 두 개념을 분리한다.

## Official Pick

회차마다 한 번 생성되는 공식 기록용 조합.

- Round ID 고정
- Dataset ID 고정
- Engine version 고정
- Config hash 고정
- Seed 기록
- 추첨 전 immutable record 생성

## Session Picks

사용자가 Recalculate로 생성하는 분석적 샘플들.

Official Pick과 달리 여러 개 생성 가능하다.

---

# 25. 결정론적 재현과 난수

## 25.1 Official deterministic seed [REQUIRED]

공식 예측의 재현성을 위해

\[
Seed=SHA256("Fortuna"\|Round\|DatasetHash\|EngineVersion)
\]

를 기반으로 deterministic PRNG를 사용할 수 있다.

## 25.2 User Recalculate randomness [CURRENT]

Windows CSPRNG (`BCryptGenRandom` with `BCRYPT_USE_SYSTEM_PREFERRED_RNG`)를 사용한다.

Uniform 6/45 샘플 생성 시 modulo bias를 금지하고 rejection sampling을 사용한다.

---

# 26. Persistence / Stability

## 26.1 Candidate persistence [CURRENT]

최종 후보장 \(\mathcal C\)에서 번호 \(i\)가 포함되는 비율:

\[
Persistence(i)=\frac{1}{|\mathcal C|}\sum_{X\in\mathcal C}\mathbf1[i\in X]
\]

이 값은 실제 추첨확률이 아니다.

## 26.2 Perturbation stability [REQUIRED]

모델 파라미터, 데이터 bootstrap, weight를 조금씩 흔들어 \(B\)번 재계산한다.

\[
Stability(i)=\frac1B\sum_{b=1}^{B}\mathbf1[i\in TopK_b]
\]

조합에 대해서도

\[
Stability(X)=P(X\in TopK\mid perturbations)
\]

를 추정한다.

## 26.3 Robust Rank [REQUIRED]

점수 1위가 아니라 rank distribution을 추적한다.

\[
Rank_b(X)
\]

에서 median rank, 5~95% interval, Top-100 survival probability 등을 계산한다.

---

# 27. Bootstrap 검증

## 27.1 Block bootstrap [REQUIRED]

시간 의존성을 완전히 깨지 않도록 단순 iid bootstrap뿐 아니라 moving-block bootstrap도 고려한다.

각 bootstrap 데이터셋 \(D_b^*\)에서 전체 또는 축소 ORACLE을 다시 계산한다.

목표:

- 번호 점수 CI
- 모델 weight CI
- 후보 rank CI
- 구조 분포 불확실성

## 27.2 Confidence intervals [REQUIRED]

예:

\[
Score_i=0.63\quad CI_{95\%}=[0.41,0.78]
\]

처럼 내부 분석 불확실성을 저장한다.

---

# 28. Parameter perturbation / Adversarial robustness

[REQUIRED]

모델이 사소한 설정 변화에 붕괴하는지 검사한다.

실험 예:

- EWMA half-life ±20%
- ensemble weight ±10~20%
- 최근 5/10/20회 제거
- 역사 데이터 5~20% 부분 제거
- 구조모델 bandwidth 변경
- crowd penalty 변경

결과가 크게 달라지면 ORACLE confidence를 낮춘다.

[RESEARCH] 추가 adversarial test:

- 회차 순서를 부분 랜덤화
- 번호 라벨 permutation
- synthetic fair draws 삽입
- 일부 historical rows masking

진짜 신호라면 적절한 교란과 무관하게 어느 정도 구조를 유지해야 하고, 잘못된 패턴이라면 쉽게 붕괴해야 한다.

---

# 29. Walk-forward backtest

최종 ORACLE에서 가장 중요한 검증 체계다. [REQUIRED]

예를 들어 700회까지 데이터로 701회를 예측하고,

\[
D_{1:700}\rightarrow Prediction_{701}
\]

701회를 추가한 뒤

\[
D_{1:701}\rightarrow Prediction_{702}
\]

를 수행한다.

이 과정을 가능한 모든 역사 구간에서 반복한다.

모든 feature normalization, weight fitting, hyperparameter selection도 해당 시점 이전 데이터만 사용해야 한다.

---

# 30. 성능 평가 지표

단순 “1등 맞췄냐”는 데이터가 너무 희소하여 모델평가 지표로 부적절하다.

최종 평가 세트:

## 30.1 Hit count distribution

예측 조합과 실제 당첨번호의 교집합

\[
H=|X_{pred}\cap X_{true}|
\]

0~6의 전체 분포를 기록한다.

## 30.2 Expected hit

균등 랜덤 한 조합의 기대 일치수는

\[
E[H]=6\cdot\frac6{45}=0.8
\]

이다.

ORACLE의 장기 평균이 0.8과 통계적으로 구별되는지 검사한다.

## 30.3 Rank of true combination

실제 당첨 조합이 8,145,060개 ORACLE 점수 순위 중 어디에 위치했는지 기록한다.

## 30.4 Number marginal ranking

실제 나온 번호들이 번호-level ranking에서 얼마나 위에 있었는지 평가한다.

## 30.5 Log score / Brier-like metrics [RESEARCH]

확률 지형이 정의되는 경우 proper scoring rule을 활용하여 calibration을 평가한다.

---

# 31. Random baseline comparison

ORACLE 성능은 반드시 대규모 균등 랜덤 baseline과 비교한다. [REQUIRED]

각 backtest 시점마다 동일한 수의 random predictor를 생성하고 ORACLE 통계량의 percentile을 계산한다.

\[
Z=\frac{Metric_{oracle}-\mu_{random}}{\sigma_{random}}
\]

그리고 empirical p-value를 함께 저장한다.

ORACLE이 랜덤과 구별되지 않으면 제품도 이를 그대로 표시해야 한다.

---

# 32. Calibration

내부 점수 900이 실제로 700보다 “더 좋은 내부 후보”로 일관되게 작동하는지 평가한다. [REQUIRED]

점수 구간별 out-of-sample 결과를 비교해 reliability curve를 만든다.

Calibration error가 커지면 해당 모델 및 전체 ORACLE confidence를 낮춘다.

중요: calibration은 실제 jackpot 확률을 0~100%로 표시하기 위한 것이 아니다.

---

# 33. ORACLE Index

사용자에게 `Confidence 87%`처럼 당첨확률로 오해될 표현은 금지한다.

대신 0~1000 범위의 **ORACLE Index**를 사용한다.

예:

```text
ORACLE Index        872 / 1000
Robust Rank         918 / 1000
Model Agreement     801 / 1000
Persistence         744 / 1000
Crowd Avoidance     903 / 1000
Null Significance   312 / 1000
Skepticism          61%
```

각 값은 내부 분석 지표이며 실제 당첨확률이 아니다.

---

# 34. Model Agreement / Disagreement

서브모델별 후보 평가 벡터가 얼마나 일치하는지 계산한다. [REQUIRED]

모델 간 rank correlation을 이용해

\[
Agreement(X)=1-Dispersion(\{rank_k(X)\})
\]

형태로 계산할 수 있다.

모델들이 서로 크게 다르면 높은 단일 점수가 있어도 confidence를 낮춘다.

---

# 35. Overfit detector

[REQUIRED]

다음 상황을 과적합 신호로 취급한다.

- train 성능만 크게 상승하고 walk-forward 성능은 정체
- hyperparameter를 조금만 바꿔도 top picks 급변
- historical shuffled data에서도 동일 성능
- 모델 복잡도 증가에 비해 out-of-sample 이득 없음
- 특정 기간에서만 성능 우수

Overfit penalty:

\[
P_{overfit}=f(TrainGap,Instability,NullSimilarity,Complexity)
\]

---

# 36. Champion–Challenger 체계

[REQUIRED]

운영 중인 안정 버전을 `Champion`, 새 연구 버전을 `Challenger`로 둔다.

Challenger 승격 조건 예:

- 완전한 walk-forward 재검증
- random baseline 대비 열화 없음
- calibration 개선 또는 동등
- rank stability 개선
- 모델 복잡도 증가 대비 성능 정당화
- 최소 관찰기간 충족

한두 회차 결과만으로 승격하지 않는다.

---

# 37. Engine versioning

버전은 모델 의미 변화에 따라 관리한다.

예:

```text
ORACLE 0.1  Assembly baseline
ORACLE 0.2  Validation foundation
ORACLE 0.3  Bayesian + survival
ORACLE 0.4  Hypergraph + robust ranks
ORACLE 0.5  Ensemble independence + calibration
ORACLE 0.6  Adaptive skepticism v2
ORACLE 0.7  Champion/challenger
ORACLE 0.8  Provider automation + immutable prediction ledger
ORACLE 0.9  Performance hardening
ORACLE 1.0  Validated production specification
```

알고리즘 버전과 앱 UI 버전은 분리한다.

---

# 38. Prediction Ledger

[REQUIRED]

모든 Official Pick은 추첨 전에 append-only ledger에 기록한다.

기록 필드:

```text
Target round
Created timestamp
Dataset hash
Engine version
Config hash
Candidate-field hash
Official pick
Seed
ORACLE Index components
Skepticism
Prediction record hash
```

레코드 해시는 이전 레코드 해시를 포함해 chain 형태로 만들 수 있다.

\[
H_t=SHA256(H_{t-1}\|Record_t)
\]

이렇게 하면 과거 결과 변조 여부를 확인할 수 있다.

---

# 39. 다중 조합 생성

[REQUIRED]

사용자가 5게임 등 여러 조합을 생성할 때 독립적으로 5번 뽑는 것보다 diversity-aware set sampling을 사용한다.

목표함수:

\[
\max_{X_1,\dots,X_m}
\sum_iS(X_i)-\lambda\sum_{i<j}OverlapPenalty(X_i,X_j)
\]

이렇게 하면 동일한 ORACLE 성격을 유지하면서도 티켓 간 과도한 중복을 줄인다.

---

# 40. 성능/컴퓨터 공학 사양

## 40.1 언어 [CURRENT]

핵심 앱과 ORACLE 코어는 Windows x86-64 Assembly를 기본으로 한다.

- C/C++ CRT 비의존
- WinAPI 직접 호출
- PE/COFF 네이티브 빌드

## 40.2 Fixed-point arithmetic [CURRENT → REQUIRED]

Stage A의 대량 계산은 float 대신 Q16/Q32 정수를 기본으로 한다.

장점:

- 예측 가능한 연산비용
- SIMD 친화성
- deterministic behavior
- 불필요한 runtime 제거

정밀 통계 계층에서는 필요한 경우 x87/SSE2/AVX scalar/vector floating-point를 직접 사용할 수 있다.

## 40.3 Bitmask [CURRENT]

6개 조합을 uint64 하나에 저장한다.

## 40.4 Cache-friendly data layout [REQUIRED]

AoS보다 hot-path에서는 SoA를 우선한다.

예:

```text
freq[45]
ewmaShort[45]
ewmaMid[45]
ewmaLong[45]
pair[45*45]
triple[C(45,3)]
```

## 40.5 Pair triangular packing [EXTENSION]

45×45 전체 행렬 대신 upper triangle 990개만 저장하는 packed representation을 지원한다.

## 40.6 Triple combinadic indexing [REQUIRED]

Triple을 hash map 없이 조합순위(combinadic rank)로 0~14,189 index에 직접 매핑한다.

## 40.7 SIMD [REQUIRED]

CPU 지원에 따라 dispatch:

- SSE2 baseline
- AVX2 optimized
- AVX-512 optional

Stage A에서 여러 후보의 특징 계산을 벡터화한다.

## 40.8 Multi-threading [REQUIRED]

조합 공간을 lexicographic range로 분할하여 worker thread에 배정한다.

각 worker는 local Top-K heap을 유지하고 마지막에 merge한다.

락 경합을 줄이기 위해 hot loop에서 global heap 사용을 금지한다.

## 40.9 NUMA-aware extension [EXTENSION]

고성능 시스템에서는 NUMA node별 local table과 candidate heap을 사용한다.

## 40.10 GPU accelerator [EXTENSION]

CPU Assembly 구현을 canonical reference로 유지하고, 선택적으로 DirectCompute/D3D12 compute backend를 추가할 수 있다.

GPU 결과는 CPU reference와 deterministic tolerance 검증을 통과해야 한다.

---

# 41. Precomputation

[REQUIRED]

회차 분석 시작 시 다음을 미리 계산한다.

- number scores 45개
- pair scores 990개
- triple scores 14,190개
- 구조 histogram lookup
- crowd lookup
- last-N draw masks
- bucket membership masks

후보 hot loop는 가능한 한 lookup + integer add + popcount 위주로 만든다.

---

# 42. Incremental update

[REQUIRED]

새 회차 하나가 추가될 때 전체 역사 데이터를 처음부터 재분석하지 않아도 되도록 incremental state를 유지한다.

업데이트 대상:

- frequency
- EWMA
- pair/triple counts
- gap state
- structural histograms
- recent mask ring buffer
- running moments

단, 주기적인 full rebuild로 누적 오류가 없는지 검증한다.

---

# 43. 수치 안정성

[REQUIRED]

- fixed-point saturation/overflow 검사
- 64비트 누적 사용
- 분산 계산 시 catastrophic cancellation 방지
- log(0) 방지를 위한 \(\epsilon\)
- covariance regularization
- softmax overflow 방지를 위한 max subtraction

Softmax:

\[
P_i=\frac{e^{z_i-z_{max}}}{\sum_je^{z_j-z_{max}}}
\]

---

# 44. 테스트 체계

## Unit tests [REQUIRED]

- combinadic enumeration 총 개수 8,145,060 검증
- 각 조합 uniqueness
- bitmask/popcount
- CSV parser
- pair/triple index
- uniform sampler bias test
- EWMA recurrence
- heap Top-K 정확성
- score deterministic replay

## Property tests [REQUIRED]

- 번호 permutation에 대한 균등 null invariance
- 동일 dataset/config에서 분석 hash 동일
- Recalculate 결과가 유효한 6/45 조합

## Statistical tests [REQUIRED]

균등 sampler를 대량 실행해 각 번호와 pair의 분포가 이론값과 일치하는지 chi-square / exact tolerance로 검사한다.

---

# 45. 데이터 누수 방지

[REQUIRED]

Backtest engine은 시점 \(t\) 이후의 다음 항목에 접근할 수 없어야 한다.

- 당첨번호
- 미래 구조통계
- 미래 normalization mean/std
- 미래 hyperparameter fit
- 미래 model weight

이를 코드 레벨에서 `CutoffRound`를 강제하여 방지한다.

---

# 46. 사용자 인터페이스 최종 기능

[REQUIRED]

메인 화면:

- 대상 회차
- 최신 데이터 회차
- Official Pick
- Recalculate
- 여러 게임 생성
- ORACLE Index
- Skepticism
- 후보장 크기
- 분석 소요시간
- Engine/Dataset version

분석 상세:

- 45개 번호 Persistence/Stability
- 모델별 점수
- 모델 Agreement
- 구조 지표
- Crowd Avoidance
- Random baseline percentile
- Backtest 요약

중요 문구:

> ORACLE 지수는 내부 모델의 분석 점수이며 실제 당첨확률이 아닙니다.

---

# 47. Expert Mode [EXTENSION]

일반 사용자에게는 숨기고 연구/개발용으로 제공한다.

- Temperature
- Skepticism override
- 모델별 enable/disable
- weight viewer
- null simulation count
- Top-K size
- bootstrap count
- time-scale half-life
- raw score export

공식 Official Pick은 기본 canonical config로만 생성 가능하게 한다.

---

# 48. 분석 리포트 내보내기 [EXTENSION]

회차마다 JSON/CSV/텍스트 리포트를 저장한다.

예:

```text
analysis/<round>/manifest.json
analysis/<round>/numbers.csv
analysis/<round>/candidates.csv
analysis/<round>/models.json
analysis/<round>/backtest.json
analysis/<round>/official.json
```

---

# 49. 모델 설명 가능성

[REQUIRED]

추천 조합마다 최종 점수뿐 아니라 기여도를 표시한다.

\[
Contribution_k(X)=w_k^{eff}\tilde z_k(X)
\]

따라서 “왜 이 조합이 나왔는가”를 모델별로 역추적할 수 있어야 한다.

---

# 50. 데이터 시각화 [EXTENSION]

- 번호 Stability heatmap
- pair graph
- score distribution
- candidate field histogram
- historical vs current regime
- backtest percentile chart
- calibration curve
- skepticism history

시각화는 분석을 설명하기 위한 것이며 예측력을 과장하면 안 된다.

---

# 51. ORACLE Research Sandbox [EXTENSION]

운영 엔진과 연구 코드를 분리한다.

새 모델은 sandbox에서만 실험하고 Champion config를 자동 변경하지 않는다.

연구 결과는 다음 메타데이터를 가진다.

```text
Experiment ID
Hypothesis
Train period
Validation period
Features
Hyperparameters
Random seed
Baseline
Metrics
Decision
```

---

# 52. Auto-pruning

[REQUIRED]

서브모델이 일정 기간 random baseline보다 의미 있는 기여를 못하면 weight를 자동 감소시킨다.

\[
w_k^{new}\propto w_k^{old}\exp(-\eta L_k)
\]

장기간 무의미하면 dormant 상태로 전환한다.

복잡한 모델을 계속 유지하는 것 자체가 목표가 아니다.

---

# 53. Drift monitoring

[REQUIRED]

각 회차 후 다음을 추적한다.

- feature distribution drift
- model output drift
- weight drift
- calibration drift
- candidate entropy drift
- skepticism drift

급격한 변화가 있으면 자동으로 `DIAGNOSTIC` 상태를 만든다.

---

# 54. Fail-safe modes

[REQUIRED]

다음 상황에서는 ORACLE 분석을 약화하거나 완전히 균등랜덤으로 복귀한다.

- 데이터 무결성 실패
- 데이터 공급자 불일치
- 모델 수치 오류
- candidate field 생성 실패
- backtest artifact 손상
- config hash 불일치
- 엔진 자체 self-test 실패

최종 fallback은 항상 유효한 uniform 6/45 generator다.

---

# 55. ORACLE 상태 머신

최종 앱 내부 상태:

```text
EMPTY
  ↓
DATA_LOADED
  ↓
VALIDATED
  ↓
FEATURES_READY
  ↓
FAST_SCAN
  ↓
DEEP_SCAN
  ↓
ROBUSTNESS_SCAN
  ↓
FIELD_READY
  ↓
OFFICIAL_LOCKED / SESSION_READY
```

오류 시:

```text
QUARANTINED
DIAGNOSTIC
UNIFORM_FALLBACK
```

---

# 56. 최종 ORACLE 수학적 형태

최종적으로 후보 \(X\)의 모델별 정규화 점수를 \(z_k(X)\), 검증된 모델 weight를 \(w_k\), 모델 독립성 보정을 \(I_k\), robustness를 \(R(X)\), overfit penalty를 \(O(X)\)라고 하자.

기본 분석점수:

\[
S(X)=\sum_{k=1}^{K}w_kI_kz_k(X)+\beta C(X)-\lambda O(X)
\]

Robust transform:

\[
S^*(X)=g(S(X),R(X))
\]

MAD 정규화:

\[
Z(X)=\frac{S^*(X)-median(S^*)}{1.4826MAD(S^*)+\epsilon}
\]

ORACLE 분포:

\[
P_{oracle}(X)=\frac{\exp(Z(X)/T)}{\sum_Y\exp(Z(Y)/T)}
\]

최종 분석적 무작위 분포:

\[
\boxed{
P_{Fortuna}(X)=
(1-\rho)P_{oracle}(X)+
\rho\frac1{8,145,060}
}
\]

이 식에서 \(\rho\)는 데이터와 검증 결과가 약할수록 커지고, 강한 out-of-sample 증거가 있을 때만 제한적으로 작아진다.

이것이 최종 Fortuna ORACLE의 핵심 정의다.

---

# 57. 구현 로드맵

## ORACLE 0.1 — 현재 기반 [CURRENT]

- x86-64 Assembly Win32 앱
- CSV 입력/검증
- 번호 빈도
- 3-band EWMA
- gap
- pair matrix
- 구조 histogram
- 8,145,060 전수 Stage A
- Top 4,096 heap
- Stage B
- Top 512 field
- Persistence
- Crowd/anti-pattern 기본형
- Recalculate
- CSPRNG uniform fallback
- Adaptive Skepticism v1

## ORACLE 0.2 — 검증 기반 [REQUIRED]

- walk-forward backtest
- random Monte Carlo baseline
- immutable prediction records
- exact performance metrics
- calibration framework
- model-level diagnostic export

## ORACLE 0.3 — Bayesian 계층 [REQUIRED]

- Beta posterior number model
- survival/hazard gap model
- time-decayed pair
- robust normalization
- posterior uncertainty

## ORACLE 0.4 — 관계/강건성 [REQUIRED]

- triple hypergraph
- bootstrap
- parameter perturbation
- Robust Rank
- candidate diversity optimizer

## ORACLE 0.5 — Ensemble intelligence [REQUIRED]

- model correlation matrix
- redundancy penalty
- learned ensemble weights
- auto-pruning
- model agreement

## ORACLE 0.6 — Skepticism v2 [REQUIRED]

- validation-driven \(\rho\)
- null significance input
- calibration uncertainty input
- model disagreement input
- fail-safe uniform collapse

## ORACLE 0.7 — Research governance [REQUIRED]

- Champion/Challenger
- experiment manifests
- promotion rules
- FDR correction

## ORACLE 0.8 — 데이터/제품화 [REQUIRED]

- provider adapter
- signed/cached snapshots
- Prediction Ledger
- Official Pick
- Session Picks
- multi-ticket optimizer

## ORACLE 0.9 — 극한 최적화 [REQUIRED]

- multithread exact scan
- AVX2
- optional AVX-512
- packed pair table
- combinadic triple index
- incremental update
- performance profiler

## ORACLE 1.0 — Production Canonical

1.0은 기능이 많아서가 아니라 다음 조건이 모두 충족될 때만 선언한다.

- Windows x64에서 reproducible build
- 전체 self-test 통과
- 완전한 walk-forward pipeline
- random baseline 비교
- no-leakage 검증
- immutable prediction ledger
- model calibration/robustness 보고서
- uniform fallback 보장
- UI에서 실제 확률과 내부 지수를 명확히 구분

---

# 58. 장기 확장 기능

## 58.1 다른 복권 규칙 지원 [EXTENSION]

엔진의 조합공간과 이론확률 계층을 일반화하여 `n choose k` 복권을 지원한다.

## 58.2 Serverless dataset sync [EXTENSION]

앱은 데이터만 받아오고 분석은 로컬 Assembly 엔진에서 수행한다.

## 58.3 Public prediction verification [EXTENSION]

Official Pick hash를 외부 공개 저장소에 추첨 전에 게시하여 사후변조가 불가능함을 증명할 수 있다.

## 58.4 Historical replay mode [EXTENSION]

특정 과거 시점으로 돌아가 당시 사용 가능했던 데이터만으로 ORACLE을 재생한다.

## 58.5 Research comparison mode [EXTENSION]

Champion과 여러 Challenger의 후보지형을 같은 화면에서 비교한다.

## 58.6 Portable core [EXTENSION]

Windows x64가 canonical 구현이지만 장기적으로 Linux x86-64용 시스템콜/API adapter를 별도 어셈블리로 추가할 수 있다.

---

# 59. 금지 사항

최종 Fortuna에서는 다음을 명시적으로 금지한다.

- 실제 당첨확률을 높인다고 단정하는 문구
- ORACLE Index를 당첨확률 %처럼 표시
- 미래 데이터가 포함된 백테스트
- 결과가 나온 뒤 과거 Official Pick 변경
- 통계적으로 실패한 모델을 마케팅 목적으로 유지
- 단일 회차 성과로 모델 승격
- random baseline 없는 성능 주장
- p-value 다중검정 보정 없이 “유의미” 주장
- 데이터 출처/버전이 없는 분석
- 사용자가 모르는 비밀 가중치 변경

---

# 60. 최종 정의

**Febius Fortuna ORACLE은 6/45의 8,145,060개 전체 조합을 직접 탐색하고, 번호 단위 시계열·Bayesian 수축·gap 생존분석·pair graph·triple hypergraph·조합 구조·정보이론·crowd behavior·강건성·다중모델 앙상블을 이용해 내부 분석 점수장을 만든다. 그 점수장은 walk-forward, bootstrap, Monte Carlo 귀무모델, FDR, calibration, model-correlation 제거와 Champion–Challenger 검증을 거치며, 신호가 약할수록 스스로 균등분포 쪽으로 복귀한다. 최종 출력은 이 검증된 분석 지형과 정확한 uniform 6/45 분포를 혼합한 확률분포에서 생성되는 분석적 무작위 조합이다.**

Fortuna의 목표는 “랜덤을 이긴다고 우기는 알고리즘”이 아니다.

**랜덤 데이터에서 가짜 질서를 만들어내지 않도록 자기 자신을 가장 엄격하게 의심하면서도, 계산 가능한 모든 구조를 끝까지 분석하는 엔진**이 최종 ORACLE의 목표다.
