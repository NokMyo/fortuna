# Advanced Evidence Layer 1.3

AEL is an additive validation and search layer used by both the normal and deep Fortuna ORACLE paths. It does not claim a lottery prediction advantage and does not replace the existing prefix-only validation gates. Its purpose is to make unsupported structure harder to promote and to spend more computation on candidate stability.

## 1. Fourth-order interaction

For a sorted zero-based quadruple `a<b<c<d`, AEL uses the combinadic index

`I4(a,b,c,d) = C(d,4) + C(c,3) + C(b,2) + a`.

There are `C(45,4)=148,995` possible four-number subsets. Every six-number draw contributes exactly `C(6,4)=15` observations.

The fair-draw reference marginal is

`p4 = C(41,2) / C(45,6) ≈ 0.00010067451927917044`.

AEL applies a symmetric prior of 480 effective draws. For a feature count `c` in `n` training draws, the posterior-style rate estimate is compared with `p4`, standardized by the finite Bernoulli working variance, and shrunk by `c/(c+16)`. A six-number candidate averages its 15 fourth-order residuals. The resulting score is then hierarchically blended with its 20 parent triple scores: 75% fourth-order residual, 25% triple mean.

This is a conservative empirical-Bayes working score, not an exact posterior probability of a future lottery combination.

## 2. Prefix-only fourth-order walk-forward

The fourth-order model has its own sequential validation. Training begins with the first 60 historical draws. At cutoff `t`, counts contain only `[0,t)`. AEL scores the held-out draw `t` and compares it against matched uniformly generated six-number masks using the same prefix counts. Only after scoring does draw `t` enter the counts.

Normal mode uses 32 matched null masks per fold. Deep mode uses 96. The fold percentile is averaged and mapped to support `max(0,min(1,2*(mean_percentile-0.5)))`. Unsupported fourth-order structure therefore receives exactly zero weight.

The main ORACLE already performs rolling prefix validation at every cutoff and the deep ORACLE additionally performs nested chronological validation; AEL does not weaken either layer.

## 3. Structure-preserving synthetic-null stress

AEL uses the most recent up to 256 prefix-validation rows. It keeps each historical forecast mask fixed and destroys the association with the realized number identities using affine permutations of the 45 labels. Multipliers are selected only from integers coprime to 45, so each transformation is bijective and preserves six-number cardinality.

The statistic is total forecast/truth overlap. Normal mode evaluates 4,096 synthetic histories; deep mode evaluates 8,192. With exceedance count `e` and `R` replications,

`p_null = (e+1)/(R+1)`.

A conservative support is

`S_null = max(0, 1 - p_null/0.20)`.

This is an adversarial stress test against identity-driven pattern discovery, not a formal proof that any surviving signal is predictive.

## 4. Temporal stability

The same recent validation rows are divided chronologically into four blocks. Each block receives half credit when its mean Brier score beats the uniform six-of-45 baseline and half credit when its mean overlap is at least the fair expectation `6*6/45 = 0.8`. The four block scores are averaged into `S_time` in `[0,1]`.

This makes a signal that appears only in one short period less influential than a signal that behaves consistently across multiple chronological regions.

## 5. Hierarchical uncertainty shrinkage

AEL never increases a base model weight merely because the new layer exists. For each already-gated model, it builds a global support term from validation sample size, synthetic-null support and temporal stability. That support is multiplied by a model-specific penalty derived from its BH q-value.

The resulting multiplier is constrained to `[0,1]`, so AEL can retain or reduce a learned weight but cannot resurrect a model closed by the original gates. The discarded fraction is stored as model uncertainty. Aggregate evidence is reduced by the same global support, and the uniform-mixture parameter `rho` moves toward 1 as uncertainty increases.

## 6. Deeper candidate search

The original 4,096 fast survivors remain the first screening boundary. AEL reruns the exact 45C6 scan with the existing `MAX_FAST=8,192` capacity, rescoring those survivors with validated model weights plus the bounded fourth-order term. The best 2,048 form an advanced stage and the best 512 then form the thermal field.

This adds a real intermediate search layer rather than simply increasing a loop count.

## 7. Complete 2-swap neighborhood

For selected elite candidates, AEL replaces two of the six selected numbers with two of the 39 outside numbers. The complete neighborhood size is

`C(6,2) * C(39,2) = 15 * 741 = 11,115`.

Normal ORACLE performs this exhaustive search for the strongest 8 field seeds. Deep ORACLE uses 32. A replacement must improve the advanced energy and must not duplicate another current field member.

## 8. Leave-two contribution stability

After normalization, AEL computes the absolute contribution of each validated model to a candidate. Let `T` be the sum of absolute contributions and `m1,m2` the two largest. Candidate stability is

`S_candidate = 1 - (m1+m2)/T`,

bounded to `[0,1]`. A high value means the score is spread across several models instead of being dominated by two components. The thermal score receives a small centered correction: ±0.05 in normal mode and ±0.10 in deep mode.

## 9. Deep joint sampling

Deep ORACLE retains the exact finite-space joint model as its proposal distribution. When AEL itself has nonzero validated support, up to 32 bounded rejection attempts incorporate the advanced candidate energy. The acceptance probability remains strictly positive, so the procedure does not delete regions of the original joint support. If AEL has no support, sampling falls back directly to the original joint sampler.

## 10. Test boundary

`engine/tests/advanced.py` compiles the actual assembly layer and independently checks:

- all 148,995 combinadic fourth-order indices,
- 15 four-subsets from a six-number mask,
- bijective 45-label permutations preserving exactly six bits,
- prefix-only fourth-order walk-forward accounting,
- four-block temporal stability,
- bounded synthetic-null p-values and support.

The existing engine and research regression suites run unchanged afterward. Windows CI separately builds the static single-EXE path and optional DLL/SDK.

## Interpretation

AEL increases analytical resolution, validation strictness and computation depth. It does not change the mathematical fact that in a fair Lotto 6/45 draw each exact six-number set has probability `1/C(45,6) = 1/8,145,060`.
