# Caliber — Project Context

Transfer document for continuing work with another AI client.

---

## Project

**Caliber** — a universal STEM solver that runs entirely in the browser.
Client-side only: no backend, no API keys, no network calls. Deployable to any
static host.

**Repo:** https://github.com/velo4705/caliber
**Root:** `~/Projects/caliber/`
**App:** `web/`
**Binary name:** n/a (static site)
**Version:** 2.0.0

---

## Status

The project was a Qt6/C++ desktop calculator until commit `90a7fd8`, which
replaced it with a React + Vite + TypeScript web app. **The Qt sources are
deleted from `main` but preserved in git history** — use as the reference when
porting anything not yet rebuilt.

Key history commits:

| Commit | Contents |
|---|---|
| `b5c06dd` | Last Qt graphing work — implicit plotting, parameter sweeps, auto plot-mode detection. **Reference implementation for the web grapher.** |
| `a239d91` | Qt graphing: right sidebar, keyboard navigation |
| `90a7fd8` | The web app. Native app deleted. |

To read the old Qt sources: `git show b5c06dd:src/modes/graphing/graphing_widget.cpp`

---

## Tech Stack

| Component | Choice | Why |
|---|---|---|
| Build | Vite 6 | Fast, zero-config static output |
| UI | React 18 | Ecosystem, forms |
| Language | TypeScript (strict) | `strict`, `noUnusedLocals`, `noUncheckedIndexedAccess` |
| Runtime deps | **React only** | Everything else is hand-written |
| Routing | Hash router (~40 lines) | No server rewrite rules needed on static hosts |
| CI | GitHub Actions | typecheck → registry check → build |

No charting, graphing, or CAS library is included yet — deliberately.

---

## Architecture

The single most important idea: **a solver is data, not a component.**

```
web/src/
├── registry/
│   ├── types.ts       Tool, ToolInput, ToolOutput, Category interfaces
│   ├── categories.ts  the 10 categories, in display order
│   └── index.ts       loader + validateTools() + searchTools()
├── tools/             one file per category, exports Tool[]
├── lib/
│   ├── expr.ts        recursive-descent parser (port of the C++ engine)
│   ├── router.ts      hash routing: #/ , #/c/<cat> , #/t/<id>?state
│   ├── hooks.ts       useRoute, useTheme
│   ├── values.ts      num/str/list/derive input coercion helpers
│   └── format.ts      number formatting (sig digits, scientific)
├── components/
│   ├── ToolView.tsx   the tool page
│   ├── InputField.tsx number / number-list / expression / select / text
│   ├── OutputGrid.tsx results, with "needs X" hints
│   └── CategoryRail.tsx
└── App.tsx            shell, search, theme toggle, home/listing views
```

Nav, search, category grouping, deep links and copy are **all derived from the
registry**. Adding a solver = adding one file. Never hardcode a tool list.

---

## Adding a solver

1. Add a `Tool` object to the relevant file in `src/tools/`.
2. Export it from that file's array.
3. Run `npm run check`.

`validateTools()` catches: duplicate IDs, unknown categories, duplicate input
or output keys, `compute()` returning undeclared keys, declared outputs that
`compute()` never returns, and `compute()` throwing.

### Solve-for-the-unknown pattern

`derive()` returns the first finite candidate, so an entered value wins and a
missing one is computed:

```ts
compute: (v) => {
  const V = num(v, 'V'), I = num(v, 'I'), R = num(v, 'R')
  const Vo = derive(V, I * R)   // if V given, echo it; else I*R
  const Io = derive(I, Vo / R)  // if I given, echo it; else derive
  return { V: Vo, I: Io, R: derive(R, Vo / Io) }
}
```

Use it in every solver where the terms are interchangeable. Divide-by-zero
returns `undefined`, which renders as "—".

---

## Current Inventory: 50 solvers

| Category | Count | Tools |
|---|---|---|
| `math` | 5 | quadratic, divisors, combinations, percent-change, logarithm |
| `statistics` | 4 | descriptive, zscore, regression, binomial |
| `geometry` | 4 | vector-magnitude, dot-product, cross-product, triangle |
| `physics` | 5 | kinematics, projectile, kinetic-energy, wave, newton |
| `chemistry` | 4 | molar-mass, ph, dilution, ideal-gas |
| `electrical` | 3 | ohm, resistor-network, ac-impedance |
| `engineering` | 3 | beam-deflection, stress-strain, material-select |
| `computing` | 3 | number-base, bitwise, expression-eval |
| `finance` | 3 | compound-interest, loan-payment, break-even |
| `units` | 12 | length, mass, volume, time, speed, energy, pressure, area, force, angle, data, temperature |
| `graphing` | 4 | graph-2d, graph-3d-projection (parametric), graph-polar, graph-implicit |

