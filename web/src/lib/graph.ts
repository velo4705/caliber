export type Series = number[][]

export interface Viewport {
  x0: number
  x1: number
  y0: number
  y1: number
}

export interface Point {
  x: number
  y: number
}

export const DEFAULT_VP: Viewport = { x0: -10, x1: 10, y0: -6, y1: 6 }

export function isParamSlot(src: string): boolean {
  return parseParamDef(src) !== undefined
}

export function clampViewport(vp: Viewport): Viewport {
  const minSpan = 1e-9
  let { x0, x1, y0, y1 } = vp
  if (!Number.isFinite(x0) || !Number.isFinite(x1)) {
    x0 = DEFAULT_VP.x0
    x1 = DEFAULT_VP.x1
  }
  if (!Number.isFinite(y0) || !Number.isFinite(y1)) {
    y0 = DEFAULT_VP.y0
    y1 = DEFAULT_VP.y1
  }
  if (x1 - x0 < minSpan) {
    const c = (x0 + x1) / 2
    x0 = c - minSpan
    x1 = c + minSpan
  }
  if (y1 - y0 < minSpan) {
    const c = (y0 + y1) / 2
    y0 = c - minSpan
    y1 = c + minSpan
  }
  return { x0, x1, y0, y1 }
}

export function scale(vp: Viewport, width: number, height: number) {
  const sx = width / (vp.x1 - vp.x0)
  const sy = height / (vp.y1 - vp.y0)
  return {
    x: (wx: number) => (wx - vp.x0) * sx,
    y: (wy: number) => height - (wy - vp.y0) * sy,
    wx: (px: number) => vp.x0 + px / sx,
    wy: (py: number) => vp.y0 + (height - py) / sy,
  }
}

export function zoomAt(vp: Viewport, width: number, height: number, px: number, py: number, factor: number): Viewport {
  const s = scale(vp, width, height)
  const anchorX = s.wx(px)
  const anchorY = s.wy(py)
  return clampViewport({
    x0: anchorX - (anchorX - vp.x0) * factor,
    x1: anchorX + (vp.x1 - anchorX) * factor,
    y0: anchorY - (anchorY - vp.y0) * factor,
    y1: anchorY + (vp.y1 - anchorY) * factor,
  })
}

export function pan(vp: Viewport, width: number, height: number, dxPx: number, dyPx: number): Viewport {
  const dx = dxPx / (width / (vp.x1 - vp.x0))
  const dy = dyPx / (height / (vp.y1 - vp.y0))
  return clampViewport({ x0: vp.x0 - dx, x1: vp.x1 - dx, y0: vp.y0 + dy, y1: vp.y1 + dy })
}

function niceNum(range: number, round: boolean): number {
  if (range <= 0 || !Number.isFinite(range)) return 1
  const exp = Math.floor(Math.log10(range))
  const f = range / Math.pow(10, exp)
  let nf: number
  if (round) nf = f < 1.5 ? 1 : f < 3 ? 2 : f < 7 ? 5 : 10
  else nf = f <= 1 ? 1 : f <= 2 ? 2 : f <= 5 ? 5 : 10
  return nf * Math.pow(10, exp)
}

export function ticks(lo: number, hi: number, target = 10): number[] {
  if (!Number.isFinite(lo) || !Number.isFinite(hi) || hi <= lo) return []
  const step = niceNum(niceNum(hi - lo, false) / Math.max(target - 1, 1), true)
  if (!Number.isFinite(step) || step <= 0) return []
  const out: number[] = []
  const start = Math.ceil(lo / step) * step
  for (let v = start, guard = 0; v <= hi + step * 1e-6 && guard < 200; v += step, guard++) {
    out.push(Math.abs(v) < step * 1e-9 ? 0 : v)
  }
  return out
}

export function formatTick(v: number): string {
  if (v === 0) return '0'
  const abs = Math.abs(v)
  if (abs >= 1e5 || abs < 1e-4) return v.toExponential(1)
  const decimals = Math.max(0, Math.min(6, -Math.floor(Math.log10(abs))))
  return v.toFixed(decimals)
}

export function formatRoot(v: number): string {
  if (!Number.isFinite(v)) return '—'
  if (v === 0) return '0'
  const abs = Math.abs(v)
  if (abs >= 1e6 || abs < 1e-4) return v.toExponential(2)
  return Number(v.toPrecision(5)).toString()
}

const Y_LIMIT = 1e9

