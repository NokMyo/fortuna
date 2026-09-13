# Febius Fortuna ORACLE Field Architecture

> 상태: **Canonical Design Appendix / 구현 전 필수 참고 문서**  
> 대상: Fortuna ORACLE 최종 엔진의 독자 계산 체계  
> 범위: ORACLE Field Architecture(OFA), 독자 공식군, 계층 구조, 결합식, 검증식, 실패 조건, 구현 순서, 계산 최적화, 출력 지표  
> 구현 원칙: **모든 공식을 계산하되, 모든 공식을 자동으로 믿지 않는다.**

---

# 0. 문서 목적과 명명 규칙

이 문서는 Febius Fortuna ORACLE의 독자적인 계산 체계를 하나도 빠뜨리지 않고 구현 가능한 수준으로 정의한다. 여기서 사용하는 이름들은 Fortuna 프로젝트 내부의 공식 명칭이며, 수학적으로 완전히 새로운 학문적 정리를 주장하기 위한 것이 아니다. 일부 구성요소는 Bayesian statistics, spectral graph theory, information theory, robust statistics, ensemble learning, statistical physics, resampling, density estimation 등 기존 수학·통계 분야의 원리를 Fortuna 전용 방식으로 결합·정규화·검증·배치한 것이다.

공개 저장소에 수식과 파라미터가 모두 공개되면 타인이 구현을 재현하는 것은 원칙적으로 가능하다. 따라서 Fortuna의 실제 차별성은 단일 공식 하나가 아니라 다음 전체 체계에 있다.

1. 어떤 데이터를 어떤 상태공간으로 표현하는가.
2. 각 신호를 어떤 귀무모델과 비교하는가.
3. 불확실성을 어떻게 계산하고 감쇠하는가.
4. 서로 중복되는 모델을 어떻게 제거하는가.
5. 조합 주변의 기하학적 안정성을 어떻게 계산하는가.
6. 가상의 과거를 수천 개 생성했을 때도 살아남는지를 어떻게 측정하는가.
7. 모델 전체가 허용할 수 있는 총 확신량을 어떻게 제한하는가.
8. 마지막에 균등 무작위 분포를 얼마나 다시 섞는가.
9. 동일 분석장에서 재계산할 때 어떤 방식으로 다른 조합을 샘플링하는가.
10. 여러 게임을 하나의 집합으로 최적화할 때 중복을 어떻게 억제하는가.

최종 수학 체계의 공식 명칭은 다음으로 통일한다.

- **OFA — ORACLE Field Architecture**: 최종 전체 아키텍처.
- **FOFT — Fortuna ORACLE Field Theory**: OFA를 설명하는 수학적 관점/이론적 명칭. 내부 별칭으로 유지한다.
- **Fortuna ORACLE Hamiltonian**: 하나의 후보 조합을 전체 독자 공식군으로 통합 평가하는 최종 에너지 함수.
- **Fortuna Probability Field**: Hamiltonian에서 변환된 8,145,060개 조합 전체의 내부 분석 선호도 분포.

---

# 1. 절대 전제

로또 6/45의 조합 공간은

\[
\Omega=\{X\subset\{1,\ldots,45\}:|X|=6\}
\]

이고,

\[
|\Omega|={45\choose6}=8,145,060
\]

이다.

공정하고 독립적인 실제 추첨에서는 모든 조합의 실제 1등 확률이 동일하다.

\[
P_{fair}(X)=\frac1{8,145,060}
\]

따라서 ORACLE의 모든 계산은 **실제 당첨확률을 직접 추정한다는 주장**이 아니라, 과거 데이터로부터 정의한 내부 분석적 선호도와 그 선호도의 불확실성·안정성·강건성을 계산한다.

모든 독자 공식은 다음 귀무가설을 출발점으로 한다.

\[
H_0:\text{draws are independent and uniformly random}
\]

ORACLE은 복잡성이 커질수록 더 강하게 이 귀무가설로 되돌아갈 수 있어야 한다.

---

# 2. 전체 계층 구조

최종 ORACLE은 모든 공식을 한꺼번에 같은 레벨에서 더하지 않는다. 다음 6개 계층으로 분리한다.

## 2.1 Signal Layer

데이터 자체에서 분석 신호를 추출한다.

- FNG — Fortuna Null Gravity
- OTRK — ORACLE Temporal Resonance Kernel
- TPA — Temporal Phase Agreement
- FSL — Fortuna Spectral Lattice
- OHC — ORACLE Hyperphase Coupling
- OHPS — ORACLE Historical Phase Space
- FEB — Fortuna Entropic Balance
- OIS — ORACLE Interaction Synergy

## 2.2 Geometry Layer

후보 하나의 점수뿐 아니라 후보 주변의 조합 공간 기하학을 분석한다.

- OBG — ORACLE Basin Geometry
- FBV — Fortuna Basin Volume
- FSS — Fortuna Swap Sensitivity

## 2.3 Counterfactual Layer

데이터와 모델 조건을 조금씩 바꿔도 후보가 살아남는지 본다.

- COS — Counterfactual Orbit Stability
- FRP — Fortuna Rank Persistence
- Bootstrap / Block Bootstrap
- Parameter Perturbation
- Data Deletion Perturbation

## 2.4 Skepticism Layer

앞의 모든 신호를 의심하고, 중복·착시·과적합을 제거한다.

- FECL — Fortuna Evidence Conservation Law
- MOM — Model Orthogonality Matrix
- OMD — ORACLE Mirage Detector
- OCSG — ORACLE Causal Skepticism Gate
- FNG shrinkage
- FDR / multiple-testing correction
- Calibration / out-of-sample validation

## 2.5 Sampling Layer

최종 확률장과 균등분포를 결합해 샘플링한다.

- Fortuna ORACLE Hamiltonian
- OSDI — ORACLE Self-Doubt Index
- ATF — Adaptive Thermal Field
- Adaptive Skepticism mixture
- Uniform fallback

## 2.6 Output Layer

사용자가 실제로 받는 추천과 지표를 만든다.

- Recalculate
- Official Pick
- Session Pick
- FCA — Fortuna Constellation Algorithm
- ORACLE Signature
- Anchor/Strong/Moderate importance labels

---

# 3. FNG — Fortuna Null Gravity

