# Solver Backlog

A survey of solvers available to implement. Sources:

1. **The Qt app's own history** (`b5c06dd`) — ~60 solvers written for the
   desktop app and never ported. The formulas exist; only the registry
   entry is missing. This is the cheapest supply available.
2. **Formula-sheet inventories** — the 174-formula and 252-formula physics
   sheets, Wikipedia's *List of equations*, and the table of thermodynamic
   equations. These are systematic rather than anecdotal.
3. **Subject-by-subject tool inventories** — 81 chemistry tools, the
   62-calculator chemistry collection, and comparable suites for
   engineering, statistics, finance, medicine and computing.
4. **The standard undergraduate curriculum** — what a degree actually
   covers.

Current inventory: **51 solvers** across 11 categories.

Effort: **S** ≈ a day · **M** ≈ a few days · **L** ≈ a week+.
Value is how many courses and exercises the solver touches.

---

## Part 1 — Port from the Qt history

Every one of these was already implemented in the desktop app. Porting is
mechanical: read the C++, write the descriptor.

### Mathematics & number theory
| Solver | Effort | Notes |
|---|---|---|
| Linear system solver (Gaussian elimination) | S | 2×2 and 3×3, Cramer's rule as an alternative |
| Matrix operations (add, multiply, determinant, inverse, transpose) | M | Needs a `matrix` input kind |
| Eigenvalues & eigenvectors | L | Characteristic polynomial; numeric for 3×3+ |
| Complex numbers (arithmetic, polar form, De Moivre) | M | |
| Polynomial operations (add, subtract, multiply, divide) | M | Synthetic division included |
| Sequences & series (arithmetic, geometric, Fibonacci, Σ) | S | |
| Set theory (union, intersection, power set, cardinality) | S | |
| GCD / LCM | S | |
| Modular arithmetic (mod, modpow, modular inverse, CRT) | S | |
| Primality tests, Euler's totient, factorisation methods | S | |
| Combinatorics (nCr, nPr, Catalan, derangements, Stirling) | S | |