function pushPoint(current: number[], x: number, y: number) {
  if (y > Y_LIMIT || y < -Y_LIMIT) return false
  current.push(x, y)
  return true
}

export function sampleCartesian(
  evaluate: (x: number) => number,
  x0: number,
  x1: number,
  points = 1200,
  yWindow?: { lo: number; hi: number },
): Series {
  const out: Series = []
  let current: number[] = []
  const span = x1 - x0
  for (let i = 0; i <= points; i++) {
    const x = x0 + (span * i) / points
    const y = evaluate(x)
    if (!Number.isFinite(y)) {
      if (current.length >= 4) out.push(current)
      current = []
      continue
    }
    if (yWindow && (y > yWindow.hi || y < yWindow.lo)) {
      if (current.length >= 4) out.push(current)
      current = []
      continue
    }
    pushPoint(current, x, y)
  }
  if (current.length >= 4) out.push(current)
  return out
}

export function samplePolar(evaluate: (theta: number) => number, turns = 1, points = 1200): Series {
  const out: Series = []
  let current: number[] = []
  const end = Math.PI * 2 * turns
  for (let i = 0; i <= points; i++) {
    const t = (end * i) / points
    const r = evaluate(t)
    if (!Number.isFinite(r)) {
      if (current.length >= 4) out.push(current)
      current = []
      continue
    }
    const x = r * Math.cos(t)
    const y = r * Math.sin(t)
    pushPoint(current, x, y)
  }
  if (current.length >= 4) out.push(current)
  return out
}

export function sampleParametric(
  fx: (t: number) => number,
  fy: (t: number) => number,
  t0: number,
  t1: number,
  points = 1200,
): Series {
  const out: Series = []
  let current: number[] = []
  for (let i = 0; i <= points; i++) {
    const t = t0 + ((t1 - t0) * i) / points
    const x = fx(t)
    const y = fy(t)
    if (!Number.isFinite(x) || !Number.isFinite(y)) {
      if (current.length >= 4) out.push(current)
      current = []
      continue
    }
    current.push(x, y)
  }
  if (current.length >= 4) out.push(current)
  return out
}

export function sampleImplicit(
  evaluate: (x: number, y: number) => number,
  vp: Viewport,
  resolution = 90,
): Series {
  const out: Series = []
  const cols = resolution
  const rows = resolution
  const dx = (vp.x1 - vp.x0) / cols
  const dy = (vp.y1 - vp.y0) / rows
  const grid: number[] = new Array((cols + 1) * (rows + 1))

  for (let r = 0; r <= rows; r++) {
    const y = vp.y0 + r * dy
    for (let c = 0; c <= cols; c++) {
      grid[r * (cols + 1) + c] = evaluate(vp.x0 + c * dx, y)
    }
  }

  const lerp = (a: number, b: number, va: number, vb: number) => {
    const d = vb - va
    if (Math.abs(d) < 1e-12) return a
    return a + ((b - a) * -va) / d
  }

  for (let r = 0; r < rows; r++) {
    for (let c = 0; c < cols; c++) {
      const x0 = vp.x0 + c * dx
      const x1 = x0 + dx
      const y0 = vp.y0 + r * dy
      const y1 = y0 + dy
      const v00 = grid[r * (cols + 1) + c] as number
      const v10 = grid[r * (cols + 1) + c + 1] as number
      const v01 = grid[(r + 1) * (cols + 1) + c] as number
      const v11 = grid[(r + 1) * (cols + 1) + c + 1] as number
      if (![v00, v10, v01, v11].every(Number.isFinite)) continue

      const hits: Point[] = []
      if (v00 * v10 < 0) hits.push({ x: lerp(x0, x1, v00, v10), y: y0 })
      if (v10 * v11 < 0) hits.push({ x: x1, y: lerp(y0, y1, v10, v11) })
      if (v11 * v01 < 0) hits.push({ x: lerp(x1, x0, v11, v01), y: y1 })
      if (v01 * v00 < 0) hits.push({ x: x0, y: lerp(y1, y0, v01, v00) })

      if (hits.length >= 2) {
        const seg: number[] = []
        pushPoint(seg, hits[0]!.x, hits[0]!.y)
        pushPoint(seg, hits[1]!.x, hits[1]!.y)
        if (seg.length === 4) out.push(seg)
      }
    }
  }
  return out
}