## 3.1 목적

모든 관측 신호를 먼저 완전한 균등 6/45 귀무분포와 비교하고, 증거가 약하면 0으로 수축시킨다. 이것은 ORACLE 전체가 과거의 우연한 편차를 실제 신호처럼 과장하지 못하게 하는 기본 중력장이다.

## 3.2 기본 standardized null residual

특징 \(f_k(X)\)에 대해 균등 귀무모델의 기대값과 분산을 각각 \(E_0[f_k]\), \(Var_0[f_k]\)라 하면

\[
Z_k(X)=\frac{f_k(X)-E_0[f_k]}{\sqrt{Var_0[f_k]+\epsilon}}
\]

이다.

## 3.3 불확실성 수축

해당 신호의 추정 불확실성을 \(\sigma_k^2\), 신호의 prior strength를 \(\Lambda_k\)라 하면

\[
G_k(X)=Z_k(X)\cdot\frac{\Lambda_k}{\Lambda_k+\sigma_k^2}
\]

로 줄인다.

\(\sigma_k^2\to\infty\)이면 \(G_k\to0\)이다. 데이터가 불충분할수록 자동으로 균등모델로 돌아간다.

## 3.4 구현 규칙

- 모든 모델 출력은 최종 결합 전에 FNG 계열 표준화 또는 동등한 null-centered normalization을 통과해야 한다.
- 귀무분포를 해석적으로 얻을 수 있으면 해석식을 우선한다.
- 해석식이 어렵다면 Monte Carlo null table을 미리 생성한다.
- FNG를 통과하지 않은 raw frequency/count는 최종 Hamiltonian에 직접 넣지 않는다.

## 3.5 가중치 0 조건

- FDR 보정 후 유의성이 없음.
- bootstrap 신뢰구간이 넓음.
- walk-forward에서 랜덤 기준선 대비 개선 없음.
- 최근 데이터 일부 제거 시 방향이 반복적으로 반전됨.

---

# 4. OTRK — ORACLE Temporal Resonance Kernel

## 4.1 목적

고정된 최근 10회/30회 같은 창 대신 여러 시간척도에서 번호의 상태가 어떻게 변하는지 동시에 측정한다.

번호 \(i\)가 회차 \(s\)에 등장했는지

\[
y_{i,s}\in\{0,1\}
\]

로 둔다.

## 4.2 연속형 EWMA family

시간척도 \(\tau\)에 대해

\[
F_i(\tau)=\frac{\sum_{s=1}^{t}e^{-(t-s)/\tau}y_{i,s}}{\sum_{s=1}^{t}e^{-(t-s)/\tau}}
\]

를 정의한다.

권장 기본 scale family:

\[
\tau\in\{4,8,16,32,64,128,256,\infty\}
\]

실제 Assembly 구현에서는 지수함수를 매 회 계산하지 않고 scale별 고정 decay coefficient를 Q-format으로 precompute한다.

## 4.3 1차 Temporal Resonance

\[
R_i^{(1)}(\tau)=F_i(\tau)-F_i(2\tau)
\]

은 짧은 시간척도와 긴 시간척도의 차이를 나타낸다. Momentum에 해당한다.

## 4.4 2차 Temporal Curvature

\[
R_i^{(2)}(\tau)=F_i(\tau)-2F_i(2\tau)+F_i(4\tau)
\]

은 추세의 가속/감속을 나타낸다.

## 4.5 최종 번호 Temporal Resonance

\[
TR(i)=\sum_\tau a_\tau R_i^{(1)}(\tau)+b_\tau R_i^{(2)}(\tau)
\]

조합 수준에서는

\[
TR(X)=\sum_{i\in X}TR(i)+\eta_T\,TPA(X)
\]

처럼 사용할 수 있다.

## 4.6 구현 주의

OTRK는 미래 예측력이 있다고 선험적으로 믿지 않는다. 반드시 walk-forward 성능과 FNG/OMD를 통과해야 한다.

---

# 5. TPA — Temporal Phase Agreement

## 5.1 목적

여러 시간척도가 서로 같은 방향의 신호를 주는지 측정한다.

각 scale의 방향을 정규화해

\[
s_\tau(i)\in[-1,1]
\]

이라 하면

\[
TPA(i)=\frac{\left|\sum_\tau\omega_\tau s_\tau(i)\right|}{\sum_\tau\omega_\tau}
\]

이다.

모든 scale이 같은 방향이면 1에 접근하고, 서로 상쇄하면 0에 접근한다.

## 5.2 조합 수준

\[
TPA(X)=\frac1{6}\sum_{i\in X}TPA(i)-\lambda_{phase}Var\{s_\tau(i):i\in X\}
\]

으로 조합 내부의 시간상 일관성까지 반영할 수 있다.

---

# 6. FSL — Fortuna Spectral Lattice

## 6.1 목적

번호쌍 15개를 독립적으로 보는 것을 넘어 45개 번호 전체 관계망의 전역적 구조를 분석한다.

## 6.2 관계 행렬

번호 \(i,j\)의 관측 동시출현 횟수를 \(O_{ij}\), 균등모델 기대값을 \(E_{ij}\), 분산을 \(V_{ij}\)라 하면

\[
A_{ij}=\frac{O_{ij}-E_{ij}}{\sqrt{V_{ij}+\lambda_A}}
\]

로 45×45 대칭행렬 \(A\)를 만든다.

## 6.3 Spectral decomposition

\[
A=U\Lambda U^T
\]

로 고유값 분해한다.

극단적인 eigenmode를 제한하기 위해

\[
g(\lambda)=\frac{\lambda}{1+\alpha|\lambda|}
\]

를 적용한다.

## 6.4 조합 spectral score

조합 \(X\)를 45차원 binary vector \(x_X\)로 표현하면

\[
FSL(X)=x_X^TUg(\Lambda)U^Tx_X
\]

이다.

## 6.5 Assembly 구현 전략

실시간으로 eigen decomposition을 Assembly로 직접 구현하는 것은 가능하나 초기 버전에서는 비용이 크다. 권장 순서:

1. symmetric Jacobi eigen solver 구현.
2. 45×45이므로 분석 1회당 계산량은 충분히 감당 가능.
3. eigenvector와 transformed lattice matrix \(L=Ug(\Lambda)U^T\)를 한 번 생성.
4. 조합 평가 hot loop에서는 \(x^TLx\)만 계산.
5. 6개 번호이므로 실제 lookup은 6개 diagonal + 15 pair terms로 압축 가능.

## 6.6 실패 조건

spectral modes가 bootstrap마다 크게 회전하거나 eigenvalue rank가 불안정하면 FSL 가중치를 낮춘다.

---

# 7. OHC — ORACLE Hyperphase Coupling

## 7.1 목적

2차 pair 관계만으로 설명되지 않는 3번호 고유 상호작용을 측정한다.

## 7.2 Triple interaction information

\[
H_{ijk}=\log\frac{P(i,j,k)P(i)P(j)P(k)}{P(i,j)P(i,k)P(j,k)}
\]

을 기본 형태로 사용한다.

## 7.3 희소성 shrinkage

triple 관측 수가 적으므로

\[
\tilde H_{ijk}=\frac{n_{ijk}}{n_{ijk}+\lambda_H}H_{ijk}
\]

로 수축한다.

## 7.4 조합 점수

6개 조합에는 20개의 triple이 존재한다.

\[
OHC(X)=\sum_{i<j<k\in X}\tilde H_{ijk}
\]

## 7.5 다중검정

45개 중 3개 조합은

\[
{45\choose3}=14,190
\]

개이므로 false discovery 위험이 매우 크다. OHC는 반드시 Benjamini-Hochberg FDR 보정을 통과한 interaction 또는 empirical-Bayes shrinkage가 충분한 interaction만 유효하게 취급한다.

## 7.6 구현

triple index는 combinadic indexing을 사용하여 dense 14,190-entry 배열로 저장할 수 있다.

---

# 8. OHPS — ORACLE Historical Phase Space

## 8.1 목적

조합을 단순 여섯 숫자가 아니라 고차원 특징 벡터로 변환하고 과거 당첨 조합들이 형성한 상태공간 안에서 후보의 위치를 측정한다.

\[
\phi(X)\in\mathbb R^d
\]

권장 feature family:

- sum, mean, median, range
- variance, standard deviation, skewness
- odd/even count
- low/high count
- decade buckets
- gap vector statistics
- adjacency count
- repeated ending digits
- prime count
- square count
- recent overlap
- entropy family
- temporal resonance statistics
- spectral coordinates
- pair/hypergraph energy
- crowd-pattern features

## 8.2 Mahalanobis geometry

과거 feature mean과 covariance를 \(\mu,\Sigma\)라 하면

\[
D_M(X)=\sqrt{(\phi(X)-\mu)^T\Sigma^{-1}(\phi(X)-\mu)}
\]

이다.

Fortuna는 \(D_M\)이 무조건 작은 조합을 선호하지 않는다. 너무 전형적인 조합도 감점한다.

\[
OHPS(X)=-\frac{(D_M(X)-d_0)^2}{2\sigma_d^2}
\]

즉 목표 거리 \(d_0\) 주변의 ‘전형적이지만 지나치게 평균적이지 않은’ 영역을 선호한다.

## 8.3 GMM/KDE 확장

단일 Gaussian으로 부족하면

\[
p(\phi)=\sum_{k=1}^{K}\pi_k\mathcal N(\phi|\mu_k,\Sigma_k)
\]

형태의 GMM을 사용한다.

Novelty는

\[
Novelty(X)=-\log p(\phi(X))
\]

로 정의할 수 있으나, 너무 큰 novelty는 outlier이므로 다시 감점한다.

---

# 9. FEB — Fortuna Entropic Balance

## 9.1 목적

단일 구간 분포가 아니라 여러 partition에서 번호 분산의 구조적 균형을 동시에 측정한다.

partition family 예:

- 2구간: 1–22 / 23–45
- 3구간
- 5구간
- 9구간
- 십의 자리
- 끝자리

partition \(m\)에서 bucket proportion을 \(p_{m,j}\)라 하면

\[
H_m(X)=-\sum_jp_{m,j}\log p_{m,j}
\]

이다.

과거 당첨 조합의 entropy 평균과 표준편차를 \(\mu_{H_m},\sigma_{H_m}\)라 하면

\[
FEB(X)=\sum_m\alpha_m\left[-\frac{(H_m(X)-\mu_{H_m})^2}{2\sigma_{H_m}^2}\right]
\]

이다.

목표는 최대 entropy가 아니라 **역사적/귀무적 분포와 비교한 적절한 entropy**이다.

---

# 10. OIS — ORACLE Interaction Synergy

## 10.1 목적

번호 각각이 좋아 보이는 정도와, 여섯 번호가 함께 있을 때 생기는 상호작용 효과를 분리한다.

조합 전체 score를 \(S(X)\), 번호별 독립 base contribution을 \(S_1(i)\)라 하면

\[
OIS(X)=S_{interaction}(X)-\sum_{i\in X}S_1(i)
\]

또는 전체 모델 구조에 따라

\[
OIS(X)=S(X)-S_{independent}(X)
\]

로 정의한다.

OIS가 높으면 개별 번호가 특별하지 않더라도 조합으로 묶였을 때 pair/spectral/hypergraph/geometry 관점에서 강한 합성효과가 있음을 뜻한다.

## 10.2 순환 정의 방지

OIS를 Hamiltonian에 넣고 Hamiltonian으로 다시 OIS를 계산하는 순환은 금지한다. 반드시 pre-interaction base score와 interaction-enhanced score를 분리한다.

---

# 11. OBG — ORACLE Basin Geometry

## 11.1 핵심 철학

최고점 하나보다 주변까지 높은 ‘평평하고 넓은 고원’을 더 신뢰한다. 조합 하나가 우연히 매우 높은 isolated spike이면 감점한다.

## 11.2 1-swap neighborhood

조합 \(X\)에서 기존 번호 하나를 빼고 없는 번호 하나를 넣어 생성 가능한 이웃을

\[
\mathcal N_1(X)
\]

라 한다.

개수는

\[
6(45-6)=234
\]

개이다.

## 11.3 주변 평균과 분산

\[
\mu_N(X)=\frac1{|\mathcal N_1|}\sum_{Y\in\mathcal N_1(X)}S(Y)
\]

