# Integrated ten-function inference 1.0

App 1.9.0 / Engine 1.6.0 / public ABI 1.0.

The [depth extension](DEPTH_VALIDATION.md) adds multiscale/graph correction,
compensated partition derivatives and causal/bootstrap gates to this base layer.

Both product ORACLE buttons execute this layer by default after the existing
normal/research analysis and Advanced Evidence Layer. Its chronological folds are drawn from the user-selected causal backtest target window; the last applicable rows in that window are used without reading their future outcomes. The random button remains
an independent uniform generator. Runtime implementation: `src/integrated.inc`
and `src/integrated_api.inc`, entirely x64 assembly. The account/license entry
points are unchanged. No external Python or engine DLL is required by the app.

## What is implemented

These are concrete, bounded versions of the ten proposed functions. They are not
claims of exact continuous Bayesian inference or of a lottery prediction edge.

| # | Function | Implementation |
|---|---|---|
| 1 | High-order joint probability | Existing prefix-fitted singleton/pair/triple coefficients, normalized on every six-subset, become the first model family. |
| 2 | Coefficient uncertainty integration | A finite empirical-Bayes posterior over coefficient scales, with Gaussian-shaped discrete prior, is marginalized in the actual predictive distribution. |
| 3 | Annealed importance sampling | Uniform start, intermediate exponential distributions and invariant independent-Metropolis kernels; the partition estimate is checked against enumeration. |
| 4 | Bayesian changepoint | Exact beta-Bernoulli posterior over a single changepoint in draw-sum parity, including the no-change model. Posterior-averaged suffix rates form another joint-model family. |
| 5 | Full synthetic-history refitting | Fresh uniform histories rerun all four bases, scale posteriors, chronological stacking and robust weight fitting. Held-out performance is compared against real-history performance. This is not merely permuting final scores. |
| 6 | Low-rank tensor | Eight rank-one tensor contractions and least-squares amplitude fit approximate the distinct-triple coefficient tensor. Its capped coefficients form the second family. |
| 7 | Dynamic state model | Per-number marginal Gaussian working-state filters track inclusion rates; those rates parameterize the fourth normalized joint family. |
| 8 | Chronological stacking | Five predictive distributions (four learned families plus uniform) are combined on strictly earlier validation rows. The last row is excluded from fitting. |
| 9 | Distributional robustness | After ordinary stacking, a KL-penalized adversary reweights validation losses and exponentiated-gradient updates refine the mixture on the probability simplex. |
| 10 | Partition-mass bounds | During exhaustive normalization, unvisited combinations are enclosed using the global energy bound. The enclosure tightens as combinations are enumerated. A declared floating-point margin expands the final bounds. |

## Profiles and cost

| Work | Normal | Deep |
|---|---:|---:|
| Families, plus uniform | 4 + 1 | 4 + 1 |
| Scale values | 0, 0.5, 1 | 0, 0.5, 1, 1.5, 2 |
| Most recent chronological validation rows | up to 8 | up to 16 |
| Complete synthetic-history refits | 2 | 8 |
| AIS paths / intermediate levels | 8 / 8 | 64 / 32 |
| Ordinary / robust weight iterations | 40 / 40 | 40 / 40 |
| Exact combination support | 8,145,060 | 8,145,060 |

All functions exist in both profiles. With exactly 60 historical rows there are
no out-of-sample rows: bases, posteriors, synthetic fits, AIS and bounds are still
computed; stacking stays at equal weights and the normal-mode extra contribution
is zero. With one validation row it is reserved for evaluation, not weight fitting.
Small null counts are coarse diagnostics (minimum p = 1/(R+1)), not a significant
discovery test. Normal and deep do not have a one-hour minimum or a sleep loop.
Cost depends on hardware and history; this release does not promise a 60-minute
deadline, adaptive one-hour scheduler, or resumable checkpoints.

`python engine/tests/benchmark-integrated.py` measures the added layer on 60
synthetic rows separately from the pre-existing analysis. Windows CI prints the
time for both complete default API routes. Do not extrapolate these measurements
to a full real history without measuring that input.

## Probability and finite posterior

For family m, E_m(S) is a bounded singleton/pair/triple energy. For each scale a:

    Z_m(a) = sum_{|S|=6} exp(a E_m(S))
    p_m(S | a) = exp(a E_m(S)) / Z_m(a)
    w_m(a | D) proportional to exp(a sum_D E_m(S) - n log Z_m(a) - a^2/2)
    p_m(S | D) = sum_a w_m(a | D) p_m(S | a)

The coefficient bases are estimated from the same training prefix. Therefore the
posterior is conditional on an empirical-Bayes basis; it is not an exact full
posterior over all 16,260 coefficients. Exact enumeration means full finite
support enumeration in floating arithmetic, not symbolic exact arithmetic.
Temporal validation refits bases on each training prefix before looking at its
next draw. No coefficient or mixture fit reads its outer evaluation target.

## Changepoint and dynamic assumptions

The changepoint statistic is `sum(ball numbers) mod 2`, a binary time series.
Each split s in [0,n) has equal prior mass. Segment likelihoods integrate a
Beta(1,1) Bernoulli parameter: B(k+1,n-k+1). Split zero represents no change.
All suffix rates are averaged with these split probabilities before producing
bounded marginal residual coefficients. This detects changes in that statistic,
not arbitrary physical changes in a draw machine.