### Calculus & analysis
| Solver | Effort | Notes |
|---|---|---|
| Numeric differentiation | S | Already in the grapher — expose as a tool |
| Numeric integration (definite) | S | Adaptive Simpson already in `lib/graph.ts` |
| Root finder (Newton–Raphson, bisection, secant) | S | |
| Limits (L'Hôpital, ∞−∞, ∞/∞) | M | |
| Taylor / Maclaurin series | M | |
| Symbolic differentiation | L | Needs a symbolic engine |
| Symbolic integration | L | Needs a symbolic engine |

### Statistics
| Solver | Effort | Notes |
|---|---|---|
| Probability distributions (normal, t, χ², F, Poisson) | M | PDF, CDF, quantiles |
| Hypothesis tests (z, t, χ², F) | M | |
| Confidence intervals | S | |
| ANOVA | M | |
| Chi-square test | S | |
| Frequency distribution & histogram data | S | |
| Covariance, Pearson & Spearman correlation | S | |
| Sample size & statistical power | M | |
| Bayesian probability & Bayes' theorem | S | |

### Physics
| Solver | Effort | Notes |
|---|---|---|
| Circular motion (centripetal force, period, banked curve) | S | |
| Rotational dynamics (torque, angular velocity, L = Iω, τ = Iα) | M | |
| Moment of inertia (point, disk, sphere, rod, parallel axis) | S | |
| Collisions (1D elastic, perfectly inelastic, restitution) | S | |
| Inclined plane (parallel/normal force, friction) | S | |
| Drag & terminal velocity | S | |
| Gravitation (Newton, escape velocity, orbital velocity, Kepler) | S | |
| SHM (spring, pendulum, physical pendulum, damped, Q factor) | M | |
| Electrostatics (Coulomb, E field, potential, Gauss's law, dipole) | M | |
| Capacitance (parallel plate, energy, series/parallel, RC, dielectric) | S | |
| Magnetism (force on wire, long wire, solenoid, cyclotron, Hall) | M | |
| Electromagnetic induction (Faraday, flux, motional EMF, transformer) | M | |
| Thermodynamics (Carnot, entropy, Gibbs free energy, first law) | M | |
| Kinetic theory (v_rms, mean free path, most probable speed) | S | |
| Relativity (Lorentz factor, time dilation, length contraction, E=mc²) | S | |
| Quantum (photon energy, de Broglie, photoelectric, Bohr, Rydberg) | S | |
| Heisenberg uncertainty | S | |
| Optics (Snell, thin lens, mirrors, critical angle, double slit, grating) | M | |
| Nuclear (decay, half-life, activity, binding energy) | S | |
| Astrophysics (Schwarzschild radius, Hubble's law, luminosity) | S | |
| Fluid (Bernoulli, continuity, buoyancy, Reynolds, Stokes drag) | M | |
| Doppler effect & sound level (dB) | S | |
| Standing waves (string, open pipe, closed pipe) | S | |

### Chemistry
| Solver | Effort | Notes |
|---|---|---|
| Stoichiometry (limiting reagent, theoretical & % yield) | M | High exam value |
| Equation balancer | M | |
| Empirical → molecular formula, combustion analysis | M | |
| Concentration (molarity, molality, normality, mass %, ppm) | S | |
| Colligative properties (ΔTf, ΔTb, osmotic pressure) | M | |
| Acid–base (pKa, buffers, Henderson–Hasselbalch, salt pH) | M | |
| Titration (unknown concentration, equivalence point) | M | |
| Equilibrium (Kc, Kp, Ksp, ICE tables, Le Chatelier) | M | |
| Thermochemistry (heat capacity, Hess's law, ΔHf, bond enthalpies) | M | |
| Kinetics (rate laws, half-life, Arrhenius activation energy) | M | |
| Electrochemistry (cell potential, Nernst, Faraday, electrolysis) | M | |
| Beer–Lambert law & calibration curve | S | |
| Degree of unsaturation, isomer counting | S | |
| Graham's law of effusion | S | |
| Nuclear half-life (already partly covered) | S | |

### Electrical
| Solver | Effort | Notes |
|---|---|---|
| Voltage divider | S | |
| RC / RL time constants, capacitor, inductor | S | |
| Op-amp (inverting, non-inverting) | S | |
| dB converter & power factor | S | |
| Phasors | M | |
| Resistor colour code decoder | S | Trivial, very popular |
| AWG wire gauge & ampacity | S | |
| Antenna length, dBm, path loss | S | |
| Three-phase power, transformer ratio | M | |
| Cable sizing, motor efficiency, solar PV | M | |

### Engineering
| Solver | Effort | Notes |
|---|---|---|
| Gear & pulley ratios | S | |
| Lever / moment arm, spring rate (Hooke) | S | |
| Fluid mechanics (Bernoulli, continuity, Reynolds) | M | |
| Heat transfer (conduction, convection, radiation, LMTD) | M | |
| Section properties (I, Z, centroid) | M | |
| Torsion & shaft design | M | |
| Column buckling (Euler) | S | |
| Friction & statics (reactions, centroid, truss) | M | |
| Machining allowance, tolerance & fits | S | |
| Hardness conversion (Rockwell/Brinell/Vickers) | S | |
| Thermal expansion of materials | S | |
| Bearing life, screw jack, screw thread | M | |

### Computing, signals & control
| Solver | Effort | Notes |
|---|---|---|
| Truth tables, K-map simplification | M | |
| Boolean algebra | S | |
| Graph theory (degree, Euler path, shortest path) | M | |
| Recurrence relations | M | |
| IEEE 754 decoding | S | |
| Asymptotic complexity & master theorem | S | |
| Hashing (MD5/SHA, CRC) | S | |
| Floating-point representation | S | |
| Formal logic & proofs | M | |
| DFT / FFT | M | |
| Convolution & correlation | S | |
| Sampling theorem & aliasing | S | |
| Filter design (Butterworth, Chebyshev) | L | |
| Transfer function & poles/zeros | M | |
| Routh–Hurwitz stability | M | |
| Root locus | L | |
| PID tuning (Ziegler–Nichols) | M | |
| Steady-state error | M | |
| State space, controllability/observability | L | |

### Finance, maths of money & dates
| Solver | Effort | Notes |
|---|---|---|
| NPV & IRR | S | |
| Payback period, WACC, CAPM | S | |
| Bond pricing, duration, yield | M | |
| Annuity, sinking fund, mortgage amortisation | S | |
| Date difference, add/subtract, age, working days | S | |
| Fractions (add, subtract, multiply, divide, compare, reduce) | S | Pure functions |
| Scientific notation, Roman numerals | S | |
| Pythagorean theorem, law of sines & cosines | S | |
| Proportion / rule of three | S | |
| Area & volume of geometric solids | S | |
| Trigonometric identities, unit circle | M | |

---

## Part 2 — Gaps the Qt app never had

### Statistics & data
- Correlation matrix, outlier detection
- Time series: moving average, exponential smoothing
- Random sampling from a distribution
- Percentile & quartile calculators
- Weighted mean, harmonic mean, geometric mean
- Pooled variance, effect size (Cohen's d)

### Biology, medicine & health
- BMI & body surface area (Du Bois, Mosteller)
- eGFR & creatinine clearance
- Drug dosage by weight, infusion rates
- Cardiac output, stroke volume
- Lung function (FEV1, FEV1/FVC, PEF)
- Cholesterol & lipid panel interpretation
- Daily calorie and protein needs
- Pregnancy date, gestational age, ovulation
- Vision: Snellen and decimal acuity, near point

### Computing & developer tools
- Character, word and line counts
- ASCII / Unicode code point tables
- Base64, URL encode, JWT decode, UUID
- Regex tester and explainer
- Hash function properties
- Sorting algorithm comparison table
- Random number and dice probability
- Colour space conversion (RGB ↔ HSL ↔ CMYK)

### Practical, work & everyday
- Cooking: ingredient scaling by pan size, baker's percentages
- Fuel economy and cost per mile
- Photography: exposure, f-stop, depth of field
- Concrete mix design, soil bearing capacity (Terzaghi)
- Levelling and survey: area, gradient
- Time duration, current time, time zone difference

### Earth & space
- Solar declination, sunrise/sunset
- Tidal range and harmonic constituents
- Slope, grade, and ramp calculations
- Magnetic declination
- Seismic magnitude conversions (Richter, moment magnitude)

---

## Part 3 — Beyond calculators

Capabilities rather than individual tools. More work, much more distinctive.

- **Symbolic engine** — exact differentiation, integration, simplification
  and solving. The single biggest unlock: it turns dozens of individual
  solvers into one capability, and it is what makes the expression evaluator
  feel like a real CAS rather than a calculator.
- **Matrix input kind** — a `matrix` input type in the registry, so matrix
  solvers are first-class rather than CSV in a text box.
- **Symbolic result type** — the registry's `Computed` is
  `number | string | undefined`; symbolic results need an expression type
  and a renderer for it.
- **Step-by-step derivation** — show how a result was reached, not just the
  result. The `steps` field already exists on every tool; this is about
  filling it properly.
- **Reverse solving** — given an output, find the input that produces it.
  The `derive()` pattern generalised to optimisation.
- **History panel** — the Qt app had one; the web app does not.
- **Saved calculations & sharing** — the URL already carries state; a short
  link or QR would extend it.

---

## Part 4 — Architectural notes

Three families need the registry to grow before they can be expressed
cleanly. Doing these early means the tools land in the right shape.

1. **`matrix` input kind.** A `ToolInput` whose value is a 2D array. Without
   it, matrix solvers are CSV strings in a text box — unpleasant to use and
   impossible to validate. Affects: matrix operations, eigenvalues, state
   space, transfer functions, graph adjacency, truss analysis.

2. **Symbolic expressions as a first-class value.** The parser now produces
   an AST, which is the foundation for differentiation and simplification.
   But symbolic results need a new output type, and the renderer needs to
   show it.

3. **Unit-aware values.** Most engineering tools in the survey take imperial
   or metric per input. The current converters are separate tools; a unit
   type on inputs would let one tool serve both. Large change, wide payoff
   — a Phase 2 project.

A fourth, smaller one:

4. **Table-backed tools.** Wire gauge, resistor colour codes, material
   properties, standard steel sections and element data all need lookup
   tables. The registry has no notion of a tool shipping a reference table.
   A `tables` field on the descriptor would keep the data beside the
   solver that uses it.

---

## Part 5 — Recommended batches

Chosen for value per unit of effort, and because they cluster — each batch
teaches you the pattern for its neighbours.

### Batch 1 — quick wins, no new architecture (~12 solvers, ~3 days)
Voltage divider · linear system solver · GCD/LCM · modular arithmetic ·
circular motion · rotational dynamics · Hooke's law / spring · gear ratio ·
lever · NPV & IRR · date difference & age · fraction arithmetic

### Batch 2 — statistics (needs a distribution library, ~10 solvers, ~4 days)
Distributions (normal, t, χ², F, Poisson) · hypothesis tests · confidence
intervals · ANOVA · chi-square · frequency & histogram data · correlation ·
covariance · sample size & power

### Batch 3 — chemistry depth (~10 solvers, ~5 days)
Stoichiometry · concentration units · colligative properties · buffers &
Henderson–Hasselbalch · titration · equilibrium constants · Hess's law ·
kinetics & Arrhenius · electrochemistry & Nernst · Beer–Lambert

### Batch 4 — engineering mechanics (~10 solvers, ~5 days)
Section properties · torsion & shafts · column buckling · statics &
reactions · truss analysis · heat transfer · fluid mechanics · machine
design (bearings, screws, springs) · manufacturing tolerance · surveying

### Batch 5 — the matrix input kind, then linear algebra (~8 solvers, ~6 days)
First the `matrix` input kind, then: matrix operations · determinant &
inverse · eigenvalues & eigenvectors · matrix powers · systems of equations ·
linear transformations · least squares · state space

### Batch 6 — signals, control and computing (~10 solvers, ~7 days)
DFT/FFT · convolution · sampling theorem · filter design · transfer
function · Routh–Hurwitz · root locus · PID tuning · truth tables & K-map ·
complexity & master theorem

### Batch 7 — health and practical (~12 solvers, ~4 days)
BMI & BSA · eGFR · drug dosage · cardiac output · lung function · calorie
needs · cooking scaling · fuel economy · photography exposure · ASCII/Unicode ·
base64/URL/JWT · regex tester

**Total from these seven batches: about 60 new solvers**, taking Caliber from
51 to roughly 110 — and every one of them is client-side, dependency-free and
expressible in the registry as it stands, except the matrix family, which
needs Batch 5's groundwork first.

---

## Notes on sourcing

Formula inventories surveyed: EngForge (188 tools), Chemsolved (81 chemistry
tools), Physics Formula Sheets (174), PhysRef (252), Formulaxis, MegaCalc,
The Engineering ToolBox, calculators.org, Engineering SuperHub, Tafelwerk.

Useful for spotting which tools a student expects to find. The formulas
themselves are standard textbook material and worth deriving rather than
copying, so that the reference notes and step-by-step working can be written
properly.