\[
\sigma_N^2(X)=\frac1{|\mathcal N_1|}\sum_Y(S(Y)-\mu_N)^2
\]

## 11.4 Basin score

\[
B(X)=\mu_N(X)-\lambda_B\sigma_N(X)
\]

## 11.5 Spike curvature

\[
C(X)=S(X)-\mu_N(X)
\]

지나치게 큰 positive curvature는 isolated spike일 수 있으므로

\[
OBG(X)=B(X)-\gamma_B\max(0,C(X)-C_0)
\]

로 감점한다.

## 11.6 확장

2-swap neighborhood를 부분 샘플링하여 더 넓은 basin 구조를 추정할 수 있다.

---

# 12. FBV — Fortuna Basin Volume

## 12.1 목적

후보 주변에 ‘높은 점수 영역’이 얼마나 넓게 존재하는지 측정한다.

반경 \(r\) neighborhood를 \(\mathcal N_r(X)\)라 하면

\[
FBV(X)=\log\sum_{Y\in\mathcal N_r(X)}\exp\left(\frac{S(Y)-S(X)}{T_B}\right)
\]

로 정의한다.

높은 FBV는 후보가 혼자 솟은 spike가 아니라 넓은 안정 영역에 속한다는 의미다.

## 12.2 계산 절약

전체 814만 후보에 FBV를 계산하지 않는다. Stage A/B survivor에만 적용하고 neighborhood lookup은 combinadic rank 또는 hash table을 사용한다.

---

# 13. FSS — Fortuna Swap Sensitivity

## 13.1 목적

최종 조합 내부에서 각 번호가 얼마나 중요한 역할을 하는지 계산한다.

번호 \(i\in X\)를 다른 번호 \(j\notin X\)로 교체한 조합을 \(X-i+j\)라 하면

\[
\Delta_{i\to j}=S(X)-S(X-i+j)
\]

이다.

번호 \(i\)의 swap sensitivity는

\[
FSS(i|X)=median_{j\notin X}\Delta_{i\to j}
\]

로 정의한다.

## 13.2 표시 등급

FSS를 candidate field 내 percentile로 변환하여 예:

- Anchor: 90 percentile 이상
- Strong: 70–90
- Moderate: 40–70
- Flexible: 40 미만

처럼 UI에 표시할 수 있다.

이는 당첨 가능성 등급이 아니라 ORACLE 내부 조합 의존도다.

---

# 14. COS — Counterfactual Orbit Stability

## 14.1 Fortuna 대표 독자 기술

한 개의 실제 역사만 보는 대신 역사와 모델 조건을 조금씩 바꾼 수천 개의 counterfactual world를 만들고, 후보가 그 세계들에서도 계속 상위권에 남는지를 측정한다.

## 14.2 Counterfactual world family

각 world \(D^{(b)}\)는 다음 중 하나 또는 여러 변형을 포함한다.

- 최근 5/10/20회 제거
- 과거 회차 1–10% random deletion
- contiguous block deletion
- ordinary bootstrap
- block bootstrap
- EWMA decay ±5/10/20%
- pair shrinkage 변화
- triple shrinkage 변화
- model weight perturbation
- 특정 서브모델 제거
- feature subset 제거
- historical window 변화

## 14.3 Orbit

world \(b\)에서 후보 \(X\)의 순위를 \(r_b(X)\)라 하면

\[
\mathcal O(X)=\{r_1(X),\dots,r_B(X)\}
\]

가 후보의 Counterfactual Orbit이다.

## 14.4 COS

\[
COS(X)=\exp\left(-\frac{MAD(r_b(X))}{\tau_r}\right)
\]

여기에 median rank 자체도 반영하려면

\[
COS^*(X)=COS(X)\cdot\exp\left(-\frac{median(r_b(X))}{\tau_m}\right)
\]

으로 확장한다.

## 14.5 구현 우선순위

COS는 계산량이 크므로 Stage C 최종 수천/수백 후보에서만 수행한다. Counterfactual world별로 전체 814만 전수 탐색이 필요한 버전과 top candidate 재평가 버전을 구분한다.

---

# 15. FRP — Fortuna Rank Persistence

bootstrap/perturbation world \(b\)에서 후보 순위가 K 이내인지 검사한다.

\[
FRP_K(X)=\frac1B\sum_{b=1}^{B}\mathbf1[rank_b(X)\le K]
\]

권장 지표:

- FRP10
- FRP100
- FRP512
- FRP4096

이를 다중 scale로 결합하면

\[
FRP(X)=\sum_K\omega_KFRP_K(X)
\]

이다.

FRP는 당첨확률이 아니라 **모델 교란에 대한 순위 생존확률**이다.

---

# 16. FECL — Fortuna Evidence Conservation Law

## 16.1 핵심 철학

**정보가 없는 곳에서 확신은 생성될 수 없다.**

모델을 5개에서 50개로 늘렸다고 데이터 자체의 정보량이 10배가 되어서는 안 된다.

## 16.2 Global evidence budget

전체 데이터가 귀무모델과 얼마나 차이가 있는지를 하나의 evidence budget \(E(D)\in[0,1]\)로 만든다.

예시 구조:

\[
E(D)=\sigma\left[aD_{KL}+bZ_{global}+cS_{stable}-dQ_{FDR}-eC_{cal}\right]
\]

여기서

- \(D_{KL}\): empirical structure와 null의 KL divergence
- \(Z_{global}\): global deviation statistic
- \(S_{stable}\): bootstrap stability
- \(Q_{FDR}\): false-discovery burden
- \(C_{cal}\): calibration error
- \(\sigma\): logistic/squashing function

## 16.3 Conservation constraint

최종 모델 가중치는

\[
\sum_k|w_k|\le E(D)
\]

또는 scale factor 형태로

\[
w_k^{final}=E(D)\frac{w_k^*}{\sum_j|w_j^*|+\epsilon}
\]

를 만족해야 한다.

## 16.4 효과

모델을 아무리 추가해도 데이터가 균등 랜덤과 거의 구별되지 않으면 모든 분석 신호의 전체 진폭이 낮아진다.

---

# 17. MOM — Model Orthogonality Matrix

## 17.1 목적

