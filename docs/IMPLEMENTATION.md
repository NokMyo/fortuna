# ORACLE reference engine — 1.0.0 implementation contract

This file describes the executable shipped as **Febius Fortuna 1.0.0**. `ORACLE.md` and `ORACLE_FIELD_ARCHITECTURE.md` remain the broader design specifications, including research targets. Their historical CURRENT/REQUIRED tags are not a claim about this binary. Product version 1.0.0 does **not** claim completion of the master specification's “Production Canonical” milestone.

## Independent engine distribution (application 1.1.0)

The unchanged reference mathematics now lives in `engine/src/` and builds into **FortunaOracle.dll 1.0.0**. The GUI executable contains no statistical models, candidate scan, SHA-256 engine or validation pipeline; `src/engine_bridge.inc` calls the public ABI and copies snapshots. DLL dependencies are only Kernel32/Advapi32. The app owns windows, dialogs, clipboard, report presentation and the choice of ledger path. See [ENGINE_API.md](ENGINE_API.md) for the versioned interface, process-local state, ownership and concurrency contract.

## Runtime and module boundaries

All application, parsing, hashing, mathematical, sampling, persistence and UI logic is x86-64 assembly. The app uses `scripts/modules.txt`; the DLL uses `engine/modules.txt`. `.rc`/manifest files contain declarative Windows resources. PowerShell, shell and Python are build/test tools only. There is no application C/C++, CRT, Python runtime, embedded browser or network client.

| Module | Responsibility |
|---|---|
| core / hash | Microsoft x64 ABI, bounded buffers, combinadics, scalar SSE2/x87 math, OS entropy and deterministic test stream, SHA-256 |
| data / stats | Transactional CSV, canonical records, prefix statistics, covariance, gap risk sets |
| linalg / models | Pivoted inverse, Jacobi eigensystem, twelve descriptive signal components |
| field / robust | Null normalization, exhaustive fast scan, survivor refinement, local geometry, alternate histories |
| sample / validation | Thermal distribution, mixture and bundle sampling, chronological validation and confirmation |
| report / ledger | Bounded UTF-8 audit report and append-only local hash chain |
| windows / ui / strings / selftest | Unicode Win32 platform, background worker, resources, diagnostic entry points |

Windows 10 1607+ x64 is the minimum because the UI uses `GetDpiForWindow`. The process is DPI aware, asInvoker, and uses ASLR, high-entropy VA, NX and x64 unwind metadata. The engine imports only Windows system DLLs; the application additionally imports FortunaOracle.dll. Native builds use clang integrated assembly, SDK `rc`, MSVC `link /nodefaultlib`. The numerical path is a deterministic reference implementation on the same build/platform; cross-toolchain bit-identical floating-point results are not promised.

## Data, identity and randomness

The [CSV contract](../data/FORMAT.md) specifies limits and canonical hashing. Input is staged; failure leaves the previous dataset intact. Contiguous round order and Gregorian dates are validated. Source authenticity, prize fields, official calendar verification, automatic HTTP refresh, provider failover, provenance signatures and a separate dataset-manifest file are not implemented. Reports carry the canonical hash, record count, last/target round and configuration hash.

Session draws call Windows `SystemFunction036`. Unsigned rejection sampling removes modulo bias, and six distinct numbers are sampled into a 45-bit mask. Entropy failure returns an error, never a fixed fallback combination. SplitMix64 is reserved for deterministic null experiments, alternate histories and sealed selections. It is not represented as a cryptographic random stream.

SHA-256 is implemented in assembly. The field identity includes the 512 masks and binary64 probabilities, dataset/config hashes, mixture coefficient, temperature and model weights, with a fixed zero-padded layout. Configuration identity includes the reference version and fixed analysis parameters. For reproducibility archive the executable, CSV, report and checksum together.

## Signal and field formulas

Let `p=6/45`, `p2=(6*5)/(45*44)`, `p3=(6*5*4)/(45*44*43)`. Counts use only rows preceding the current origin. Models are descriptive scores, not estimated lottery winning probabilities.

