import { validateTools, TOOLS } from '../src/registry'
import { CATEGORIES, CATEGORY_MAP } from '../src/registry/categories'
import { searchTools } from '../src/registry'
import { Parser } from '../src/lib/expr'
import {
  clampViewport,
  describeSweep,
  formatTick,
  findXIntercepts,
  findYIntercept,
  integrate,
  numericDerivative,
  paramProduct,
  parseParamDef,
  pan,
  sampleCartesian,
  sampleImplicit,
  sampleParametric,
  samplePolar,
  scale,
  shadeArea,
  ticks,
  usesParam,
  zoomAt,
  type ParamDef,
} from '../src/lib/graph'
import {
  buildSurface,
  clampPitch,  DEFAULT_CAMERA,
  heightColor,
  normalise,
  project,
  quadDepth,
  rotate,
  shade,
  sortByViewDepth,
} from '../src/lib/graph3d'

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
  const raw = (k: string) => (values[k] ?? graph.functions.find((f) => f.key === k)?.default ?? '').trim()
  const src = (k: string) => raw(k)

  const defs = graph.functions.map((f) => parseParamDef(raw(f.key))).filter((d): d is ParamDef => d !== undefined)
  const names = defs.map((d) => d.name)
  const combos = paramProduct(defs)
  const swept = new Set(
    graph.functions.filter((f) => parseParamDef(raw(f.key)) !== undefined).map((f) => f.key),
  )
  const variants = (k: string) => {
    const s = src(k)
    if (!names.some((n) => usesParam(s, n))) return [{}]
    return combos.filter((c) => names.every((n) => n in c))
  }

  let total = 0

  if (graph.mode === 'parametric') {
    for (const fn of graph.functions) {
      if (!fn.key.startsWith('x')) continue
      const partner = graph.functions.find((f) => f.key === `y${fn.key.slice(1)}`)
      if (!partner || swept.has(fn.key) || !src(fn.key) || !src(partner.key)) continue
      for (const vars of variants(fn.key)) {
        const fx = compileFor(src(fn.key))
        const fy = compileFor(src(partner.key))
        for (const seg of sampleParametric((t) => fx({ ...vars, t }), (t) => fy({ ...vars, t }), t0, t1, 600)) {
          total += seg.length
        }
      }
    }
  } else if (graph.mode === 'surface') {
    for (const fn of graph.functions) {
      if (swept.has(fn.key) || !src(fn.key)) continue
      for (const vars of variants(fn.key)) {
        const f = compileFor(src(fn.key))
        const mesh = buildSurface((x, y) => f({ ...vars, x, y }), { ...vp, resolution: 20 })
        total += mesh.quads.length
      }
    }
  } else {
    for (const fn of graph.functions) {
      if (swept.has(fn.key) || !src(fn.key)) continue
      for (const vars of variants(fn.key)) {
        const f = compileFor(src(fn.key))
        if (graph.mode === 'cartesian') {
          for (const seg of sampleCartesian((x) => f({ ...vars, x }), vp.x0, vp.x1, 600)) total += seg.length
        } else if (graph.mode === 'polar') {
          for (const seg of samplePolar((t) => f({ ...vars, t }), 1, 600)) total += seg.length
        } else {
          for (const seg of sampleImplicit((x, y) => f({ ...vars, x, y }), vp, 60)) total += seg.length
        }
      }
    }
  }
  return total / 2
}

report(chartTools.length === 5, `${chartTools.length} chart tools registered`)

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