export function numericDerivative(
  evaluate: (x: number) => number,
  x0: number,
  x1: number,
  points = 1200,
): Series {
  const out: Series = []
  let current: number[] = []
  const span = x1 - x0
  for (let i = 0; i <= points; i++) {
    const x = x0 + (span * i) / points
    const h = Math.max(Math.abs(x), 1) * 1e-5
    const d = (evaluate(x + h) - evaluate(x - h)) / (2 * h)
    if (!Number.isFinite(d) || d > Y_LIMIT || d < -Y_LIMIT) {
      if (current.length >= 4) out.push(current)
      current = []
      continue
    }
    current.push(x, d)
  }
  if (current.length >= 4) out.push(current)
  return out
}

function bisect(f: (x: number) => number, a: number, b: number): number | undefined {
  let lo = a
  let hi = b
  const flo = f(lo)
  if (!Number.isFinite(flo)) return undefined
  for (let i = 0; i < 60; i++) {
    const mid = (lo + hi) / 2
    const fm = f(mid)
    if (!Number.isFinite(fm)) return undefined
    if (fm === 0) return mid
    if (Math.sign(fm) === Math.sign(flo)) lo = mid
    else hi = mid
  }
  return (lo + hi) / 2
}

export function findXIntercepts(
  evaluate: (x: number) => number,
  x0: number,
  x1: number,
  points = 1200,
): number[] {
  const roots: number[] = []
  const span = x1 - x0
  if (span === 0 || !Number.isFinite(span)) return roots
  const step = span / points

  let prevX = x0
  let prevY = evaluate(x0)
  if (Number.isFinite(prevY) && prevY === 0) roots.push(x0)

  for (let i = 1; i <= points; i++) {
    const x = x0 + step * i
    const y = evaluate(x)
    if (!Number.isFinite(y)) {
      prevX = x
      prevY = y
      continue
    }
    if (y === 0) {
      roots.push(x)
    } else if (Number.isFinite(prevY) && prevY !== 0 && Math.sign(prevY) !== Math.sign(y)) {
      const root = bisect(evaluate, prevX, x)
      if (root !== undefined) roots.push(root)
    }
    prevX = x
    prevY = y
  }

  const merged: number[] = []
  for (const r of roots.sort((a, b) => a - b)) {
    const last = merged[merged.length - 1]
    if (last === undefined || Math.abs(r - last) > Math.abs(step) * 1.5) merged.push(r)
  }
  return merged
}

export function findYIntercept(
  evaluate: (x: number) => number,
  x0: number,
  x1: number,
): number | undefined {
  if (x0 > 0 || x1 < 0) return undefined
  const y = evaluate(0)
  return Number.isFinite(y) ? y : undefined
}

export function integrate(
  evaluate: (x: number) => number,
  a: number,
  b: number,
  tolerance = 1e-9,
): number | undefined {
  if (!Number.isFinite(a) || !Number.isFinite(b) || a === b) return undefined
  const lo = Math.min(a, b)
  const hi = Math.max(a, b)
  const sign = a <= b ? 1 : -1

  for (let i = 0; i <= 24; i++) {
    if (!Number.isFinite(evaluate(lo + ((hi - lo) * i) / 24))) return undefined
  }

  const simpson = (l: number, h: number): number => {
    const m = (l + h) / 2
    return ((h - l) / 6) * (evaluate(l) + 4 * evaluate(m) + evaluate(h))
  }

  const refine = (l: number, h: number, whole: number, tol: number, depth: number): number => {
    const m = (l + h) / 2
    const left = simpson(l, m)
    const right = simpson(m, h)
    const delta = left + right - whole
    if (depth <= 0 || Math.abs(delta) <= 15 * tol) return left + right + delta / 15
    return refine(l, m, left, tol / 2, depth - 1) + refine(m, h, right, tol / 2, depth - 1)
  }

  const result = refine(lo, hi, simpson(lo, hi), tolerance, 20)
  if (!Number.isFinite(result)) return undefined
  return result * sign
}

export function shadeArea(
  evaluate: (x: number) => number,
  a: number,
  b: number,
  baseline = 0,
  points = 400,
): number[] | undefined {
  if (!Number.isFinite(a) || !Number.isFinite(b) || a === b) return undefined
  const lo = Math.min(a, b)
  const hi = Math.max(a, b)
  const path: number[] = []
  const span = hi - lo
  for (let i = 0; i <= points; i++) {
    const x = lo + (span * i) / points
    const y = evaluate(x)
    if (!Number.isFinite(y) || y > Y_LIMIT || y < -Y_LIMIT) return undefined
    path.push(x, y)
  }
  path.push(hi, baseline, lo, baseline)
  return path
}