Trend, frequency, temporal resonance처럼 비슷한 정보를 보는 모델을 여러 번 세는 double counting을 방지한다.

후보 집합에 대한 모델 출력 상관도를

\[
C_{ij}=Corr(S_i(X),S_j(X))
\]

로 계산하고 공분산행렬을 \(\Sigma_M\)라 한다.

## 17.2 correlation-aware weight

각 모델의 out-of-sample quality를 \(q\)라 하면

\[
w^*=(\Sigma_M+\lambda I)^{-1}q
\]

를 기본 후보 가중치로 사용할 수 있다.

그 뒤 음수/과도한 weight에 constraint를 적용하고 FECL budget으로 다시 정규화한다.

## 17.3 ORACLE Orthogonality Principle

서로 거의 같은 모델 두 개가 같은 신호를 발견하는 것은 독립 증거 두 개가 아니다. 서로 독립적인 모델들이 같은 후보를 지지할 때만 ensemble consensus의 의미가 커진다.

---

# 18. OMD — ORACLE Mirage Detector

## 18.1 목적

겉보기에는 강한 패턴이지만 랜덤 데이터에서도 흔히 생기는 통계적 착시를 감지한다.

관측 통계량 \(A_{obs}\)에 대해 균등 random simulation을 생성해

\[
p_{null}=P(A_{random}\ge A_{obs})
\]

를 추정한다.

다수의 패턴을 동시에 검사한다면 Benjamini-Hochberg 등으로

\[
q_{FDR}
\]

를 계산한다.

## 18.2 Mirage strength

\[
M(A)=(1-q_{FDR})\cdot Stability(A)
\]

로 정의한다.

최종 penalty 형태는

\[
MiragePenalty(A)=1-M(A)
\]

이다.

## 18.3 규칙

- p-value 단독 사용 금지.
- effect size와 stability를 함께 사용.
- multiple testing 보정 필수.
- OHC/FSL/다중 feature 탐색에는 특히 강하게 적용.

---

# 19. OCSG — ORACLE Causal Skepticism Gate

## 19.1 목적

모델이 복잡한 설명은 만들지만 실제 out-of-sample 성능에는 기여하지 않는 경우 제거한다.

전체 모델의 validation loss/utility를 \(L\), 모델 \(k\)를 제거했을 때를 \(L_{-k}\)라 한다.

utility가 높을수록 좋은 정의를 사용하면

\[
\Delta_k=L-L_{-k}
\]

또는 loss가 낮을수록 좋은 정의에서는 부호를 반대로 정의한다.

핵심은 **모델을 제거했을 때 성능이 나빠지는가**이다.

## 19.2 Gate

\[
I_k=\begin{cases}
1,&\Delta_k>\delta_{min}\text{ and statistically stable}\\
0,&\text{otherwise}
\end{cases}
\]

최종 Hamiltonian에는

\[
w_kI_kS_k(X)
\]

형태로 들어간다.

## 19.3 soft gate 확장

binary gate 대신

\[
I_k=\sigma\left(\frac{\Delta_k-\delta_0}{T_g}\right)
\]

로 연속적으로 사용할 수 있다.

---

# 20. HSSM — Human Selection Shadow Model

## 20.1 목적

당첨확률을 높이는 모델이 아니라, 당첨됐을 때 다른 사람과 같은 조합을 선택했을 가능성을 줄이는 보조 모델이다.

사람의 선택 확률을

\[
P_{human}(X)
\]

로 근사한다.

feature 예:

- 1–31 생일 영역 편향
- 7 및 일부 선호수
- round number
- 연속수
- 동일 끝자리
- 산술수열
- 대칭
- 용지상 직선/대각선 패턴
- 모두 같은 십의 자리
- 지나치게 ‘예쁜’ 간격

## 20.2 Shadow score

\[
Shadow(X)=-\log(P_{human}(X)+\epsilon)
\]

## 20.3 엄격한 표시 규칙

실제 판매 티켓 데이터가 없다면 HSSM은 heuristic이다. `Crowd Avoidance`, `Human Selection Shadow`로 표시하며 실제 공동당첨 확률을 정확히 추정한다고 표현하지 않는다.

---

# 21. Fortuna ORACLE Hamiltonian

이것이 모든 독자 공식군을 하나의 에너지 함수로 합치는 최종 핵심식이다.

먼저 Signal Layer:

\[
\mathcal S_{signal}(X)=w_TOTRK(X)+w_PTPA(X)+w_SFSL(X)+w_HOHC(X)+w_QOHPS(X)+w_EFEB(X)+w_IOIS(X)
\]

Geometry Layer:

\[
\mathcal S_{geom}(X)=w_BOBG(X)+w_VFBV(X)+w_FFSS_{combo}(X)
\]

Counterfactual Layer:

\[
\mathcal R_{cf}(X)=w_CCOS(X)+w_RFRP(X)
\]

Skepticism penalties:

\[
\mathcal P_{skeptic}(X)=\lambda_MMirage(X)+\lambda_UUncertainty(X)+\lambda_OOverfit(X)+\lambda_DDisagreement(X)
\]

Crowd term은 실제 추첨 신호와 분리한다.

\[
\mathcal C(X)=\lambda_HShadow(X)
\]

최종 Hamiltonian:

\[
\boxed{
\mathcal H_F(X)=
-E(D)\left[\mathcal S_{signal}(X)+\mathcal S_{geom}(X)\right]
-\mathcal R_{cf}(X)
+\mathcal P_{skeptic}(X)
-\mathcal C(X)
}
\]

여기서 모든 \(w\)는 MOM, OCSG, FECL을 통과한 가중치다.

낮은 Hamiltonian이 높은 ORACLE 선호도를 의미한다.

---

# 22. OSDI — ORACLE Self-Doubt Index

## 22.1 목적

ORACLE이 자기 분석을 얼마나 믿지 말아야 하는지 명시적으로 계산한다.

## 22.2 구성요소

- Evidence weakness
- Calibration error
- Model disagreement
- Bootstrap variance
- Null similarity
- Out-of-sample degradation
- Drift score

정규화된 값을 사용하여

\[
OSDI=\sigma\left[a_1(1-Evidence)+a_2CalibrationError+a_3ModelDisagreement+a_4BootstrapVariance+a_5NullSimilarity+a_6Drift\right]
\]