---

## Two Tool Kinds

`Tool` is a **discriminated union**, not one loose interface:

```ts
export type Tool = SolverTool | ChartTool

interface SolverTool { kind?: 'solver'; inputs; outputs; compute; steps? }
interface ChartTool   { kind: 'chart';  graph: GraphSpec }
```

`App.tsx` branches on `tool.kind` to render `ToolView` or `GraphView`. This
keeps graphing inside the registry — so it is searchable and deep-linkable —
without pretending an interactive canvas is a form. **When adding a new kind,
narrow before touching fields; do not add optional fields to both arms.**

`validateTools()` skips the `compute()` probe for charts and instead checks the
graph mode and function keys.

---

## Graphing

Pure maths lives in `src/lib/graph.ts` and is **covered by the self-check** —
deliberately, because the rendering itself cannot be tested without a browser.
`scripts/selfcheck.ts` asserts viewport transforms, tick generation, and that
every chart descriptor *and every example* produces geometry.

`GraphView.tsx` is the only stateful component: canvas rendering, pointer pan,
wheel/pinch zoom, and hover tracing. Series are recomputed in a `useMemo` at
reduced resolution while dragging (260 points) and full resolution (1400) when
idle, because the parser re-parses the string on every `evaluate()` call.

The component builds an `evaluators` memo (one compiled closure per plotted
expression) that `series`, `derivatives` and `shaded` all read from. Do not
recompile per feature — a second `compile` pass would double the parse cost on
every drag frame.

### Derivatives and integrals

`numericDerivative()` is a central difference with `h = max(|x|, 1) × 10⁻⁵`.
Symbolic differentiation is deliberately *not* attempted: the parser is
recursive descent returning values, so there is no AST to differentiate. A
symbolic layer would require reworking `expr.ts`.

`integrate()` is **adaptive Simpson** (Richardson correction, depth 20). It
probes 25 points across the interval first and returns `undefined` if any are
non-finite, so an integral across a pole is refused rather than silently
returning a wrong number. Verified to 3.2e-14 on ∫₀^π sin x dx.

Both the derivative and the integral need a finite region, so both refuse
rather than producing nonsense. Keep that behaviour: a plausible-looking wrong
number is worse than an em dash.

### Intercepts

`findXIntercepts()` scans the visible viewport for sign changes and refines
each bracket by bisection, so the roots are exact to machine precision rather
than snapped to a sample. `findYIntercept()` evaluates at x = 0 directly and
returns `undefined` when the y-axis is off screen.

**Known limitation:** only sign changes and exact zeros are detected. A tangent
root that falls between two samples — `(x − 0.37)²` — is missed, because the
function never changes sign. The self-check asserts this limitation explicitly
so it is not mistaken for a bug later.

Labels use `formatRoot()` (five significant figures), not `formatTick()` (which
is tuned for axis marks). Labels are placed by trying a few candidate positions
and skipping any that would overlap a label already drawn, so crowded sweeps
stay readable.

- `sampleCartesian` splits into multiple polylines at non-finite or
  out-of-window values, so asymptotes do not draw vertical artefacts.
- `sampleImplicit` is marching squares over the viewport grid.
- `samplePolar` substitutes θ for `t`; `sampleParametric` pairs `x1`/`y1` keys.

### Parameter sweeps — `a = [1...5]`, `a = [1, 2, 4]` or
`a = [0...1...0.1]` — becomes a parameter definition rather than a curve. Other
slots referencing that name are expanded over `paramProduct()` (the cartesian
product across all parameters, capped at 240 combinations).

**Parameters are bound as parser variables, not substituted into the string.**
This is a deliberate improvement on the Qt implementation, which used a
regex replace. With `a = [1...5]`, the expression `atan(2)` would have had its
`a` clobbered into `5tan(2)` by naive substitution. `usesParam()` matches only
whole identifiers, and the sampler passes `{ ...combo, x }` to `parser.setVars`.
The self-check asserts exactly this.

---

## Commands

```bash
cd web
npm install
npm run dev          # dev server
npm run build        # → web/dist
npm run preview      # serve production build
npm run typecheck    # tsc --noEmit
npm run check        # registry validation + expression tests ← run this
npm run verify       # headless browser checks (needs the dev server running)
```