| Design name | Shipped calculation and scope |
|---|---|
| FNG | `(posterior-p)/sqrt(p(1-p)/(N+60))`, posterior `(count+60p)/(N+60)`. This is a null-variance standardized shrinkage residual, not an exact posterior-variance calculation. |
| OTRK | Seven mass-normalized EWMA scales 4,8,16,32,64,128,256; sum of adjacent-scale first differences plus half of second differences. Fixed coefficients; no coefficient optimization. |
| TA/TPA | Absolute mean of six above/below-null signs from the temporal scales. Describes cross-scale agreement. |
| Gap/hazard | Number-specific discrete event/risk counts, initial interval left-censored, ongoing intervals contribute risk, gap bins capped at 63; `(events+60p)/(risk+60)-p`. No complete survival-curve report or explicit gap-surprise model. |
| Pair graph | Symmetric 45×45 null-centered shrunk count residual plus one decayed graph. No separate pair PMI model or per-pair exact hypothesis test. |
| FSL | Jacobi eigendecomposition of the symmetric relation matrix (24 sweeps); spectral response `lambda/(1+0.1*abs(lambda))`, reconstructed into pair lookups. Full eigenbasis, not top-component selection. |
| OHC | All 14,190 unordered triples; smoothed log interaction from triple/single/pair frequencies, finite-without-replacement null centering, and count shrinkage `c/(c+8)`. **No triple-level BH correction**; the aggregate model is subjected to held-out model-level BH. No 4-way interactions. |
| OHPS | Eight coordinates: sum, range, odd count, low-number count (≤22), adjacency, primes, ending collisions, five-bin entropy. Welford covariance, ridge, pivoted inverse; negative distance from Mahalanobis radius `sqrt(7)`. No GMM/KDE. |
| FEB | Entropy typicality relative to historical means/variances in 2,3,5,9 equal partitions and 10 ending-digit buckets. |
| OIS | Fixed combination of pair, spectral and triple interaction components. Correlation correction later reduces duplicate evidence. |
| HSSM | Heuristic penalties for birthday-range numbers, round/multiple patterns, repeated endings and equal gaps. No ticket-sales calibration or expected prize-sharing claim. |
| Robust normalization | 512 deterministic uniform reference combinations; median/MAD with 1.4826 scale factor, epsilon floor, clipping to ±4. Final energy has its own median/MAD. |
| Stage A/B | Gosper enumeration of exactly 8,145,060 masks; six node and fifteen pair fixed-point lookups; deterministic bounded heap of 4,096, then twelve-model refinement to 512. Tie order is deterministic. Global coverage applies to **fast** scoring, not full deep scoring of all combinations. |
| OBG | All `6*39=234` one-ball replacements for each survivor; mean minus standard deviation minus half positive spike curvature. No two-swap neighborhood. |
| FBV | Stable log-sum-exp of one-swap descriptive neighbor energies. Fixed local temperature. |
| FSS | Six median leave-one-ball replacement effects over 39 alternatives; report summarizes them. No separate strong/medium/weak per-ball UI badges. |
| COS | 32 worlds, eight each from IID resampling, circular blocks (length 8), random 10% deletion, trailing 5/10/15/20% deletion. Refit base models, rank the same 512 survivors. `exp(-MAD(rank)/100)`. World seed hashes the **prefix only**, so future records cannot affect resampling. |
| FRP | Fractions inside conditional top 10/top 100 over 32 worlds and median conditional rank. These are not exhaustive global ranks in each world. No hyperparameter-jitter/adversarial family. |
| OMD / FDR | 512 matched null experiments; plus-one Monte Carlo p-values, resolution 1/513. Eleven predictive descriptive components receive model-level Benjamini–Hochberg q-values. HSSM is excluded. |
| OCSG | Positive held-out standardized-score utility, positive additive leave-one-model-out contrast, q<0.05. This is an **additive surrogate**, not rerunning the full pipeline with every component removed. |
| MOM | Divide eligible component utility by `1+sum(abs(correlation))` against the null-normalized model vectors. No inverse-correlation portfolio or PCA. |
| FECL | Normalize eligible relative weights to sum 1, multiply by evidence `E=clip(mean_hits-0.8,0,1)*(1-p_null)`. Component weight sum is bounded by E. |
| OSDI / uncertainty | Conservative reduced policy `rho=1-E/2`, so at least half is uniform even after promotion; E=0 gives rho=1. The architecture's full multi-term OSDI/disagreement/uncertainty composite is not implemented. |
| Hamiltonian / ATF | Evidence-weighted model score plus 0.1 HSSM, 0.05 E times geometry, 0.5 E times COS+FRP100. Equal-model mean is used only for the fixed diagnostic challenger when weights sum to zero. Stable softmax over robust-clipped energy; 24 binary-search steps, temperature [0.02,64], entropy target 0.9 log(512). |
| Probability field | `rho*Uniform(all combinations)+(1-rho)*Oracle(512 survivors)`. Uniform component preserves global support. Oracle branch probabilities sum to one. |
| FCA | Greedy `FCA_1`: for each additional ticket sample 64 policy candidates and minimize the sum of squared overlaps with earlier tickets. Reject exact duplicates. This changes the bundle's joint law. The determinant/DPP alternative is not implemented. |
| ORACLE Signature / Index | Dataset, config and field SHA-256; index is empirical relative descriptive score against the fixed 512 uniform references, scaled to 0–1000. No fabricated score-to-jackpot conversion. |