console.log('\nparameter sweeps:')
let sFail = 0
const sreport = (ok: boolean, msg: string) => {
  if (!ok) sFail++
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${msg}`)
}

const range5 = parseParamDef('a = [1...5]')
sreport(range5?.name === 'a', 'parses "a = [1...5]"')
sreport(
  JSON.stringify(range5?.values) === JSON.stringify([1, 2, 3, 4, 5]),
  `integer range yields ${range5?.values.length} integer steps (${range5?.values.join(',')})`,
)

const list = parseParamDef('k = [0.5, 2, 10]')
sreport(
  JSON.stringify(list?.values) === JSON.stringify([0.5, 2, 10]),
  `explicit list yields ${list?.values.join(',')}`,
)

const fractional = parseParamDef('b = [0...1...0.1]')
sreport(
  (fractional?.values.length ?? 0) === 11 && Math.abs((fractional?.values.at(-1) ?? 0) - 1) < 1e-9,
  `explicit step "[0...1...0.1]" spans 0 to 1 in ${fractional?.values.length} steps`,
)
sreport(
  JSON.stringify(parseParamDef('b = [0...1]')?.values) === JSON.stringify([0, 1]),
  'integer endpoints default to a step of 1, so "[0...1]" is just the endpoints',
)
sreport(parseParamDef('b = [0...1...0]') === undefined, 'zero step is rejected')

const backwards = parseParamDef('n = [5...1]')
sreport(
  JSON.stringify(backwards?.values) === JSON.stringify([5, 4, 3, 2, 1]),
  'descending range counts down',
)

sreport(parseParamDef('sin(x)') === undefined, 'plain expression is not a parameter definition')
sreport(parseParamDef('a = []') === undefined, 'empty list is rejected')
sreport(parseParamDef('a = [1...1]') === undefined, 'zero-width range is rejected')
sreport(parseParamDef('9a = [1]') === undefined, 'identifier must not start with a digit')

sreport(usesParam('a*sin(x)', 'a'), 'detects parameter use')
sreport(!usesParam('sin(x)', 'a'), 'does not match a bare letter inside a function name')
sreport(!usesParam('max(x)', 'a'), 'does not match the "a" in max')
sreport(usesParam('(a)^2', 'a'), 'matches a parenthesised parameter')
sreport(!usesParam('factor(x)', 'a'), 'does not match a substring of a longer identifier')

const combos = paramProduct([
  { name: 'a', values: [1, 2] },
  { name: 'b', values: [10, 20] },
])
sreport(combos.length === 4, `cartesian product gives ${combos.length} combinations`)
sreport(
  combos.every((c) => typeof c.a === 'number' && typeof c.b === 'number'),
  'every combination binds both parameters',
)
sreport(paramProduct([]).length === 1, 'no parameters yields a single empty combination')
sreport(
  paramProduct([
    { name: 'a', values: [1, 2, 3] },
    { name: 'b', values: [1, 2, 3] },
    { name: 'c', values: [1, 2, 3] },
  ]).length === 27,
  'three parameters of three values give 27 combinations',
)
sreport(
  paramProduct([
    { name: 'a', values: [1, 2, 3, 4, 5, 6, 7, 8] },
    { name: 'b', values: [1, 2, 3, 4, 5, 6, 7, 8] },
    { name: 'c', values: [1, 2, 3, 4, 5, 6, 7, 8] },
  ]).length === 240,
  'product is capped to 240 to stop runaway sweeps',
)

const paramParser = new Parser('rad')
let paramOk = true
for (const v of [1, 2, 3]) {
  paramParser.setVars({ x: v, a: v })
  if (Math.abs(paramParser.evaluate('a*x') - v * v) > 1e-9) paramOk = false
}
sreport(paramOk, 'a parameter bound as a variable evaluates correctly across a sweep')

const shadowParser = new Parser('rad')
shadowParser.setVars({ x: 2, a: 5 })
const shadowOk = Math.abs(shadowParser.evaluate('atan(2)') - Math.atan(2)) < 1e-12
sreport(
  shadowOk,
  'a parameter named "a" cannot corrupt "atan(2)" — the value of a is ignored inside a function name',
)

const swept = paramProduct(parseParamDef('a = [1...5]') ? [parseParamDef('a = [1...5]')!] : [])
const sweepSeries = swept.map((combo) => {
  const f = compileFor('a*x')
  return sampleCartesian((x) => f({ ...combo, x }), 0, 2, 50)
})
sreport(
  sweepSeries.length === 5 && sweepSeries.every((seg) => seg.length > 0),
  `sweeping a=[1...5] yields ${sweepSeries.length} distinct non-empty curves`,
)
sreport(
  describeSweep([{ name: 'a', values: [1, 2, 3, 4, 5] }]).includes('a = 1…5 (5)'),
  `describeSweep reads "${describeSweep([{ name: 'a', values: [1, 2, 3, 4, 5] }])}"`,
)

console.log('\nderivative and integral:')
let aFail = 0
const areport = (ok: boolean, msg: string) => {
  if (!ok) aFail++
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${msg}`)
}
const closeTo = (got: number | undefined, want: number, tol = 1e-6) =>
  got !== undefined && Math.abs(got - want) <= tol

