# Depth validation 1.0 — app 1.10.0 / engine 1.7.0

This layer extends the existing ten-function pipeline in **both** product ORACLE
modes. `src/depth.inc` and `src/depth_api.inc` are x64 assembly. Python is used
only for independent testing. The engine stays in this repository's `engine/`
folder and is statically linked into the application. Account checks and the
classic native Windows interface remain in place.

## Added calculations

1. Four discounted beta working filters, at half-lives 16, 64, 256 and infinity.
2. Causal composite-likelihood weighting of those four time scales.
3. Coupled graph-Laplacian regularization of number inclusion estimates, with a
   measured linear-system residual and failure on nonconvergence.
4. Compensated summation for the partition function and its first two derivatives.
5. Full-support energy means and Fisher curvature at every family/scale point.
6. Runtime checks for nonfinite probabilities, posterior mass, partition bounds
   and invalid curvature.
7. Causal online expert mixtures, scored before each weight update.
8. Per-family and online likelihood-ratio processes with correction across time
   and five candidate processes.
9. Stationary circular block bootstrap intervals for causal mean log gain.
10. Outer-row model-ablation sensitivity and loss CUSUM diagnostics.

The additional dynamic calculations change the fourth family used by actual
generation. Validation can only reduce the normal-mode mixture contribution;
it cannot reopen an original uniform-only gate. Deep mode remains an explicitly
unvalidated research mixture. Mathematical complexity is not evidence of a
lottery prediction advantage.

## Multiscale marginal estimation

Let N=45, p=6/N, y[t,i] indicate inclusion of ball i, and kappa=60. Each horizon h
has d[h]=exp(-log(2)/h), with d[infinity]=1. Start excess counts c=0, mass m=0 and
score ell=0. Before incorporating row t, form

    q[h,i] = (kappa*p + c[h,i]) / (kappa + m[h])
    ell[h] += (1/N) sum_i { y[t,i] log q[h,i]
                          + (1-y[t,i]) log(1-q[h,i]) }
    c[h,i] = d[h]*c[h,i] + y[t,i]
    m[h]   = d[h]*m[h] + 1

At the end of the training prefix, weights are softmax(ell), starting from equal
prior weights. Their weighted final q is averaged 50:50 with the existing Kalman
marginal estimate. The score is a **tempered composite marginal likelihood**:
ball indicators within a draw are dependent, so these weights are not an exact
joint Bayesian posterior. All information used here belongs to the training
prefix. Held-out rows are not used to build their own predictions.

## Coupled graph correction

Use the absolute existing prefix-fitted pair coefficient as symmetric edge
weight w[i,j], with a zero diagonal. Define L=diag(W*1)-W and
b=multiscale/Kalman average-p. Solve

    (I+L)x=b

This minimizes 0.5||x-b||² + 0.5*x' L x. The first term retains the estimates;
the second discourages large differences across fitted relationships. Absolute
pair residuals define smoothing strength, not a claimed physical interaction.
The system is positive definite and strictly diagonally dominant. Jacobi uses

    x_next[i] = (b[i]+sum_j w[i,j] x[j]) / (1+sum_j w[i,j])

for 32 normal / 96 deep iterations. The maximum residual must be <=1e-10, and
nonfinite residuals fail the analysis. Pair caps bound the contraction factor.
The corrected rate is p+x. Dynamic singleton coefficients become clip(x/2,
-.05,.05). Pair and triple coefficients retain their existing bounds. Thus the
existing |E|<=.8 rejection bound remains applicable; only six-subsets are allowed.

## Partition derivatives and numerical audits

The full combination traversal accumulates

    Z(a)  = sum_S exp(a*E(S))
    Z1(a) = sum_S E(S)*exp(a*E(S))
    Z2(a) = sum_S E(S)^2*exp(a*E(S))
    mean_E = Z1/Z
    Fisher_E = Z2/Z - (Z1/Z)^2

All three use Kahan compensated summation. Fisher_E is the second derivative of
log Z with respect to the scalar coefficient scale, **not** a full 16,260 by
16,260 information matrix. Tiny negative variances within 1e-10 are rounded to
zero; larger negative or nonfinite values fail. Posteriors must be nonnegative,
finite and sum to one within 1e-10. Each logZ must be within
log(number_of_combinations) +/- a*.8 plus tolerance.

These checks detect corruption and numerical inconsistency. They are not a
machine-certified proof of error bounds for elementary functions or all sums.

## Causal evaluation and multiple testing

For each of up to 8 normal / 16 deep chronological folds, all bases and partition
functions are refitted using only earlier rows. Let P[t,m] be the probability
assigned to the next observed draw by family m. The fifth expert is uniform.

Online weights v start at 1/5. The current predictive law is

    Q[t] = .5*uniform + .5*sum_m v[t,m]*P[t,m]

Only AFTER scoring the draw are the weights updated:

    v[t+1,m] = .98*v[t,m]*P[t,m]/sum_j v[t,j]*P[t,j] + .02/5

The .02 fixed share prevents expert elimination. Four family likelihood-ratio
processes and the online process accumulate log(Nchoose6*probability/1.000001).
The margin is a declared numerical allowance. In exact arithmetic, predictable
normalized alternatives against independent uniform draws give nonnegative
likelihood-ratio martingales. A union bound across the five processes and Ville's
inequality motivate the reported value

    adjusted_p = min(1, 5*exp(-max_{process,time including zero} log_e))