으로 둔다.

\[
OSDI\in[0,1]
\]

0은 내부적으로 상대적으로 안정적, 1은 사실상 균등 랜덤으로 복귀해야 할 수준이다.

## 22.3 Uniform mixture coefficient

\[
\rho=\rho_{min}+(\rho_{max}-\rho_{min})OSDI
\]

이다.

---

# 23. ATF — Adaptive Thermal Field

## 23.1 목적

softmax temperature를 고정하지 않고 확률장의 엔트로피가 지나치게 낮거나 높지 않도록 자동 조정한다.

ORACLE Hamiltonian으로부터

\[
P_T(X)=\frac{e^{-\mathcal H_F(X)/T}}{\sum_Ye^{-\mathcal H_F(Y)/T}}
\]

를 정의한다.

분포 entropy:

\[
H(P_T)=-\sum_XP_T(X)\log P_T(X)
\]

목표 entropy \(H^*\)에 대해

\[
T^*=\arg\min_T|H(P_T)-H^*|
\]

를 찾는다.

## 23.2 구현

- monotonic한 구간에서 binary search 가능.
- probability underflow 방지를 위해 log-sum-exp 사용.
- Assembly에서는 score를 max/min shift하여 fixed/floating hybrid로 계산 가능.

---

# 24. 최종 Fortuna Probability Field

ATF가 찾은 \(T^*\)로

\[
P_{oracle}(X)=\frac{e^{-\mathcal H_F(X)/T^*}}{\sum_Ye^{-\mathcal H_F(Y)/T^*}}
\]

를 만든다.

그 뒤 OSDI가 정한 \(\rho\)로 균등분포를 섞는다.

\[
\boxed{
P_{Fortuna}(X)=(1-\rho)P_{oracle}(X)+\rho\frac1{8,145,060}
}
\]

이 식이 최종 생성 분포다.

중요: 이것은 실제 추첨확률이 아니라 ORACLE의 **샘플링 정책 분포**다.

---

# 25. Recalculate의 정확한 의미

같은 회차, 같은 Dataset Snapshot, 같은 Engine Version, 같은 Config에서는 분석장

\[
P_{Fortuna}(X)
\]

가 동일하다.

`Recalculate`는 분석을 처음부터 다시 만드는 기능이 아니라 **동일한 확률장에서 새로운 조합을 다시 샘플링하는 기능**이 기본이다.

따라서 다시 계산해도 다음 속성들이 통계적으로 반복되는 것이 정상이다.

- 특정 번호의 높은 Persistence
- 유사한 OTRK profile
- 유사한 spectral region
- 유사한 basin characteristics
- 유사한 entropy range
- 비슷한 ORACLE Signature distribution

`Deep Reanalyze`를 별도 기능으로 둘 경우에만 model perturbation 또는 새로운 data snapshot으로 분석장을 재구성한다.

---

# 26. FCA — Fortuna Constellation Algorithm

## 26.1 목적

여러 게임을 각각 독립 추출하지 않고, 높은 ORACLE 품질과 낮은 상호중복을 동시에 갖는 조합 집합을 선택한다.

선택할 집합을

\[
\mathcal S=\{X_1,\dots,X_m\}
\]

라 한다.

## 26.2 단순 목적함수

\[
FCA_1(\mathcal S)=\sum_i\log P_F(X_i)-\lambda\sum_{i<j}Overlap(X_i,X_j)
\]

## 26.3 Determinantal diversity

더 정교한 최종식은 similarity kernel \(K\)를 사용한다.

\[
\boxed{
FCA(\mathcal S)=\sum_i\log P_F(X_i)+\eta\log\det(K_{\mathcal S}+\epsilon I)
}
\]

Determinant가 클수록 서로 다른 방향의 조합들이 선택된다.

## 26.4 Kernel 예

\[
K_{ij}=\exp(-\alpha d_J(X_i,X_j))
\]

또는 Jaccard similarity / Hamming-like distance를 사용할 수 있다.

## 26.5 구현 전략

- 후보 field 상위 수천 개에서 greedy determinant gain.
- m이 작으므로 exact 또는 near-exact subset search도 가능.
- 사용자에게 5게임/10게임 생성 시 기본 사용.

---

# 27. ORACLE Signature

각 최종 조합에는 내부 분석 지문을 생성한다.

예시:

```text
ORACLE SIGNATURE
Temporal Resonance      0.731
Temporal Agreement      0.804
Spectral Lattice        0.864
Hyperphase Coupling     0.592
Historical Phase Space  0.748
Entropic Balance        0.742
Interaction Synergy     0.781
Basin Geometry          0.913
Basin Volume            0.852
Counterfactual Orbit    0.887
Rank Persistence        0.901
Human Shadow            0.816
Self-Doubt              0.417

FORTUNA FIELD INDEX      842 / 1000
```

`FORTUNA FIELD INDEX`는 실제 당첨확률이 아니다. 최종 Hamiltonian, robustness, percentile을 사용자용 0–1000 index로 변환한 내부 상대지표다.

권장 변환:

\[
FFI(X)=round(1000\cdot Percentile(-\mathcal H_F(X)))
\]

필요하면 robustness를 포함해

\[
FFI^*(X)=round(1000\cdot[aP_H(X)+bCOS(X)+cFRP(X)])
\]

처럼 정의한다.

---

# 28. 모든 공식을 동시에 사용하는 규칙

Fortuna는 최종적으로 이 문서의 모든 공식을 구현한다. 단, 다음 원칙을 따른다.

1. **계산 여부와 신뢰 여부를 분리한다.** 모델은 계산되더라도 validation 실패 시 weight=0이 될 수 있다.
2. **Raw score 직접 합산 금지.** 모든 출력은 null-centering, robust scaling, calibration을 거친다.
3. **MOM으로 중복을 제거한다.** 비슷한 모델 여러 개가 같은 신호를 반복 증폭하지 못한다.
4. **OCSG가 기여하지 않는 모델을 차단한다.** ablation에서 성능 기여가 없으면 gate를 닫는다.
5. **FECL이 전체 신뢰량을 제한한다.** 모델 수가 늘어도 evidence budget은 늘지 않는다.
6. **COS/FRP가 불안정한 고점을 제거한다.** 한 데이터셋에서만 높은 조합은 최종 단계에서 약해진다.
7. **OBG/FBV가 isolated spike를 제거한다.** 주변까지 안정된 고점을 선호한다.
8. **OSDI가 마지막 의심량을 결정한다.** 전체 시스템이 랜덤과 구별되지 않으면 uniform mixture가 지배한다.