const d = numericDerivative((x) => Math.sin(x), 0, Math.PI * 2, 400)
const dMid = d.length === 1 ? d[0]! : []
areport(d.length === 1 && dMid.length > 400, 'derivative of sin(x) is one continuous curve')
let maxDev = 0
for (let i = 0; i + 1 < dMid.length; i += 2) {
  const x = dMid[i] as number
  const got = dMid[i + 1] as number
  maxDev = Math.max(maxDev, Math.abs(got - Math.cos(x)))
}
areport(maxDev < 1e-5, `numeric d/dx sin(x) tracks cos(x) to ${maxDev.toExponential(1)}`)

const quad = numericDerivative((x) => x * x, 0, 5, 200)
const quadMid = quad[0] ?? []
const quadOk =
  quadMid.length > 100 &&
  Array.from({ length: Math.floor(quadMid.length / 2) }, (_, i) => {
    const x = quadMid[i * 2] as number
    const v = quadMid[i * 2 + 1] as number
    return Math.abs(v - 2 * x) < 1e-6
  }).every(Boolean)
areport(quadOk, 'numeric d/dx x² returns 2x at every sample')

const poleDeriv = numericDerivative((x) => 1 / x, -2, 2, 300)
areport(poleDeriv.length >= 2, `d/dx (1/x) splits at the pole into ${poleDeriv.length} segments`)

areport(closeTo(integrate((x) => x * x, 0, 3), 9), '∫₀³ x² dx = 9')
areport(closeTo(integrate(Math.sin, 0, Math.PI), 2), '∫₀^π sin(x) dx = 2')
areport(closeTo(integrate((x) => 1, -5, 5), 10), '∫₋₅⁵ 1 dx = 10')
areport(closeTo(integrate((x) => Math.exp(x), 0, 1), Math.E - 1), '∫₀¹ eˣ dx = e − 1')
areport(closeTo(integrate((x) => 1 / (1 + x * x), 0, 1), Math.PI / 4), '∫₀¹ 1/(1+x²) dx = π/4')
areport(closeTo(integrate((x) => x * x, 3, 0), -9), 'reversed bounds flip the sign')
areport(integrate((x) => x, 1, 1) === undefined, 'zero-width interval is rejected')
areport(integrate((x) => 1 / (x - 0.5), 0, 1) === undefined, 'integral across a pole is rejected')
areport(integrate((x) => Math.sqrt(x - 2), 0, 4) === undefined, 'integral over a negative square root is rejected')

const sineIntegral = Math.abs((integrate(Math.sin, 0, Math.PI) as number) - 2)
areport(sineIntegral < 1e-9, `adaptive Simpson is accurate to ${sineIntegral.toExponential(1)}`)

const shadePath = shadeArea((x) => x * x, 0, 1, 0, 50)
areport(shadePath !== undefined && shadePath.length === 106, 'shadeArea returns 51 curve points plus 2 baseline corners')
areport(shadePath !== undefined && shadePath[0] === 0 && closeTo(shadePath[1] as number, 0, 1e-9), 'shade starts on the curve at the lower bound')
areport(
  shadePath !== undefined &&
    closeTo(shadePath[shadePath.length - 4] as number, 1, 1e-9) &&
    (shadePath[shadePath.length - 3] as number) === 0 &&
    (shadePath[shadePath.length - 2] as number) === 0 &&
    (shadePath[shadePath.length - 1] as number) === 0,
  'shade returns to the upper bound then runs back along the baseline',
)
areport(shadeArea((x) => 1 / (x - 0.5), 0, 1, 0, 50) === undefined, 'shading across a pole is rejected')
areport(shadeArea((x) => x, 2, 2, 0, 50) === undefined, 'shading a zero-width region is rejected')