export function escapeRegExp(s: string): string {
  return escapeRe(s)
}

export interface TraceHit {
  index: number
  x: number
  y: number
  distance: number
}

function escapeRe(s: string): string {
  return s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')
}

export interface ParamDef {
  name: string
  values: number[]
}

const PARAM_DEF = /^\s*([a-zA-Z_][a-zA-Z0-9_]*)\s*=\s*\[([^\]]*)\]\s*$/

export function parseParamDef(src: string): ParamDef | undefined {
  const m = PARAM_DEF.exec(src)
  if (!m) return undefined
  const name = m[1] as string
  const body = (m[2] as string).trim()
  if (!body) return undefined

  if (body.includes('...')) {
    const parts = body.split('...')
    if (parts.length < 2 || parts.length > 3) return undefined
    const lo = Number((parts[0] as string).trim())
    const hi = Number((parts[1] as string).trim())
    if (!Number.isFinite(lo) || !Number.isFinite(hi) || lo === hi) return undefined
    let step: number
    if (parts.length === 3) {
      step = Math.abs(Number((parts[2] as string).trim()))
      if (!Number.isFinite(step) || step === 0) return undefined
    } else {
      const magnitude = Math.abs(hi - lo)
      step = Number.isInteger(lo) && Number.isInteger(hi) ? 1 : magnitude / Math.min(magnitude, 24)
    }
    if (hi < lo) step = -step
    const count = Math.min(Math.floor(Math.abs((hi - lo) / step)) + 1, 200)
    const values: number[] = []
    for (let i = 0; i < count; i++) values.push(Number((lo + i * step).toPrecision(12)))
    return { name, values }
  }

  const values = body
    .split(',')
    .map((s) => Number(s.trim()))
    .filter((n) => Number.isFinite(n))
  return values.length ? { name, values } : undefined
}

export function usesParam(src: string, name: string): boolean {
  if (!name) return false
  try {
    return new RegExp(`(?<![a-zA-Z0-9_])${escapeRe(name)}(?![a-zA-Z0-9_])`).test(src)
  } catch {
    return false
  }
}

export function paramProduct(defs: ParamDef[], limit = 240): Record<string, number>[] {
  let combos: Record<string, number>[] = [{}]
  for (const def of defs) {
    const next: Record<string, number>[] = []
    for (const combo of combos) {
      for (const v of def.values) next.push({ ...combo, [def.name]: v })
    }
    combos = next
    if (combos.length >= limit) return combos.slice(0, limit)
  }
  return combos
}

export function describeSweep(defs: ParamDef[]): string {
  return defs
    .map((d) => {
      const first = d.values[0]
      const last = d.values[d.values.length - 1]
      const distinct = new Set(d.values.map((v) => String(v))).size > 1
      return distinct && first !== undefined && last !== undefined
        ? `${d.name} = ${first}…${last} (${d.values.length})`
        : `${d.name} = ${first}`
    })
    .join(' · ')
}

export function traceAt(
  vp: Viewport,
  width: number,
  height: number,
  px: number,
  py: number,
  series: Series[],
  tolerancePx = 14,
): TraceHit | undefined {
  const s = scale(vp, width, height)
  let best: TraceHit | undefined
  for (let i = 0; i < series.length; i++) {
    const data = series[i]
    if (!data) continue
    for (const poly of data) {
      for (let j = 0; j + 3 < poly.length; j += 2) {
        const ax = s.x(poly[j] as number)
        const ay = s.y(poly[j + 1] as number)
        const bx = s.x(poly[j + 2] as number)
        const by = s.y(poly[j + 3] as number)
        const d = pointToSegment(px, py, ax, ay, bx, by)
        if (d <= tolerancePx && (!best || d < best.distance)) {
          best = { index: i, x: poly[j] as number, y: poly[j + 1] as number, distance: d }
        }
      }
    }
  }
  return best
}

function pointToSegment(px: number, py: number, ax: number, ay: number, bx: number, by: number): number {
  const vx = bx - ax
  const vy = by - ay
  const len2 = vx * vx + vy * vy
  if (len2 === 0) return Math.hypot(px - ax, py - ay)
  let t = ((px - ax) * vx + (py - ay) * vy) / len2
  t = Math.max(0, Math.min(1, t))
  return Math.hypot(px - (ax + t * vx), py - (ay + t * vy))
}