---

# 29. Weight가 0으로 떨어지는 공통 조건

서브모델 \(M_k\)는 다음 중 하나 이상이면 자동 감쇠 또는 비활성화한다.

- Walk-forward performance ≤ matched random baseline.
- Corrected significance가 사전 threshold 미달.
- Effect size가 실질적으로 무시 가능한 수준.
- Bootstrap sign stability 미달.
- Parameter perturbation에서 방향이 불안정.
- 최근 window에서 calibration degradation 발생.
- 다른 모델과 상관도가 지나치게 높고 독립 기여가 없음.
- OCSG ablation에서 제거해도 성능이 같거나 향상됨.
- Drift detector가 해당 모델의 historical relationship 붕괴를 감지.

soft weight 예:

\[
w_k^{raw}=Performance_k\cdot Stability_k\cdot(1-q_k)\cdot Independence_k
\]

그 뒤 MOM/FECL을 적용한다.

---

# 30. Counterfactual 계산량 관리

모든 공식이 최종적으로 구현되어도 814만 조합 × 수천 world를 무식하게 계산하면 비효율적이다. 따라서 단계별 pruning을 사용한다.

## Stage A — Global Fast Field

- 8,145,060개 전수
- number base
- OTRK summary
- pair residual
- fast entropy proxies
- coarse structure
- simple crowd penalties

상위 약 4,096–65,536개 유지.

## Stage B — Deep Structural Field

- FSL
- OHC
- OHPS
- FEB full
- OIS

상위 약 512–4,096개 유지.

## Stage C — Geometry

- OBG
- FBV
- FSS

상위 수백–수천 개.

## Stage D — Counterfactual

- COS
- FRP
- bootstrap
- parameter perturbation

최종 후보 수백 개.

## Stage E — Skepticism & Final Field

- MOM
- OMD
- OCSG
- FECL
- OSDI
- ATF
- final sampling distribution

---

# 31. Robust normalization

모델별 score를 평균/표준편차만으로 정규화하면 outlier 영향을 많이 받는다. 기본 robust normalization은

\[
Z_{robust}(X)=\frac{S(X)-median(S)}{1.4826\,MAD(S)+\epsilon}
\]

를 사용한다.

필요 시 winsorization 또는 Huber clipping을 적용한다.

\[
Z_c=clip(Z,-z_{max},z_{max})
\]

이는 한 모델 하나가 Hamiltonian 전체를 지배하지 못하게 한다.

---

# 32. Model Disagreement

여러 독립 모델이 후보에 대해 서로 크게 갈리면 uncertainty를 높인다.

모델별 normalized score를 \(z_k(X)\)라 하면

\[
Disagreement(X)=Var_k(z_k(X))
\]

MOM으로 높은 상관 모델을 먼저 묶은 뒤 cluster-level disagreement를 계산하는 버전이 더 바람직하다.

이 값은 OSDI 및 Hamiltonian uncertainty penalty에 들어간다.

---

# 33. Uncertainty composite

후보별 uncertainty는 최소 다음을 통합한다.

\[
U(X)=a\,BootstrapVar(X)+b\,ParameterVar(X)+c\,ModelDisagreement(X)+d\,[1-COS(X)]+e\,[1-FRP(X)]
\]

높은 raw score라도 uncertainty가 크면 최종 에너지가 상승한다.

---

# 34. ORACLE Field Index와 실제 확률의 분리

절대 금지 표현:

- “당첨확률 84%”
- “정확도 91%”
- “1등 확률 상승 보장”

허용 지표:

- ORACLE Field Index
- ORACLE Rank Percentile
- Counterfactual Stability
- Rank Persistence
- Model Consensus
- Human Selection Shadow
- Basin Stability

모든 지표는 내부 상대지표라고 표시한다.

---

# 35. Official Pick과 Session Pick

## Official Pick

- 특정 target draw에 대한 canonical prediction.
- DatasetHash, EngineVersion, ConfigHash, Seed로 재현 가능.
- 생성 후 Prediction Ledger에 봉인.
- 추첨 후 변경 금지.

## Session Pick / Recalculate

- 같은 분석장에서 반복 샘플링.
- 매번 달라질 수 있음.
- ORACLE field의 성격은 유지.

둘을 UI와 저장 형식에서 명확히 분리한다.

---

# 36. Prediction Seal

Official Pick에는 다음을 저장한다.

```text
target_draw
engine_version
dataset_hash
config_hash
generated_at
numbers
field_index
oracle_signature
seed_or_rng_commitment
prediction_hash
previous_prediction_hash
```

Hash chain:

\[
H_t=SHA256(H_{t-1}\|Record_t)
\]

로 Prediction Ledger를 구성하면 추첨 후 과거 예측을 수정했는지 검증할 수 있다.

---

# 37. Assembly 구현 원칙

최종 canonical 구현은 Windows x86-64 Assembly를 중심으로 한다.

## Hot path

- fixed-point Q16/Q32 우선
- 64-bit bitmask
- POPCNT
- BMI2 선택적 사용
- AVX2/AVX-512 runtime dispatch
- SoA memory layout
- branchless scoring 가능한 항목은 branchless화
- pair/triple lookup precompute
- worker-local heap

## Heavy analysis path

FSL eigen solver, covariance inverse, GMM, bootstrap 등은 계산 빈도가 낮으므로 double precision SSE2/AVX 경로를 허용한다. CRT 없이 필요한 수학 함수는 직접 구현하거나 polynomial/table approximation을 사용한다.

## Determinism

동일 CPU feature path 여부와 무관하게 Official Pick의 canonical mode는 가능한 한 동일 결과를 내도록 한다. SIMD path와 scalar reference의 허용 오차를 정의하고 최종 rank tie-break는 lexicographic/combinadic deterministic rule을 사용한다.

---

# 38. Reference Mode와 Accelerated Mode

## Reference Mode

