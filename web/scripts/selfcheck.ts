import { validateTools, TOOLS } from '../src/registry'
import { CATEGORIES, CATEGORY_MAP } from '../src/registry/categories'
import { searchTools } from '../src/registry'
import { Parser } from '../src/lib/expr'
import {
  clampViewport,
  formatTick,
  pan,
  sampleCartesian,
  sampleImplicit,
  sampleParametric,
  samplePolar,
  scale,
  ticks,
  zoomAt,
} from '../src/lib/graph'

const problems = validateTools(TOOLS)
console.log(`tools registered: ${TOOLS.length}`)
console.log(`categories used: ${new Set(TOOLS.map((t) => t.category)).size} / ${CATEGORIES.length}`)
for (const c of CATEGORIES) {
  const n = TOOLS.filter((t) => t.category === c.id).length
  console.log(`  ${c.id.padEnd(12)} ${n}`)
}
if (problems.length) {
  console.log(`\nVALIDATION PROBLEMS (${problems.length}):`)
  for (const p of problems) console.log('  ! ' + p)
} else {
  console.log('\nregistry validation: PASS')
}

const p = new Parser('rad')
const exprTests: [string, number][] = [
  ['2+3*4', 14],
  ['(2+3)*4', 20],
  ['2^3^2', 512],
  ['-2^2', -4],
  ['2^-1', 0.5],
  ['10%3', 1],
  ['3!', 6],
  ['sqrt(16)', 4],
  ['2(3+4)', 14],
  ['pi', Math.PI],
  ['gcd(12,18)', 6],
  ['lcm(4,6)', 12],
  ['1/3', 1 / 3],
]
let exprFail = 0
console.log('\nexpression engine:')
for (const [src, want] of exprTests) {
  const got = p.evaluate(src)
  const ok = Math.abs(got - want) < 1e-9
  if (!ok) exprFail++
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${src} = ${got}${ok ? '' : ` (want ${want})`}`)
}
const deg = new Parser('deg')
const sinDeg = deg.evaluate('sin(30)')
console.log(`  ${Math.abs(sinDeg - 0.5) < 1e-9 ? 'ok  ' : 'FAIL'} sin(30) in degrees = ${sinDeg}`)

console.log('\ngraph maths:')
let gFail = 0
const approx = (got: number, want: number, tol = 1e-6) => Math.abs(got - want) <= tol
const report = (ok: boolean, msg: string) => {
  if (!ok) gFail++
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${msg}`)
}

const VP = { x0: -10, x1: 10, y0: -6, y1: 6 }
const W = 400
const H = 300
const s = scale(VP, W, H)
report(approx(s.x(0), W / 2), 'scale maps x=0 to horizontal centre')
report(approx(s.y(0), H / 2), 'scale maps y=0 to vertical centre')
report(approx(s.wx(W), 10, 1e-9), 'screenToWorld inverts at right edge')

const z = zoomAt(VP, W, H, W / 2, H / 2, 0.5)
report(approx(z.x1 - z.x0, 10, 1e-9), 'zoom 0.5 halves the x span')
report(approx(z.x0, -5, 1e-9) && approx(z.x1, 5, 1e-9), 'zoom about centre keeps the anchor fixed')

const panned = pan(VP, W, H, 40, 0)
report(approx(panned.x1 - panned.x0, 20, 1e-9), 'pan preserves span')
report(panned.x0 < VP.x0, 'pan by +40px moves the view left')

const degenerate = clampViewport({ x0: 5, x1: 5, y0: 0, y1: 0 })
report(degenerate.x1 > degenerate.x0 && degenerate.y1 > degenerate.y0, 'clampViewport fixes a zero-span viewport')
const nanVp = clampViewport({ x0: NaN, x1: NaN, y0: NaN, y1: NaN })
report(Number.isFinite(nanVp.x0) && Number.isFinite(nanVp.y0), 'clampViewport recovers from NaN')

const t = ticks(-10, 10, 10)
report(t.length >= 9 && t.length <= 12, `ticks(-10,10) gives ${t.length} marks`)
report(t.includes(0) && t.includes(-10) && t.includes(10), 'ticks includes 0 and both endpoints')
report(t.every((v, i) => i === 0 || v > (t[i - 1] as number)), 'ticks are strictly increasing')
const coarse = ticks(0, 1000000, 8)
report(coarse.length >= 8 && coarse.length <= 14, `ticks(0,1e6) gives ${coarse.length} marks`)
report(coarse[0] === 0 && coarse[coarse.length - 1] === 1000000, 'coarse ticks span both ends of the range')
report(formatTick(0) === '0' && formatTick(1500) === '1500', 'formatTick trims trailing zeros')