`npm run check` is headless and runs in CI. `npm run verify` drives a real
Chromium through Playwright: it asserts that pages render, that the canvas is
actually painted (by counting distinct pixel colours), that search and URL
state work, and that there are no console errors. It writes annotated
screenshots to `web/screenshots/` — **look at them.** The first run of this
caught five bugs the maths tests could not: duplicate React keys, every curve
in a sweep sharing one colour, a false "unknown name" error, a control
overlapping the axis labels, and a stale hardcoded tool count.

---

## Expression Engine

`src/lib/expr.ts` — recursive descent, ported from the C++ `ExpressionParser`.

- `+ - * / % ^`, unary minus, parentheses, factorial `!`
- **implicit multiplication**: `2(3+4)`, `3sin(x)`
- precedence: `parseExpression → parseTerm → parseUnary → parsePower → parsePostfix`
- `^` is **right associative**; unary binds looser than power (`-2² = -4`)
- ~40 functions: trig (angle-mode aware), `ln/log/log2`, `sqrt/cbrt/exp`,
  `min/max/pow/hypot/gcd/lcm/mod/atan2`, `fact`
- constants: `pi` `π` `e` `tau` `phi`
- factorial: exact for integers, Lanczos gamma otherwise
- results memoised in a 500-entry cache
- throws `ExprError` with a character position

`expression-eval` is a live user-facing tool for this engine, plus a test
surface. Self-check covers precedence, associativity and implicit multiply.

---

## URL State

Values live in the hash, so every calculation is a shareable link:

```
#/t/ohm?V=12&R=470
#/t/quadratic?a=1&b=-3&c=2
```

`ToolView` writes non-default values to the hash with `history.replaceState`,
so the browser back button is not spammed.

---

## Conventions

- **Strict TypeScript.** No `any`. `noUnusedLocals` and
  `noUnusedParameters` are on — the build will fail on dead code.
- **No comments** unless they explain a non-obvious constraint.
- **`Computed` is `number | string | undefined`.** String outputs exist (hex,
  triangle type). In `steps()` and any arithmetic, coerce with
  `Number(x)` or `formatNumber(x)` — don't assume numeric.
- **CSS is one file**, `src/styles.css`, with custom properties for theming.
  `[data-theme='dark']` overrides the tokens. Mobile breakpoint at 900px.
- `vite.config.ts` uses `base: './'` — required for static hosts on subpaths.

---

## Known Gaps

- **No charting library.** The grapher draws its own canvas primitives, which
  is fine for 2D curves but means there are no statistical charts (histogram,
  regression scatter) yet.
- **No 3D surfaces.** The Qt version had Q3DSurface plots; the web version has
  no equivalent. `three.js` would be the first runtime dependency beyond React.
- **The parser re-parses on every call.** Adequate at current sample counts
  (260–1400 points), but an AST-compiled evaluator would be the fix if the
  grapher ever feels slow.
- **Not yet a PWA** — no manifest, service worker, or offline caching.
- **No unit-test framework.** `scripts/selfcheck.ts` is a bespoke harness;
  consider replacing with Vitest.
- **No visual regression baseline.** `npm run verify` catches breakage but
  does not diff against a previous render.
- SEO/meta is minimal — one description tag, no OG tags, no sitemap.
- No accessibility audit; keyboard focus exists but has not been tested.
- Tool descriptions/keywords are thin. Worth enriching for search quality.

---

## Platform

- **Linux:** primary. `npm run build && serve web/dist`.
- **Windows / macOS:** identical — it's a static site.
- **Mobile:** responsive CSS, but untested on real devices.
- **Offline:** works only once loaded, unless a service worker is added.

---

## Important Notes for Continuing Work

1. **Never hardcode tool lists.** Sidebar, search, category counts and the
   home grid all read from the registry. Adding a tool should not require
   touching `App.tsx` or `CategoryRail.tsx`.
2. **Run `npm run check` after adding any solver.** The validator has already
   caught one real bug (a unit-label case mismatch in the pressure converter
   that made it silently return `{}`).
3. **The Qt code is in git history, not the working tree.** `git show
   b5c06dd:src/<path>` is the fastest way to recover any formula or algorithm
   that has not been ported yet. Use it as a checklist for the port.
4. **`web/` is the whole product.** There is no server component to keep in
   sync; if something needs a backend, that is a design change, not an
   implementation detail.
5. `CONTEXT.md`, `ROADMAP.md` and `notetoself.md` are gitignored local files.
   Keep them current or they will mislead the next session — as they did
   after the Qt era.