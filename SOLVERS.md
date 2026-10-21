# Solver Backlog

A survey of solvers available to implement, gathered from three sources:

1. **The Qt app's own history** (`b5c06dd`) — ~60 solvers that were written
   for the desktop app and never ported. The formulas exist; only the
   registry entry is missing. This is the cheapest possible supply of
   solvers.
2. **The standard undergraduate curriculum** — what a degree in
   mechanical, electrical, civil or chemical engineering actually covers.
3. **A survey of comparable public calculators** — EngForge, Formulaxis,
   MegaCalc, The Engineering ToolBox — to check for blind spots.

Current inventory: **51 solvers** across 11 categories.

Priorities use **effort** (S = a day, M = a few days, L = a week or more)
and **value** (how many courses and exercises it touches).

---

## Tier 1 — port from the Qt history (formulas already written)

Everything here was already implemented in the desktop app. Port it is
mechanical: read the C++, write the descriptor.

### Mathematics & Linear Algebra

| Solver | Effort | Notes |
|---|---|---|
| Linear system solver (Gaussian elimination, 2×2/3×3) | S | Pair with the existing quadratic solver |
| Matrix operations (add, multiply, determinant, inverse, transpose) | M | Needs a `matrix` input kind — see *Architectural notes* |
| Eigenvalues & eigenvectors | L | Characteristic polynomial; numerical methods for 3×3+ |
| Complex numbers (arithmetic, polar form, De Moivre) | M | |
| Polynomial operations (add, multiply, divide, synthetic division) | M | |
| Sequences & series (arithmetic, geometric, Fibonacci, Σ) | S | |
| Set theory (union, intersection, power set, cardinality) | S | |
| GCD / LCM | S | The Qt app had it; the web app has divisors but not this |
| Modular arithmetic (mod, modpow, inverse, CRT) | S | |
| Number theory (primality, Euler's totient, factorisation methods) | S | |

### Calculus & Analysis

| Solver | Effort | Notes |
|---|---|---|
| Numeric differentiation | S | Already exist in the grapher — expose as a tool |
| Numeric integration (definite) | S | Adaptive Simpson already exists in `lib/graph.ts` |
| Definite limits (L'Hôpital, ∞−∞) | M | |
| Taylor / Maclaurin series | M | |
| Root finder (Newton–Raphson, bisection, secant) | S | |
| Symbolic differentiation | L | Needs a symbolic engine — see *Architectural notes* |
| Symbolic integration | L | Needs a symbolic engine |

### Statistics

| Solver | Effort | Notes |
|---|---|---|
| Probability distributions (normal, t, χ², F, Poisson) | M | CDF, PDF, quantiles |
| Hypothesis tests (z, t, χ², F) | M | |
| Confidence intervals | S | |
| ANOVA | M | |
| Chi-square test | S | |
| Frequency distribution & histogram data | S | |
| Covariance, Pearson & Spearman correlation | S | |

### Physics

| Solver | Effort | Notes |
|---|---|---|
| Circular motion (centripetal force, period) | S | |
| Rotational dynamics (torque, moment of inertia, angular momentum) | M | |
| Gravitation (Newton, escape velocity, orbital period) | S | |
| Electrostatics (Coulomb, E-field, potential, capacitors) | M | |
| Magnetism (force on a current, solenoid, flux) | M | |
| Thermodynamics (Carnot efficiency, entropy, heat capacity) | M | |
| Waves & optics (Snell, critical angle, thin lens, mirrors) | M | |
| Relativity (time dilation, length contraction, E = mc²) | S | |

### Chemistry

| Solver | Effort | Notes |
|---|---|---|
| Stoichiometry (limiting reagent, theoretical & % yield) | M | High value for coursework |
| Combined gas law & Dalton's law of partial pressures | S | |
| Concentration (molarity, molality, normality, ppm, mass %) | S | |
| Calorimetry & specific heat | S | |
| Hess's law & enthalpy of reaction | M | |
| Equilibrium constants (Kc, Kp, Le Chatelier) | M | |
| Electrochemistry (Faraday's law, Nernst equation) | M | |
| Nuclear half-life & decay | S | |

### Electrical

| Solver | Effort | Notes |
|---|---|---|
| Voltage divider | S | |
| RC / RL time constants, capacitor & inductor | S | |
| Op-amp (inverting, non-inverting) | S | |
| dB converter & power factor | S | |
| Phasors (AC complex) | M | |
| Resistor colour code decoder | S | Trivial, very popular |
| AWG wire gauge | S | Trivial, includes ampacity table |
| Antenna length & dBm / path loss | S | |

### Engineering (mechanical, civil)

| Solver | Effort | Notes |
|---|---|---|
| Gear & pulley ratios | S | |
| Lever / moment arm | S | |
| Spring rate (Hooke's law) | S | |
| Fluid mechanics (Bernoulli, continuity, Reynolds) | M | |
| Heat transfer (conduction, convection, radiation) | M | |
| Section properties (I, Z, centroid) | M | |
| Torsion & shaft design | M | |
| Column buckling (Euler) | S | |
| Friction & statics (reactions, centroid) | M | |
| Gear & pulley / screw & bolt torque | S | |

### Computing, Signals & Control

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

### Finance & Dates

| Solver | Effort | Notes |
|---|---|---|
| NPV & IRR | S | |
| Payback period, WACC, CAPM | S | |
| Bond pricing, duration, yield | M | |
| Annuity & sinking fund | S | |
| Date difference, add/subtract, age | S | |
| Working days, day of week | S | |

---

## Tier 2 — gaps the Qt app never had

Found by comparing against the current curriculum and public calculators.

### Statics & structures
- Centroid & centre of mass, composite bodies
- Truss analysis (method of joints)
- Shear force & bending moment diagrams
- Mohr's circle (stress transformation)
- Stress concentration factors
- Column design, footing bearing pressure

### Rotational mechanics
- Torque from force and lever arm; power from torque and RPM
- Shaft torsion & angle of twist
- Bearing life, screw jack, screw thread
- Four-bar linkage, cam profile

### Manufacturing & materials
- Machining allowances, tolerance & fits
- Hardness conversion (Rockwell/Brinell/Vickers)
- Thermal expansion of materials
- Creep, fatigue (S-N curve)

### Process & energy
- Latent heat, thermal equilibrium temperature
- Heat exchanger LMTD & NTU
- Psychrometrics (humidity, dew point, wet bulb)
- Refrigeration COP, vapour compression
- Pump & fan affinity laws
- Darcy–Weisbach friction factor, Venturi meter

### Electrical (advanced)
- Three-phase power, transformer ratios & VA rating
- Cable sizing & ampacity
- Motor efficiency & starting
- Solar PV array sizing
- Transmission line / impedance
- Capacitor bank & power factor correction

### Statistics & data
- Correlation matrix, outlier detection
- Sample size & statistical power
- Bayesian probability & Bayes theorem
- Time series: moving average, exponential smoothing
- Random number & distribution sampling

### Computing
- ASCII/Unicode encoding tables
- Regular expressions tester
- Hash function properties, rainbow tables
- Complexity classes comparison table
- Sorting algorithm comparison

### Biology & medicine (optional)
- BMI & body surface area (Du Bois)
- eGFR / creatinine clearance
- Drug dosage by weight, infusion rates
- Poiseuille flow, lung capacity
- Epidemic models (SIR/SEIR)

---

## Tier 3 — beyond calculators

Things that make the project *feel* like a solver rather than a formula
lookup. More work, more distinctive.

- **Symbolic engine** — exact differentiation, integration, simplification,
  and solving. This is the single biggest unlock: it turns dozens of
  individual solvers into one capability, and it is what makes the
  expression evaluator feel like a real CAS.
- **Matrix input kind** — a `matrix` input type in the registry, so matrix
  solvers are first-class rather than CSV in a text box.
- **Symbolic/numeric step-by-step derivation** — show how a result was
  reached, not just the result.
- **Reverse solving** — given an output, find the input that produces it
  (the `derive()` pattern generalised to optimisation).
- **Saved calculations & sharing** — the URL already carries state; a short
  link or QR would extend it.
- **History panel** — the Qt app had one; the web app does not.

---

## Architectural notes

Three families need changes to the registry before they can be expressed
cleanly. Worth doing early so the tools land in the right shape.

1. **`matrix` input kind.** A `ToolInput` whose value is a 2D array. Without
   it, matrix solvers are CSV strings in a text box, which is unpleasant to
   use and impossible to validate. Affects: matrix operations, eigenvalues,
   state space, transfer functions, graph adjacency.

2. **Symbolic expressions as a first-class value.** The parser now produces
   an AST, which is the foundation for differentiation and simplification.
   But the registry's `Computed` type is `number | string | undefined`.
   A symbolic result needs an expression type, and the output renderer
   needs to show it.

3. **Unit-aware values.** Most engineering tools in the survey take imperial
   or metric per input. The current converters are separate tools; a unit
   type on inputs would let one tool serve both. Large change, wide payoff —
   probably a Phase 2 project.

---

## Recommended first batch

Chosen for value per unit of effort, and because they cluster — each one
teaches you the pattern for its neighbours.

| # | Solver | Why now |
|---|---|---|
| 1 | Voltage divider | S, 2 minutes, and it is in every circuit course |
| 2 | Linear system solver | S, completes the equations family |
| 3 | GCD / LCM + modular arithmetic | S, three tools from one number-theory table |
| 4 | Probability distributions (normal/t/χ²/F) | M, unlocks hypothesis tests and confidence intervals |
| 5 | Circular motion + rotational dynamics | S/M, completes mechanics |
| 6 | Stoichiometry | M, high exam value, and the chemistry set is thin |
| 7 | Numeric derivative / integral / root finder | S, reuses existing maths in `lib/graph.ts` |
| 8 | Combinatorics (nCr, nPr, Catalan, derangements) | S, pure functions, no tables |
| 9 | Date tools (difference, add, age, working days) | S, self-contained, immediately useful |
| 10 | Series & sequences | S, pure functions |

That is roughly **25 new solvers** from about a week of work, and it takes
the count from 51 to around 75 while closing the most obvious gaps.

---

## Notes on sourcing

Comparable public calculators surveyed for coverage:
EngForge (188 tools), Formulaxis, MegaCalc, The Engineering ToolBox,
engineerscalc.com, Engineering SuperHub. Useful for spotting which tools
a student expects to find — but the formulas are standard textbook
material and worth deriving rather than copying, so that the reference
notes and step-by-step working can be written properly.
