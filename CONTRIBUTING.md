# Contributing to Caliber

Caliber is a universal STEM solver that runs entirely in the browser. It has
one organising idea, and almost every contribution is easier once you know it.

> **A solver is data, not a component.**

Everything — the sidebar, search, category grouping, shareable URLs, the copy
button — is *derived* from a registry of plain objects. Adding a solver means
adding one file. If your change touches more than that, something has gone
wrong.

---

## Getting set up

Requires **Node 22 or newer**.

```bash
git clone https://github.com/velo4705/caliber.git
cd caliber/web
npm install
npm run dev
```

The dev server runs on <http://localhost:5173> with hot reload.

The application lives entirely in `web/`. The old Qt desktop app was removed
in `4222ca3`; its source is still in git history at `b5c06dd` and is a useful
reference when porting a formula.

---

## The commands

| Command | When |
|---|---|
| `npm run check` | Before every commit. Registry validation and the maths suite. |
| `npm run typecheck` | Before every commit. `tsc --noEmit`. |
| `npm run verify` | Before opening a PR. Drives a real browser. |
| `npm run build` | Before opening a PR. |
| `npm run bench` | If you touched the expression engine. |

`npm run check` and `npm run typecheck` are headless and run in CI.
`npm run verify` needs a server:

```bash
npm run dev        # terminal one
npm run verify     # terminal two
```

It writes annotated screenshots to `web/screenshots/`. **Look at them.** The
first time this suite ran it caught five bugs that the maths tests could not:
duplicate React keys, every curve in a parameter sweep sharing one colour, a
false "unknown name" error, a control overlapping the axis labels, and a
stale hardcoded tool count. A passing test suite is not the same as a
correct UI.

---

## Adding a solver

Solvers live in `web/src/tools/`, one file per category. Open the right one
and add an entry to its exported array.

```ts
// web/src/tools/electrical.ts
export const electricalTools: Tool[] = [
  // … existing tools
  {
    id: 'resistor-divider',
    category: 'electrical',
    name: 'Voltage Divider',
    tagline: 'Split a supply across two resistors',
    keywords: ['divider', 'potential', 'tap'],
    inputs: [
      { key: 'Vs', label: 'Supply voltage', unit: 'V', default: '12' },
      { key: 'R1', label: 'R1', unit: 'Ω', default: '1000' },
      { key: 'R2', label: 'R2', unit: 'Ω', default: '2000' },
    ],
    outputs: [
      { key: 'out', label: 'Output voltage', unit: 'V', formula: 'Vs · R2/(R1+R2)', emphasis: 'primary' },
      { key: 'current', label: 'Current', unit: 'A', formula: 'Vs/(R1+R2)' },
    ],
    compute: (v) => {
      const Vs = num(v, 'Vs')
      const R1 = num(v, 'R1')
      const R2 = num(v, 'R2')
      if (Vs === undefined || R1 === undefined || R2 === undefined || R1 + R2 === 0) return {}
      return { out: (Vs * R2) / (R1 + R2), current: Vs / (R1 + R2) }
    },
    reference: ['Vout = Vs · R2 / (R1 + R2)'],
    examples: [{ label: '1k / 2k from 12 V', inputs: { Vs: '12', R1: '1000', R2: '2000' } }],
  },
]
```

### Rules the validator enforces

`npm run check` fails if you break any of these, so you will find out
immediately:

- `id` is unique across the whole registry
- `category` matches an id in `src/registry/categories.ts`
- input and output keys are unique
- `compute()` never returns a key that is not a declared output
- every declared output is reachable — `compute()` must return it
- `compute()` does not throw when called with the default inputs

### Solving for an unknown

Where the terms of a formula are interchangeable, use `derive()` so the user
can supply any sufficient subset:

```ts
const Vo = derive(V, I * R)    // if V was given, echo it; otherwise compute it
const Io = derive(I, Vo / R)   // if I was given, echo it; otherwise derive it
return { V: Vo, I: Io, R: derive(R, Vo / Io) }
```

`derive()` returns the first finite candidate, so an entered value always
wins. Division by zero yields `undefined`, which the UI renders as an em
dash rather than `Infinity` or `NaN`.

### Showing working

`steps` is optional and renders a "Show steps" button. Keep it to a few
lines that show the substitution, not a derivation:

```ts
steps: (_v, r) => [
  `Vout = Vs · R2 / (R1 + R2) = ${fmt(r.out)} V`,
  `Current = Vs / (R1 + R2) = ${fmt(r.current)} A`,
],
```

The first argument is the raw inputs and the second is the computed result.
**Prefix an unused parameter with `_`** — `noUnusedParameters` is on, so
`steps: (v, r) =>` with no use of `v` fails the build.

Note that `r` is typed `number | string | undefined` — some outputs are
strings (hex digits, a triangle's type). Coerce with `Number(x)` before doing
arithmetic on it, or use `formatNumber` for display.

### Making it findable

`keywords` is what search matches against, and it is currently the weakest
part of the registry. When you add a solver, add the words somebody would
actually type — `"ohm"`, `"resistance"`, `"voltage"`, `"current"` — not just
the formal name. `name` and `tagline` are searched too, but only as whole
words.

---

## Adding a graph tool

Graph tools are a different kind of object in the same registry. See
`web/src/tools/graphs.ts` for four working examples.

```ts
{
  id: 'graph-2d',
  kind: 'chart',
  category: 'graphing',
  name: 'Function Grapher',
  graph: {
    mode: 'cartesian',          // cartesian | polar | parametric | implicit | surface
    viewport: { x0: -10, x1: 10, y0: -6, y1: 6 },
    functions: [{ key: 'f1', label: 'f₁(x)', default: 'sin(x)' }],
  },
}
```

All the maths lives in `web/src/lib/graph.ts` and `web/src/lib/graph3d.ts`,
and both are covered by the self-check. **Put new maths in the library, not
in the component** — the component is the only part that cannot be tested
without a browser.

---

## Code style

- **Strict TypeScript.** `strict`, `noUnusedLocals` and `noUnusedParameters`
  are on, so dead code fails the build. Never use `any` or `@ts-ignore`.
- **No comments** unless they explain a non-obvious constraint. A comment
  that restates the code is noise.
- **One file per category** in `web/src/tools/`, each exporting a `Tool[]`.
- **Never let user input throw during render.** `compileExpression()` raises
  on invalid syntax, and a half-typed expression is invalid. Compile inside a
  `try`, treat a failure as "no data", and show the message next to the
  field. A throw inside a `useMemo` unmounts the whole view.
- **Plain CSS** in `web/src/styles.css`, using the custom properties already
  defined there. `[data-theme='dark']` overrides the tokens; a new component
  should work in both themes without extra rules.
- **No runtime dependencies.** React is the only one. The expression parser,
  the hash router, the 2D grapher and the 3D renderer are all hand-written,
  and that is deliberate — see the README.

---

## Commits and pull requests

Commit messages follow the Conventional Commits style already used in this
repository:

```
feat(graphing): parameter sweeps
fix(units): pressure converter returned an empty result
refactor: extract the evaluators memo
test: add headless browser verification
docs: explain the derive() pattern
```

Please open a pull request against `main` with:

- a description of what changed and why
- the output of `npm run check` and `npm run typecheck`
- a note of what you verified in the browser, if the change is visual

If your change is visual, a screenshot before and after is worth more than
any amount of description.

---

## Publishing

The site is published by committing the built app to `docs/`:

```bash
cd web
npm run publish:pages
git add docs && git commit -m "build: publish" && git push
```

GitHub Pages serves it as a plain static site because the build includes a
`.nojekyll` file, which makes Pages skip Jekyll entirely. If you ever see
`No such file or directory: docs` from a Pages build, that file is missing.

## Where to help

The most useful contributions right now are **breadth**: 51 solvers is a long
way from "universal". The Qt app at `b5c06dd` contains tools that were never
ported — `git show b5c06dd:src/modes/<mode>/<mode>_widget.cpp` — and the
formulae there translate directly into registry entries.

Other things that would help:

- **Keywords and taglines.** Search quality is poor because the descriptors
  are thin. This is low-effort and high-impact.
- **Tier 2 solvers.** Every solver currently computes. Adding `reference`,
  `steps` and worked `examples` turns a calculator into a teaching tool.
- **Statistical charts.** Descriptive statistics should have a histogram;
  regression should have a scatter plot. The canvas renderer in
  `web/src/lib/graph.ts` can be reused.
- **A visual regression baseline.** `npm run verify` catches breakage but
  does not diff against a previous render.

---

## Licence

Caliber is MIT licensed — see [LICENSE](LICENSE). By contributing you agree
that your work is published under the same terms.