The dynamic family uses p=6/N, Q=0.0001, R=p(1-p), initial mean p and variance R.
For each observation: P'=P+Q, K=P'/(P'+R), m'=m+K(y-m), P_new=K R.
This is a marginal Gaussian working filter for binary observations, not an exact
joint Bernoulli state posterior. Final probabilities obey the six-ball constraint
because the final joint normalization still enumerates only six-subsets.
Since engine 1.5, Kalman rates are blended with four discounted beta estimates
and graph-regularized before forming the dynamic coefficients; see the extension.

## Stacking and robustness

The first F-1 chronological predictions fit the mixture; the last is untouched
outer confirmation. Starting at equal weights, ordinary stacking maximizes the
mean log predictive probability. A further 40 steps optimize the KL-penalized
worst-case loss with temperature 1:

    max_q sum_t q_t loss_t - KL(q || uniform)
    q_t proportional to exp(loss_t)

Exponentiated-gradient updates keep mixture weights nonnegative and summing to
one. Fixed iteration counts approximate the optimum; no convergence guarantee is
asserted. The algorithm includes the uniform model as its fifth family.

## Sampling and compatibility

Normal: P = (1-alpha) P_legacy + alpha P_integrated. Alpha is at most 0.5 and at
most 1-rho; it is reduced by null support and is zero unless there are at least
two chronological rows and positive outer gain. This cannot reopen an original
uniform-only gate, but it is not independent proof that the new mixture improves
prediction. Deep: P = 0.5 uniform + 0.5 P_integrated (research, unvalidated).
Engine 1.5 additionally requires available bootstrap_low>0 and adjusted sequential
p<=.05 before allowing nonzero normal alpha. These tests only close existing gates.

Sampling first selects the family and scale, then uses exact rejection with a
global energy bound. Singleton/pair/triple caps imply |E| <= 0.8; scales reach
at most 2. Rejection exhaustion or RNG failure returns an error, never a biased
fallback. Games are independent, and duplicates across games are allowed.

The legacy joint state is restored transactionally after integration. Its
forecast ledger continues to record **the legacy joint distribution**, not the
integrated mixture. UI menu labels identify that distinction. Existing records
are not rewritten. `GetIntegratedProbability` exposes the actual new single-game
law for future integrations; a versioned persistent integrated ledger is not
implemented in this change.

## Bounds and diagnostics

For scale 1, partial sum A, remaining count r and |E| <= 0.8:

    A + r exp(-0.8) <= Z <= A + r exp(0.8)

The enclosure is updated during enumeration and finishes at the fully summed Z.
Bounds are widened by 1e-6 relative for numerical safety. Exp and summation are
ordinary floating arithmetic; this is **not** machine-certified interval
arithmetic. The reported final enclosure and AIS relative error refer to the
dynamic family at scale 1. Other scales/families also use full enumeration for
their normalizers. AIS is diagnostic and does not replace the exact-support sums.

The report prints profile, completed-component bitmap (1023 on completed runs),
validation row count, completed null refits, diagnostic null p, held-out log gain,
AIS relative error and final Z enclosure. A zero validation count means no
out-of-sample evidence, not that a model has passed validation.

## API and failure rules

- `FortunaOracleAnalyzeAdvanced()` includes normal integrated inference.
- `FortunaOracleAnalyzeResearchAdvanced(flags)` includes deep integrated inference.
- `FortunaOracleGenerateIntegrated(count, masks, capacity)` generates 1..10 games.
- `FortunaOracleGetIntegratedProbability(mask, double*)` returns actual single-game P.
- `FortunaOracleGetIntegratedSnapshot(void*,64)` returns eight uint32 values
  (size,ready,profile,completed,folds,null_refits,grid,version), then four doubles
  (null_p,heldout_log_gain,mixture_weight,AIS_relative_error).

Standard argument/busy/not-ready/RNG errors follow ABI 1.0. Successful CSV loading
and new analyses invalidate the integrated cache. Random generation preserves it.
Cancellation restores the input history and original joint model, clears readiness,
and produces no integrated success result. Existing lower-level Analyze/Generate
exports remain available as legacy SDK routes; the application uses integrated
routes by default. Authentication is not bypassed by tests or new APIs in the app.

## Validation

`tests/integrated.py` compares actual assembly against NumPy/Python finite-space
references for normalization, discrete posterior weights, changepoints, dynamic
state equations, tensor fit, stacking/robust optimization and sampled frequencies.
It tests future-prefix isolation, original-state restoration, full synthetic
refits and cancellation. `tests/integrated-windows.py` executes both production
default routes on all 45 balls, verifies the bitmap, generation/probability/report,
uniform-cache preservation and failure invalidation through the Windows DLL ABI.

Method background: [AIS](https://arxiv.org/abs/physics/9803008),
[Bayesian changepoints](https://arxiv.org/abs/0710.3742),
[predictive stacking](https://arxiv.org/abs/1704.02030).
The bounded implementation above defines the exact behavior of this release;
it does not claim to implement every variant described by these papers.
