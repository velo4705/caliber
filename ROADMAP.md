# Caliber Roadmap

Rewrite of the Qt-era roadmap. The desktop app is gone; everything below is
scoped to the web app in `web/`.

---

## Now — ship the foundation

- [x] Registry-driven tool architecture (a solver is data, not a component)
- [x] 46 solvers across 10 categories
- [x] Expression engine ported from C++
- [x] Hash routing with shareable state
- [x] Responsive layout, light/dark themes
- [x] CI: typecheck → registry validation → build

---

## Done — graphing

- [x] **Registry integration.** `kind: 'chart'` discriminator; `Tool` is a
      discriminated union so charts stay searchable and deep-linkable without
      `ToolView` pretending a canvas is a form.
- [x] 2D cartesian plotting with adaptive sampling (260 pts dragging, 1400 idle)
- [x] Implicit plots via marching squares
- [x] Parametric curves and polar plots
- [x] Pointer pan, wheel zoom, two-finger pinch zoom
- [x] Hover tracing with a coordinate readout
- [x] Per-series visibility toggles, reset, example presets
- [x] Graph maths covered by the self-check, including every example
- [x] Parameter sweeps — `a = [1...5]`, explicit lists, and `a = [0...1...0.1]`
      with an explicit step; multi-parameter cartesian product capped at 240
- [x] Derivative overlay (numeric central difference) and integral shading with
      the value reported, using adaptive Simpson integration
- [x] Axis crossing detection with labelled intercepts — x-intercepts found by
      bisection on sign changes, y-intercept evaluated directly, with label
      collision avoidance

## Next — graphing depth

- [x] 3D surfaces, hand-rolled on canvas — no three.js, keeping "React only"
      as a dependency rule. Rotate, zoom, wireframe and solid modes, viridis
      height ramp, painter's algorithm sorted by view depth
- [x] AST-compiled expression evaluator — parse once, evaluate many.
      2.76× faster than re-parsing on every call
- [ ] Tangent-root detection — a double root away from a sample point is
      currently missed, since only sign changes and exact zeros are found
- [ ] Perspective projection and a real lighting model for the 3D surface
- [ ] Axis labels on the 3D surface, and a height legend

---

## Depth — make existing solvers better

Tier 1 is compute-only. Tier 2 adds the full treatment. Prioritise by usage.

- [ ] Tier 2 reference content: formula, derivation, worked examples, units
- [ ] Unit-aware inputs across physics, chemistry and engineering
- [ ] Per-tool test fixtures in the self-check harness
- [ ] Chart outputs where they genuinely help:
  - descriptive stats → histogram, box plot
  - regression → scatter with fitted line
  - binomial → PMF bar chart
- [ ] Richer keywords and descriptions for search quality

---

## Coverage — reach toward "universal"

See **[SOLVERS.md](SOLVERS.md)** for the full survey: ~60 solvers that existed
in the Qt app and were never ported, plus systematic formula inventories for
physics (174 and 252 formula sheets), chemistry (81 tools), statistics,
finance, medicine and computing.

Current: 51 solvers across 11 categories.

- [ ] **Batch 1 — quick wins** (~12, ~3 days): voltage divider, linear
      systems, GCD/LCM, modular arithmetic, circular & rotational motion,
      Hooke's law, gear ratio, lever, NPV/IRR, date tools, fraction arithmetic
- [ ] **Batch 2 — statistics** (~10, ~4 days): distributions, hypothesis
      tests, confidence intervals, ANOVA, chi-square, correlation, power
- [ ] **Batch 3 — chemistry** (~10, ~5 days): stoichiometry, concentration
      units, colligative properties, buffers, titration, equilibrium, Hess's
      law, kinetics, electrochemistry, Beer–Lambert
- [ ] **Batch 4 — engineering mechanics** (~10, ~5 days): section
      properties, torsion, buckling, statics, trusses, heat transfer, fluids
- [ ] **Batch 5 — matrix input kind, then linear algebra** (~8, ~6 days)
- [ ] **Batch 6 — signals, control & computing** (~10, ~7 days)
- [ ] **Batch 7 — health & practical** (~12, ~4 days)

---

## Quality

- [x] Headless browser verification (`npm run verify`) — asserts pages render,
      the canvas is painted, search and URL state work, and there are no
      console errors; wired into CI as a gate
- [ ] Replace `scripts/selfcheck.ts` with Vitest
- [ ] Property-based tests for the expression parser
- [ ] Accessibility audit — keyboard navigation, focus order, ARIA, contrast
- [ ] Visual regression baseline, so a layout regression fails the build
- [ ] SEO: Open Graph tags, sitemap, per-tool meta descriptions
- [ ] Bundle budget monitoring; currently 236 KB / 77 KB gzipped

---

## Distribution

- [ ] PWA: web manifest, service worker, offline caching, install prompt
- [ ] Per-tool documentation pages, generated from the registry's `reference`
- [ ] In-app link to share a calculation (Web Share API on mobile)
- [ ] Custom themes, mirroring the gradient themes the Qt app had
- [ ] Lightweight themes via CSS custom properties persisted to `localStorage`

---

## Deliberately not planned

- Backend / user accounts. Everything is client-side by design.
- Cloud sync or history across devices. Keep state in the URL.
- Any solver that requires a paid API key.