Theoretical control requires the stated independent-uniform null, prefix-safe
probabilities and adequate numerical accuracy. This is an in-memory retrospective
window calculation, not tamper-proof prospective evidence or a permanent testing
budget across repeated user experiments. Changing the window/model after seeing
results or repeatedly choosing datasets is outside that guarantee.

The separate existing stacking fit still excludes the final outer fold. The new
e-processes do not certify the final stacked mixture; they are additional
conservative gates and diagnostics. No p-value is a probability of winning.

## Bootstrap and stress checks

At least eight causal gains log(Q[t]/uniform/1.000001) are required. Resampling
starts at a uniform circular index; each subsequent position restarts uniformly
with probability 1/L, otherwise advances one position (wrapping around).
L=2 normal / 4 deep. This yields geometrically distributed block lengths.
Use 512 normal / 4096 deep replicates of the same window length. Sort replicate
means and report empirical 2.5% and 97.5% endpoints (integer ranks floor(R/40)
and R-1-floor(R/40)). The approximation assumes an appropriate stationary weakly
dependent gain process. Such short windows have limited inferential power.

This bootstrap resamples already causal scores; it does **not** refit the engine.
The existing separate uniform synthetic-history test continues to refit all
bases and mixture weights. With fewer than eight folds, bootstrap count is zero
and its zero endpoints mean unavailable, not a zero-width confidence interval.

For each nonuniform family, ablation removes it from the original stacked weights,
renormalizes the remainder, and measures the absolute log-probability change on
the last untouched outer draw. The report gives the maximum change. Loss CUSUM
uses c[t]=max(0,c[t-1]-gain[t]) and reports max c. These are sensitivity/drift
diagnostics with no calibrated rejection threshold.

## Configurable historical analysis range

Engine 1.7 changes the range control from a validation-only target window to the
historical dataset seen by the entire pipeline. The complete imported source is
preserved separately; selecting an inclusive range copies only that slice into
the active history. Final fitting, ordinary validation, integrated inference,
deep nested validation, reports and the next-round target therefore cannot read
rows after the selected end round.

A selected range must contain at least 60 contiguous draws and may start at the
first loaded round. Within the active range, the first 60 rows are training-only.
Later rows are evaluated causally from strictly earlier rows. If the active range
contains at least 120 rows, its final 30 evaluation targets are reserved for
independent confirmation. SetBacktestRange(0,0) restores the full loaded source.

For example, with source history 1..1242, selecting 1..432 makes the engine
snapshot report history_count=432 and last_round=432; the forecast target is
therefore 433. Rounds 433..1242 remain only in the preserved source buffer and do
not participate in that analysis.

Changing the range invalidates completed normal/deep analysis state. The GUI time
estimate is a hardware-calibrated scheduling hint, not a statistical quantity and
not a completion guarantee.

## Normal-mode gate and cost

In addition to every existing gate, normal alpha becomes zero unless there are
bootstrap results, adjusted_p<=.05 and bootstrap_low>0. Deep retains its 50:50
uniform/integrated research distribution. Uniform random generation stays separate.

Both modes fit all new models. Normal/deep chronological windows expand from
2/8 to 8/16; full null-history counts remain 2/8. New moments add arithmetic to
every full-support enumeration. Time depends strongly on the amount of history,
mode and CPU. There is no sleep loop, one-hour minimum, deadline or checkpointing.
Exactly 60 history rows still provide no held-out evidence; descriptive model
calculations run, while bootstrap is unavailable and adjusted_p=1.

## Diagnostics API and tests

`FortunaOracleGetDepthSnapshot(out,bytes)` requires bytes>=96 and a completed
integrated analysis. It follows standard -1 argument, -2 busy, -4 not-ready errors,
with no output writes on these errors. The first eight uint32 values are size=96,
version=0x10000, ready, profile, folds, bootstrap count, graph iterations, reserved.
The next eight doubles are graph residual, adjusted p, online log-e, bootstrap
low/high, loss CUSUM, maximum ablation change, reserved. Reserved fields are zero.

The normal report includes these results and 4*grid energy moments. Existing
integrated snapshot ABI remains 64 bytes. CSV replacement/cancellation invalidates
results; uniform generation preserves the cache. Existing forecast ledger records
the legacy joint distribution, not this extended integrated mixture.

`tests/depth.py` independently compares the filters and graph solve with NumPy,
derivatives with exact finite support and finite differences, tests tiny-term
summation, corrupted posteriors, future isolation, all 2^10 binary null paths,
stationary-bootstrap analytic variance, and both complete default pipelines.
Windows tests run both full 45-ball routes and verify snapshot bounds/state,
generation, reports and cancellation. Test code is not a runtime dependency.

Background: [Ramdas and Wang, Hypothesis Testing with E-values](https://www.math.uwaterloo.ca/~wang/files/e-book-final.pdf),
[Politis and Romano, The Stationary Bootstrap (1994)](https://doi.org/10.1080/01621459.1994.10476870).
The equations above define this implementation and its limitations.
