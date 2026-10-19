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

## Next — graphing depth

- [ ] 3D surfaces — decide if `three.js` is acceptable as the first real
      runtime dependency beyond React
- [ ] Integral shading and derivative overlay (the Qt app had both)
- [ ] Axis crossing detection and labelled intercepts
- [ ] AST-compiled evaluator, if the re-parsing cost ever becomes visible

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

- [ ] Audit the Qt tools still unported, tool by tool (`git show b5c06dd:src/`)
- [ ] Series expansion of thin categories: thermodynamics, organic chemistry,
      control systems, discrete math, civil/mechanical
- [ ] Symbolic layer: differentiation, integration, limits, matrix operations
- [ ] Matrix / linear algebra tools (eigenvalues, LU, determinants)
- [ ] Number theory expansion: modular arithmetic, sequences, recurrence
- [ ] Interpolation and numerical methods: Newton–Raphson, bisection, spline

---

## Quality

- [ ] Replace `scripts/selfcheck.ts` with Vitest
- [ ] Property-based tests for the expression parser
- [ ] Accessibility audit — keyboard navigation, focus order, ARIA, contrast
- [ ] Real-browser verification (no headless browser was available in the dev
      environment, so the UI is currently unverified visually)
- [ ] SEO: Open Graph tags, sitemap, per-tool meta descriptions
- [ ] Bundle budget monitoring; currently 214 KB / 69 KB gzipped

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