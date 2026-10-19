# Caliber

**The universal STEM solver.** A collection of scientific, engineering and
financial calculators that runs entirely in your browser — no account, no
backend, no network calls.

> Caliber is a universal solver for STEM students, engineers and anyone who
> needs an answer to a formula quickly and can trust the result.

## Why it works the way it does

Most calculator apps hardwire every tool into the UI. In Caliber, **a solver is
data, not a component**. Each tool is a plain object:

```ts
export default {
  id: 'ohm',
  category: 'electrical',
  name: "Ohm's Law",
  keywords: ['ohm', 'resistance', 'voltage', 'current'],
  inputs:  [{ key: 'V', label: 'Voltage', unit: 'V' }, /* … */],
  outputs: [{ key: 'I', label: 'Current', unit: 'A', formula: 'I = V / R' }],
  compute: (v) => ({ V: derive(v.V, v.I * v.R), I: derive(v.I, v.V / v.R) }),
  reference: ['V = I × R', 'P = V × I = I²R = V²/R'],
}
```

The sidebar, search, category grouping, shareable URLs and the copy action are
all *derived* from that registry. Adding a solver means adding one file.

## Solvers

| Category | Count | Examples |
|---|---|---|
| Algebra | 5 | Quadratic roots, divisors, nCr/nPr, percent change, logarithms |
| Statistics | 4 | Descriptive, z-score, linear regression, binomial |
| Geometry & Vectors | 4 | Magnitude, dot & cross product, triangle solver |
| Physics | 5 | SUVAT, projectile, energy & momentum, waves, Newton's laws |
| Chemistry | 4 | Molar mass, pH, dilution, ideal gas law |
| Electrical | 3 | Ohm's law, resistor networks, AC RLC impedance |
| Engineering | 3 | Beam deflection, stress & strain, material properties |
| Computing | 3 | Number bases, bitwise ops, expression evaluator |
| Finance | 3 | Compound interest, loan repayment, break-even |
| Units | 12 | Length, mass, volume, time, speed, energy, pressure, area, force, angle, data, temperature |

## Solve for the unknown

Many solvers implement the `derive()` pattern: supply any sufficient subset of
terms and it returns the rest.

```
#t/ohm?V=12&R=470     →  I = 0.0255 A,  P = 0.306 W
#t/ideal-gas?v=5&n=2&t=300   →  P = 9.84 atm
```

Values live in the URL, so every calculation is a shareable link.

## Expression engine

Caliber ships its own recursive-descent parser — a TypeScript port of the
original C++ engine — so expressions evaluate instantly and offline:

```
2^3^2      →  512        (right associative)
-2^2       →  -4         (unary binds looser than power)
2(3+4)     →  14         (implicit multiplication)
sin(30)    →  0.5        (in degrees)
gcd(12,18) →  6
10!        →  3628800
```

Try it live at `#/t/expression-eval`. Supports 40+ functions, constants
(`pi`, `e`, `tau`, `phi`), factorial via Lanczos gamma, and DEG/RAD/GRAD modes.

## Running it

```bash
cd web
npm install
npm run dev        # dev server
npm run build      # production build → web/dist
npm run preview    # serve the production build locally
npm run typecheck  # tsc --noEmit
npm run check      # registry validation + expression engine tests
```

`npm run check` is the important one — it validates every registered tool at
build time (duplicate IDs, unknown categories, outputs that `compute()` never
returns, functions that throw) and runs the expression engine test suite.

## Deploying

`base` is set to `'./'` in `vite.config.ts`, so `web/dist/` works on any static
host without path rewriting.

```bash
npm run build
# copy web/dist/ to GitHub Pages, Netlify, Cloudflare Pages, S3, anything
```

## Architecture

```
web/src/
├── registry/      types, categories, loader + validator
├── tools/         one file per category — every solver is data
├── lib/           expression parser, hash router, hooks, formatting
└── components/    ToolView, InputField, OutputGrid, CategoryRail
```

- **No runtime dependencies beyond React.** Charts, graphing and any CAS are
  intentionally not included yet.
- **Hash routing** means the app is a single static bundle with no server
  rewrite rules.
- **Every calculation is client-side.** Nothing is transmitted anywhere.

## License

MIT — see [LICENSE](LICENSE).