- deterministic scalar 또는 제한된 SIMD
- 테스트와 재현성 기준
- 모든 공식의 기준 결과 생성

## Accelerated Mode

- AVX2/AVX-512
- 멀티스레드
- 선택적 GPU

Accelerated Mode는 Reference Mode와 정해진 tolerance 내에서 score/rank가 일치해야 한다.

---

# 39. 검증 프로토콜

모든 신규 독자 공식은 다음 절차를 통과해야 한다.

1. Unit correctness test.
2. Uniform synthetic data test.
3. Pathological synthetic data test.
4. Historical walk-forward.
5. Matched random baseline.
6. Bootstrap stability.
7. Parameter perturbation.
8. Ablation test.
9. Correlation/MOM inspection.
10. Calibration test.
11. Computational performance benchmark.
12. Champion/Challenger evaluation.

‘복잡하고 그럴듯하다’는 이유만으로 Champion에 들어갈 수 없다.

---

# 40. Champion–Challenger

현재 운영 모델을 Champion이라 하고 새 공식/가중치 체계를 Challenger로 둔다.

Challenger는 최소 다음에서 Champion을 비교한다.

- mean match count
- ≥2/≥3/≥4 hit frequency
- rank/calibration metrics
- robustness
- random-baseline percentile
- computational cost
- stability under drift

통계적으로 실질적인 개선이 확인되지 않으면 승격하지 않는다.

---

# 41. 최종 OFA 데이터 흐름

```text
Historical Draw Snapshot
        ↓
Integrity / Canonical Hash
        ↓
Uniform Null Model
        ↓
FNG
        ↓
OTRK + TPA
        ↓
Pair Graph → FSL
        ↓
Triple Hypergraph → OHC
        ↓
High-dimensional Features → OHPS + FEB
        ↓
Interaction Decomposition → OIS
        ↓
8,145,060 Global Scan
        ↓
Deep Survivor Field
        ↓
OBG + FBV + FSS
        ↓
Counterfactual Worlds
        ↓
COS + FRP
        ↓
OMD + MOM + OCSG
        ↓
FECL Evidence Budget
        ↓
Fortuna ORACLE Hamiltonian
        ↓
OSDI
        ↓
ATF
        ↓
P_oracle(X)
        ↓
Uniform Mixture
        ↓
P_Fortuna(X)
        ↓
Official Pick / Recalculate
        ↓
FCA for multi-ticket constellation
        ↓
ORACLE Signature + Prediction Seal
```

---

# 42. 대표 독자 기술 10종

Fortuna 브랜드 문서나 About 화면에서 대표 기술군으로 사용할 명칭은 다음과 같이 고정한다.

1. **Temporal Resonance** — 다중 시간척도 공명 분석.
2. **Spectral Lattice** — 45번호 전역 관계망의 고유모드 분석.
3. **Hyperphase Coupling** — pair로 설명되지 않는 triple 상호작용.
4. **Counterfactual Orbit** — 수천 가상 역사에서 후보 순위 궤도 추적.
5. **Basin Geometry** — 후보 주변 234개 이상의 조합 지형 분석.
6. **Evidence Conservation** — 데이터가 허용하는 총 확신량을 제한.
7. **Model Orthogonality** — 중복 모델의 증거 중복 계산 억제.
8. **Mirage Detection** — 랜덤에서도 생기는 가짜 패턴 제거.
9. **Self-Doubt Index** — ORACLE 자신의 분석 불확실성 수치화.
10. **Constellation Optimization** — 여러 추천을 하나의 다양성 최적화 문제로 계산.

이 10개는 다른 세부 공식들을 대표하는 외부-facing 기술명이다.

---

# 43. 최종 구현 체크리스트

아래 항목이 모두 구현되어야 OFA 전체가 완성된 것으로 본다.

- [ ] FNG complete null-centered normalization
- [ ] OTRK multi-scale temporal family
- [ ] TPA phase agreement
- [ ] FSL spectral lattice
- [ ] OHC triple hyperphase
- [ ] OHPS high-dimensional historical phase space
- [ ] FEB multi-partition entropy
- [ ] OIS interaction synergy
- [ ] OBG 1-swap basin geometry
- [ ] FBV basin volume
- [ ] FSS swap sensitivity
- [ ] COS counterfactual worlds and orbit stability
- [ ] FRP multi-K rank persistence
- [ ] FECL global evidence budget
- [ ] MOM correlation-aware ensemble
- [ ] OMD null simulation + FDR
- [ ] OCSG ablation gate
- [ ] HSSM crowd shadow
- [ ] Fortuna ORACLE Hamiltonian
- [ ] composite uncertainty
- [ ] model disagreement
- [ ] OSDI
- [ ] ATF
- [ ] final uniform mixture
- [ ] deterministic Official Pick mode
- [ ] stochastic Session Recalculate mode
- [ ] FCA multi-ticket constellation
- [ ] ORACLE Signature
- [ ] Prediction Seal / hash-chain ledger
- [ ] Reference Mode
- [ ] Accelerated Mode
- [ ] walk-forward validator
- [ ] Monte Carlo null laboratory
- [ ] bootstrap / block-bootstrap
- [ ] parameter perturbation
- [ ] Champion–Challenger
- [ ] drift monitoring
- [ ] reproducibility tests

---

# 44. 최종 설계 문장

Fortuna ORACLE의 최종 설계 철학은 다음 한 문장으로 고정한다.

> **Fortuna ORACLE은 모든 조합을 평가하고, 모든 공식을 계산하지만, 그 어떤 공식도 무조건 믿지 않는다. 시간·관계·고차원 구조·조합 공간의 기하학·반사실적 역사·모델 간 독립성·통계적 착시·불확실성을 동시에 평가하고, 데이터가 허용하는 만큼만 신뢰한 뒤, 남은 확신마저 균등 무작위와 다시 섞어 하나의 분석적 확률장을 생성한다.**

이 문서에 정의된 OFA의 모든 공식과 계층은 향후 구현 대상이다. 어떤 항목도 ‘아이디어 메모’로 삭제하지 않는다. 구현 과정에서 수식이 개선될 경우 기존 정의를 덮어쓰기보다 버전과 변경 이유를 기록하고, Champion–Challenger 검증을 통과한 뒤 Canonical 수식을 갱신한다.