console.log('\nintercepts:')
let iFail = 0
const ireport = (ok: boolean, msg: string) => {
  if (!ok) iFail++
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${msg}`)
}
const near = (v: number | undefined, want: number, tol = 1e-6) =>
  v !== undefined && Math.abs(v - want) <= tol
const hasNear = (roots: number[], want: number, tol = 1e-4) => roots.some((r) => Math.abs(r - want) <= tol)

const quadRoots = findXIntercepts((x) => x * x - 4, -5, 5, 2000)
ireport(quadRoots.length === 2, `x² − 4 has ${quadRoots.length} roots`)
ireport(hasNear(quadRoots, -2) && hasNear(quadRoots, 2), 'roots land on ±2')

const cubicRoots = findXIntercepts((x) => x * x * x - x, -2, 2, 2000)
ireport(cubicRoots.length === 3, `x³ − x has ${cubicRoots.length} roots`)
ireport(
  hasNear(cubicRoots, -1) && hasNear(cubicRoots, 0) && hasNear(cubicRoots, 1),
  'roots land on −1, 0 and 1',
)

const sinRoots = findXIntercepts((x) => Math.sin(x), -0.5, 3.5, 2000)
ireport(hasNear(sinRoots, 0) && hasNear(sinRoots, Math.PI), 'sin(x) roots land on 0 and π')

ireport(findXIntercepts((x) => x * x, -1, 1, 2000).length === 1, 'x² still reports its root at 0, which is sampled exactly')
ireport(
  findXIntercepts((x) => (x - 0.37) * (x - 0.37), -1, 1, 2000).length === 0,
  'a tangent root away from a sample point is missed — only sign changes and exact zeros are found',
)
ireport(findXIntercepts((x) => 1 / x, -2, 2, 2000).length === 0, 'a pole is not mistaken for a root')
ireport(findXIntercepts((x) => x + 5, -1, 1, 2000).length === 0, 'a function with no root reports none')
ireport(findXIntercepts((x) => x + 5, -1, 1, 2000).every((r) => r >= -1 && r <= 1), 'roots stay inside the scanned window')

ireport(near(findYIntercept((x) => 3 * x + 2, -5, 5), 2), 'y-intercept of 3x + 2 is 2')
ireport(findYIntercept((x) => 3 * x + 2, 1, 5) === undefined, 'y-intercept is undefined when x = 0 is off screen')
ireport(findYIntercept((x) => 1 / x, -5, 5) === undefined, 'y-intercept of 1/x is undefined')

console.log('\n3d surface:')
let tFail = 0
const treport = (ok: boolean, msg: string) => {
  if (!ok) tFail++
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${msg}`)
}

const rot = rotate({ x: 1, y: 0, z: 0 }, Math.PI / 2, 0)
treport(Math.abs(rot.x) < 1e-12 && Math.abs(rot.z + 1) < 1e-12, 'a quarter turn about y maps +x onto −z')

const flat = rotate({ x: 1, y: 2, z: 3 }, 0, 0)
treport(flat.x === 1 && flat.y === 2 && flat.z === 3, 'a zero rotation is the identity')

const pr = project({ x: 0, y: 0, z: 0 }, DEFAULT_CAMERA, 400, 300)
treport(Math.abs(pr.x - 200) < 1e-9 && Math.abs(pr.y - 150) < 1e-9, 'the origin projects to the centre of the canvas')
const right = project({ x: 1, y: 0, z: 0 }, { ...DEFAULT_CAMERA, yaw: 0, pitch: 0 }, 400, 300)
treport(right.x > 200 && Math.abs(right.y - 150) < 1e-9, 'with no rotation +x is to the right of the origin')
const zoomed = project({ x: 1, y: 0, z: 0 }, { ...DEFAULT_CAMERA, zoom: 2, yaw: 0, pitch: 0 }, 400, 300)
treport(Math.abs(zoomed.x - right.x) > 1, 'doubling the zoom moves the point further out')

treport(clampPitch(9) === 1.5 && clampPitch(-9) === -1.5, 'pitch is clamped so the surface cannot flip over')

const plane = buildSurface((x, y) => 3, { x0: -1, x1: 1, y0: -1, y1: 1, resolution: 4 })
treport(plane.quads.length === 16, `a 4x4 grid yields ${plane.quads.length} quads`)
treport(plane.zMin === 3 && plane.zMax === 3, 'a constant surface reports a zero height range')
treport(
  plane.quads.every((q) => Math.abs(q.nz - 1) < 1e-6),
  'the normal of a flat surface points straight up',
)

const bowl = buildSurface((x, y) => x * x + y * y, { x0: -2, x1: 2, y0: -2, y1: 2, resolution: 8 })
treport(bowl.zMin === 0 && bowl.zMax === 8, 'the bowl spans z from 0 to 8')