const line = sampleCartesian((x) => 2 * x, 0, 10, 100)
report(line.length === 1 && line[0]!.length === 202, 'cartesian line yields one unbroken polyline')

const pole = sampleCartesian((x) => 1 / x, -2, 2, 200)
report(pole.length === 2, `1/x splits at the asymptote into ${pole.length} segments`)

const boom = sampleCartesian((x) => 1 / (x * x), -1, 1, 200)
report(boom.length === 2, '1/x² splits either side of the pole')

const circleTop = sampleCartesian((x) => Math.sqrt(Math.max(4 - x * x, 0)), -2, 2, 200)
report(circleTop.length >= 1, 'sqrt(4 - x²) plots as a connected arc')

const polar = samplePolar((t) => 1, 1, 360)
report(polar.length === 1 && polar[0]!.length === 722, 'polar r=1 traces a full circle')

const para = sampleParametric((t) => Math.cos(t), (t) => Math.sin(t), 0, Math.PI * 2, 360)
report(para.length === 1, 'parametric circle is continuous')

const implicit = sampleImplicit((x, y) => x * x + y * y - 4, VP, 80)
report(implicit.length > 20, `implicit circle yields ${implicit.length} segments`)
const segs = implicit.filter((s2) => s2.length === 4)
report(
  segs.every((s2) => {
    const [x1, y1, x2, y2] = s2
    return Math.abs(Math.hypot(x1 as number, y1 as number) - 2) < 0.05 && Math.abs(Math.hypot(x2 as number, y2 as number) - 2) < 0.05
  }),
  'every implicit segment endpoint lies on the circle of radius 2',
)

const empty = sampleImplicit(() => NaN, VP, 20)
report(empty.length === 0, 'implicit with no sign change produces no segments')

console.log('\ngraph tools end-to-end:')

const chartTools = TOOLS.filter((t): t is Extract<typeof t, { kind: 'chart' }> => t.kind === 'chart')

const compileFor = (src: string) => {
  const parser = new Parser('rad')
  return (vars: Record<string, number>) => {
    try {
      parser.setVars(vars)
      return parser.evaluate(src)
    } catch {
      return NaN
    }
  }
}

const runChart = (tool: (typeof chartTools)[number], values: Record<string, string>) => {
  const { graph } = tool
  const vp = graph.viewport ?? { x0: -10, x1: 10, y0: -6, y1: 6 }
  const [t0, t1] = graph.tRange ?? [0, Math.PI * 2]
  const src = (k: string) => (values[k] ?? graph.functions.find((f) => f.key === k)?.default ?? '').trim()
  let total = 0

  if (graph.mode === 'parametric') {
    for (const fn of graph.functions) {
      if (!fn.key.startsWith('x')) continue
      const partner = graph.functions.find((f) => f.key === `y${fn.key.slice(1)}`)
      if (!partner || !src(fn.key) || !src(partner.key)) continue
      const fx = compileFor(src(fn.key))
      const fy = compileFor(src(partner.key))
      for (const seg of sampleParametric((t) => fx({ t }), (t) => fy({ t }), t0, t1, 600)) total += seg.length
    }
  } else {
    for (const fn of graph.functions) {
      const s = src(fn.key)
      if (!s) continue
      const f = compileFor(s)
      if (graph.mode === 'cartesian') {
        for (const seg of sampleCartesian((x) => f({ x }), vp.x0, vp.x1, 600)) total += seg.length
      } else if (graph.mode === 'polar') {
        for (const seg of samplePolar((t) => f({ t }), 1, 600)) total += seg.length
      } else {
        for (const seg of sampleImplicit((x, y) => f({ x, y }), vp, 60)) total += seg.length
      }
    }
  }
  return total / 2
}

report(chartTools.length === 4, `${chartTools.length} chart tools registered`)

for (const tool of chartTools) {
  const n = runChart(tool, {})
  report(n > 10, `${tool.name} (${tool.graph.mode}) defaults produce ${Math.round(n)} points`)
  for (const ex of tool.examples ?? []) {
    const m = runChart(tool, ex.inputs)
    report(m > 10, `${tool.name} example "${ex.label}" produces ${Math.round(m)} points`)
  }
}

report(
  searchTools('graph').length + searchTools('plot').length + searchTools('curve').length > 0,
  'graph tools are reachable from search',
)

console.log('\nsearch:')
for (const q of ['ohm', 'beam', 'pH', 'regression', 'hex']) {
  const hits = searchTools(q)
  console.log(`  "${q}" -> ${hits.length} hits, top: ${hits[0]?.tool.name ?? 'none'}`)
}

void CATEGORY_MAP
process.exit(problems.length + exprFail + gFail ? 1 : 0)