Some design alternatives are intentionally represented by simpler reference calculations. The master and field formulas should not be cited as proof that every optional term is active in this executable.

## Chronological validation and promotion

For N<60, analysis is unavailable and uniform generation remains available. For 60≤N<120, every origin from 60 to N−1 trains only on `[0,origin)` and evaluates the next draw, but no adaptive policy can be promoted. N=60 has no held-out outcomes.

For N≥120, the final 30 rows are reserved before learning any weights. The first phase evaluates the fixed descriptive challenger at every origin from 60 through N−31. Every fold rebuilds feature statistics, normalization, eigen/covariance/triple models, the exhaustive scan, all geometry and all alternate worlds from the prefix. Truth is supplied only to evaluation and exact fast-rank accounting; it does not influence candidate selection. Prefix-hash seeding is covered by a future-data mutation test.

The first phase records hit distribution, mean hits, null percentile interval of mean hits, exact actual fast-score rank, marginal Brier loss and 10-bin inclusion calibration. Monte Carlo comparisons and model tests aggregate held-out outcomes. Gates require ≥30 learning folds, matched-null p<0.05, lower Brier than uniform, and eligible positive model utilities/q-values. Correlation-corrected weights and E are then **frozen**.

An eligible policy is replayed on the reserved last 30 rows. Features refit at each origin, but weights and the evidence budget do not learn from confirmation outcomes. Its actual uniform-mixture marginal Brier and sampled hits are evaluated against 512 uniform baselines. Both p<0.05 and Brier improvement are required. Otherwise weights are cleared and rho becomes 1. Confirmation state 0 means no eligible promotion, 1 pass, 2 reject. Confirmation rows and summary are exported. A passing retrospective test does not establish a persistent predictive advantage; repeated user-selected datasets are not adjusted as additional hypothesis tests.

Final analysis restores all N rows and constructs the session field using the approved policy. Recalculate samples that field, without a fresh model fit. Analysis cancellation clears field readiness and returns to uniform mode.

## Reports and local prediction ledger

The report is bounded to 2 MiB and includes model p/q/gates, relative weights and normalizers, 45 inclusion marginals/persistence values, all 512 candidates with geometry/world summaries, learning and confirmation folds, and explicit method limitations. It does not export every eigenvector, per-world per-model vector, histogram or sensitivity plot. UI “analysis detail” opens the report; there is no separate interactive expert dashboard.

Ledger records are 256-byte FLG1 blocks with previous hash and SHA-256 digest, dataset/config/field identities, target, timestamp, seed, mask, index and policy coefficient. Each existing record is verified before append. Same-target requests reuse the existing record. A new target must increase and precede the local clock deadline corresponding to Saturday 20:45 KST. Writes are flushed; concurrent writers are excluded. A truncated/corrupt chain is refused. This is not atomic against power loss, a digital signature, an authenticated data provider, an external time authority or an external publication anchor. Deleting/rewriting the entire local chain cannot be detected without a separately trusted copy.

## Verification and remaining work

Release CI exercises the real assembly on Linux against independent Python/NumPy expectations and builds/runs the native Windows executable. Checks cover SHA padding boundaries, binomials, elementary math, strict transactional/canonical parsing, count and prime features, matrix inversion, BH values, exhaustive enumeration, valid distinct survivors, probability sums, alternate-world prefix isolation, full rolling validation, independent-confirmation rejection and positive promotion on a deliberately non-lottery sequence, replay hashes, valid bundles, report bounds, cancellation, startup/CLI errors, Unicode file paths, absence of CRT imports, GUI generation and capture, plus temporary-ledger append/reuse/truncation refusal.

The native adapter only changes the calling convention and OS entropy shim. Tests are engineering checks, not evidence of lottery prediction performance. No real lottery dataset is bundled, no actual-history performance claim is made, and code signing is not included.

Outstanding master-specification items include automatic official data/provider/manifests, richer structural histograms and regime/change-point models, exact per-signal null tests and triple-wise FDR, full pipeline ablation, parameter perturbation, Bayesian model averaging, drift/adaptive governance, model-disagreement composite, external prediction verification, cached incremental analysis, multi-worker scan, packed pair optimization, AVX2/AVX-512 dispatch, GPU/NUMA, determinant bundles and research/visualization interfaces. GMM/HMM/KDE and other research/extension entries remain future work. These are not silently marked complete by releasing the native reference product.