const cam = { ...DEFAULT_CAMERA }
const proj = bowl.points.map((p) => project(p, cam, 400, 300))
const sorted = sortByViewDepth(bowl.quads, proj)
const depths = sorted.map((q) => quadDepth(q, proj))
treport(
  depths.every((d, i) => i === 0 || d >= (depths[i - 1] as number)),
  'quads sort back to front by view depth for the painter algorithm',
)
const worldOrder = bowl.quads.map((q) => Object.values(q.corners).join(','))
const viewOrder = sorted.map((q) => Object.values(q.corners).join(','))
treport(
  worldOrder.join('|') !== viewOrder.join('|'),
  'view depth is not the same as world z, so sorting by world space would be wrong',
)
treport(bowl.maxRadius > 0, `the mesh reports a bounding radius of ${bowl.maxRadius.toFixed(2)}`)

const segmentsCross = (p1: any, p2: any, p3: any, p4: any) => {
  const den = (p2.x - p1.x) * (p4.y - p3.y) - (p2.y - p1.y) * (p4.x - p3.x)
  if (Math.abs(den) < 1e-12) return false
  const t = ((p3.x - p1.x) * (p4.y - p3.y) - (p3.y - p1.y) * (p4.x - p3.x)) / den
  const u = ((p3.x - p1.x) * (p2.y - p1.y) - (p3.y - p1.y) * (p2.x - p1.x)) / den
  return t > 0 && t < 1 && u > 0 && u < 1
}
let bowties = 0
for (const quad of bowl.quads) {
  const { a: ia, b: ib, c: ic, d: id } = quad.corners
  const pa = proj[ia]!
  const pb = proj[ib]!
  const pc = proj[ic]!
  const pd = proj[id]!
  if (segmentsCross(pa, pb, pd, pc) || segmentsCross(pb, pd, pc, pa)) bowties++
}
treport(bowties === 0, `no projected quad is a self-intersecting bowtie (${bowties} found)`)

const saddle = buildSurface((x, y) => Math.sin(x) * Math.cos(y) + 0.25 * (x * x - y * y), {
  x0: -5, x1: 5, y0: -5, y1: 5, resolution: 40,
})
const saddleProj = saddle.points.map((p) => project(p, DEFAULT_CAMERA, 1114, 504))
let saddleBowties = 0
for (const quad of saddle.quads) {
  const { a: ia, b: ib, c: ic, d: id } = quad.corners
  const pa = saddleProj[ia]!
  const pb = saddleProj[ib]!
  const pc = saddleProj[ic]!
  const pd = saddleProj[id]!
  if (segmentsCross(pa, pb, pd, pc) || segmentsCross(pb, pd, pc, pa)) saddleBowties++
}
treport(saddleBowties === 0, `the saddle surface has no bowtie quads either (${saddleBowties} found)`)

const holey = buildSurface((x, y) => (Math.abs(x) < 0.25 ? NaN : x), { x0: -1, x1: 1, y0: -1, y1: 1, resolution: 8 })
treport(holey.quads.length < 64, `quads touching a NaN are dropped (${holey.quads.length} of 64)`)

treport(normalise(5, 0, 10) === 0.5, 'normalise maps the midpoint to 0.5')
treport(normalise(5, 5, 5) === 0.5, 'a zero range normalises to 0.5 rather than dividing by zero')

const c0 = heightColor(0)
const c1 = heightColor(1)
treport(c0[0] !== c1[0] || c0[1] !== c1[1] || c0[2] !== c1[2], 'the colour ramp changes across the range')
treport(
  heightColor(-1)[0] === heightColor(0)[0] && heightColor(2)[0] === heightColor(1)[0],
  'the colour ramp clamps out-of-range values',
)
treport(shade({ nx: 0, ny: 0, nz: 1 }) > 0.5, 'a surface facing the light is brighter than one facing away')
treport(shade({ nx: 0, ny: 0, nz: -1 }) < shade({ nx: 0, ny: 0, nz: 1 }), 'the underside of the surface is darker')

console.log('\nsearch:')
for (const q of ['ohm', 'beam', 'pH', 'regression', 'hex']) {
  const hits = searchTools(q)
  console.log(`  "${q}" -> ${hits.length} hits, top: ${hits[0]?.tool.name ?? 'none'}`)
}

void CATEGORY_MAP
process.exit(problems.length + exprFail + gFail + sFail + aFail + iFail + tFail ? 1 : 0)