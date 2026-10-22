import { useCallback, useEffect, useMemo, useRef, useState } from 'react'
import type { ChartTool, GraphMode } from '../registry/types'
import { compileExpression, evaluateNode, type Node } from '../lib/expr'
import { CATEGORY_MAP } from '../registry/categories'
import { navigate } from '../lib/router'
import { formatNumber } from '../lib/format'
import {
  clampViewport,
  DEFAULT_VP,
  describeSweep,
  findXIntercepts,
  findYIntercept,
  formatRoot,
  formatTick,
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
  traceAt,
  usesParam,
  zoomAt,
  type ParamDef,
  type Series,
  type TraceHit,
  type Viewport,
} from '../lib/graph'

const PALETTE = ['#6366f1', '#f97316', '#10b981', '#ec4899', '#06b6d4', '#a855f7']

interface Props {
  tool: ChartTool
  params: URLSearchParams
}

const MODE_LABEL: Record<GraphMode, string> = {
  cartesian: 'y = f(x)',
  polar: 'r = f(θ)',
  parametric: 'x = x(t), y = y(t)',
  implicit: 'g(x, y) = 0',
  surface: 'z = f(x, y)',
}

export function GraphView({ tool, params }: Props) {
  const { graph } = tool

  const initial = useMemo(() => {
    const out: Record<string, string> = {}
    for (const fn of graph.functions) {
      const p = params.get(fn.key)
      out[fn.key] = p ?? fn.default ?? ''
    }
    return out
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [tool.id])

  const [exprs, setExprs] = useState<Record<string, string>>(initial)
  const [hidden, setHidden] = useState<Set<string>>(new Set())
  const [vp, setVp] = useState<Viewport>(graph.viewport ?? DEFAULT_VP)
  const [hover, setHover] = useState<{ px: number; py: number } | null>(null)
  const [dragging, setDragging] = useState(false)
  const [showDeriv, setShowDeriv] = useState(false)
  const [showIntercepts, setShowIntercepts] = useState(false)
  const [shadeOn, setShadeOn] = useState(false)
  const [shadeKey, setShadeKey] = useState('')
  const [shadeA, setShadeA] = useState('0')
  const [shadeB, setShadeB] = useState('1')
  const [size, setSize] = useState({ w: 800, h: 500 })
  const [theme, setTheme] = useState(() => document.documentElement.dataset.theme ?? 'light')

  const wrapRef = useRef<HTMLDivElement>(null)
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const pointers = useRef(new Map<number, { x: number; y: number }>())
  const pinchRef = useRef<{ dist: number; cx: number; cy: number } | null>(null)
  const rafRef = useRef(0)

  useEffect(() => {
    const observer = new ResizeObserver((entries) => {
      const rect = entries[0]?.contentRect
      if (rect) setSize({ w: Math.max(rect.width, 1), h: Math.max(rect.height, 1) })
    })
    if (wrapRef.current) observer.observe(wrapRef.current)
    return () => observer.disconnect()
  }, [])

  useEffect(() => {
    const mo = new MutationObserver(() => setTheme(document.documentElement.dataset.theme ?? 'light'))
    mo.observe(document.documentElement, { attributes: true, attributeFilter: ['data-theme'] })
    return () => mo.disconnect()
  }, [])

  const setExpr = useCallback(
    (key: string, value: string) => {
      setExprs((prev) => {
        const next = { ...prev, [key]: value }
        const qs = new URLSearchParams()
        for (const [k, v] of Object.entries(next)) {
          if (v && v !== (graph.functions.find((f) => f.key === k)?.default ?? '')) qs.set(k, v)
        }
        history.replaceState(null, '', `#/t/${tool.id}${qs.toString() ? `?${qs}` : ''}`)
        return next
      })
    },
    [graph.functions, tool.id],
  )

  const slots = useMemo(() => {
    const defs: Record<string, ParamDef> = {}
    for (const fn of graph.functions) {
      const d = parseParamDef((exprs[fn.key] ?? '').trim())
      if (d) defs[fn.key] = d
    }
    const params = Object.values(defs)
    return {
      defs,
      params,
      names: params.map((p) => p.name),
      combos: paramProduct(params),
      sweptKeys: new Set(Object.keys(defs)),
    }
  }, [exprs, graph.functions])

  const errors = useMemo(() => {
    const out: Record<string, string> = {}
    for (const fn of graph.functions) {
      const src = (exprs[fn.key] ?? '').trim()
      if (!src || slots.defs[fn.key]) continue
      const vars: Record<string, number> = { x: 1, y: 1, t: 1 }
      for (const name of slots.names) {
        if (!usesParam(src, name)) continue
        const combo = slots.combos.find((c) => name in c)
        if (combo) vars[name] = combo[name] as number
      }
      try {
        evaluateNode(compileExpression(src), vars, 'rad')
      } catch (e) {
        const msg = e instanceof Error ? e.message : 'Invalid expression'
        out[fn.key] = msg.includes('Unknown name')
          ? `${msg} — is it a parameter you have not defined?`
          : msg
      }
    }
    return out
  }, [exprs, graph.functions, slots])

  const plots = useMemo(() => {
    const visible = (key: string) => (exprs[key] ?? '').trim() !== '' && !hidden.has(key) && !slots.sweptKeys.has(key)

    const out: { key: string; fn: (typeof graph.functions)[number]; src: string; vars: Record<string, number> }[] = []

    if (graph.mode === 'parametric') {
      for (const fn of graph.functions) {
        if (!fn.key.startsWith('x') || !visible(fn.key)) continue
        const partner = graph.functions.find((f) => f.key === `y${fn.key.slice(1)}`)
        if (!partner || !visible(partner.key)) continue
        out.push({ key: fn.key, fn, src: exprs[fn.key] as string, vars: {} })
      }
      return out
    }

    for (const fn of graph.functions) {
      if (!visible(fn.key)) continue
      const src = exprs[fn.key] as string
      const referenced = slots.names.filter((n) => usesParam(src, n))
      if (referenced.length === 0) {
        out.push({ key: fn.key, fn, src, vars: {} })
      } else {
        for (const combo of slots.combos) {
          if (!referenced.every((n) => n in combo)) continue
          out.push({ key: fn.key, fn, src, vars: combo })
        }
      }
    }
    return out
  }, [exprs, hidden, graph.mode, graph.tRange, graph.functions, slots])

const evaluators = useMemo(() => {
    return plots.map(({ key, fn, src, vars }, i) => {
      let node: Node | undefined
      try {
        node = compileExpression(src)
      } catch {
        node = undefined
      }
      const call = (extra: Record<string, number>) => {
        if (!node) return NaN
        try {
          return evaluateNode(node, { ...vars, ...extra }, 'rad')
        } catch {
          return NaN
        }
      }
      const color = fn.color ?? (PALETTE[i % PALETTE.length] as string)
      return { key, uid: `${key}#${i}`, color, call, src, vars, valid: node !== undefined }
    })
  }, [plots])

  const shadeOptions = useMemo(() => {
    const seen = new Map<string, string>()
    for (const e of evaluators) if (!seen.has(e.key)) seen.set(e.key, e.src)
    return [...seen.entries()]
  }, [evaluators])

  const intercepts = useMemo(() => {
    if (!showIntercepts || graph.mode !== 'cartesian') return []
    const resolution = dragging ? 300 : 1200
    return evaluators.map((ev) => ({
      color: ev.color,
      roots: findXIntercepts((x) => ev.call({ x }), vp.x0, vp.x1, resolution),
      yIntercept: findYIntercept((x) => ev.call({ x }), vp.x0, vp.x1),
    }))
  }, [showIntercepts, evaluators, graph.mode, vp, dragging])

  const series = useMemo(() => {
    const resolution = dragging ? 260 : 1400
    const [t0, t1] = graph.tRange ?? [0, Math.PI * 2]
    return evaluators.map((ev) => {
      const color = ev.color
      if (graph.mode === 'cartesian') {
        const pad = (vp.y1 - vp.y0) * 12
        return {
          color,
          data: sampleCartesian((x) => ev.call({ x }), vp.x0, vp.x1, resolution, { lo: vp.y0 - pad, hi: vp.y1 + pad }),
        }
      }
      if (graph.mode === 'polar') {
        return { color, data: samplePolar((t) => ev.call({ t }), 1, resolution) }
      }
      if (graph.mode === 'parametric') {
        const partnerKey = `y${ev.key.slice(1)}`
        const partner = evaluators.find((e) => e.key === partnerKey)
        if (!partner) return { color, data: [] as Series }
        return {
          color,
          data: sampleParametric((t) => ev.call({ t }), (t) => partner.call({ t }), t0, t1, resolution),
        }
      }
      return { color, data: sampleImplicit((x, y) => ev.call({ x, y }), vp, dragging ? 40 : 110) }
    })
  }, [evaluators, graph.mode, graph.tRange, vp, dragging])

  const derivatives = useMemo(() => {
    if (!showDeriv || graph.mode !== 'cartesian') return []
    const resolution = dragging ? 260 : 1400
    return evaluators.map((ev) => ({
      color: ev.color,
      data: numericDerivative((x) => ev.call({ x }), vp.x0, vp.x1, resolution),
    }))
  }, [showDeriv, evaluators, graph.mode, vp, dragging])

  const shaded = useMemo(() => {
    if (!shadeOn || graph.mode !== 'cartesian') return undefined
    const target = evaluators.find((e) => e.key === shadeKey) ?? evaluators[0]
    if (!target) return undefined
    const a = Number(shadeA)
    const b = Number(shadeB)
    if (!Number.isFinite(a) || !Number.isFinite(b)) return undefined
    const path = shadeArea((x) => target.call({ x }), a, b, 0)
    if (!path) return undefined
    return {
      color: target.color,
      path,
      value: integrate((x) => target.call({ x }), a, b),
    }
  }, [shadeOn, shadeKey, shadeA, shadeB, evaluators, graph.mode])

  const draw = useCallback(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const dpr = window.devicePixelRatio || 1
    const { w, h } = size
    canvas.width = Math.round(w * dpr)
    canvas.height = Math.round(h * dpr)
    canvas.style.width = `${w}px`
    canvas.style.height = `${h}px`

    const ctx = canvas.getContext('2d')
    if (!ctx) return
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0)

    const dark = theme === 'dark'
    const bg = dark ? '#161923' : '#ffffff'
    const grid = dark ? '#232838' : '#eceef3'
    const axis = dark ? '#3a4256' : '#c8cddb'
    const text = dark ? '#9299ad' : '#626a7d'

    ctx.fillStyle = bg
    ctx.fillRect(0, 0, w, h)

    const s = scale(vp, w, h)

    ctx.lineWidth = 1
    ctx.strokeStyle = grid
    ctx.beginPath()
    for (const tx of ticks(vp.x0, vp.x1, Math.max(4, Math.round(w / 90)))) {
      const px = Math.round(s.x(tx)) + 0.5
      ctx.moveTo(px, 0)
      ctx.lineTo(px, h)
    }
    for (const ty of ticks(vp.y0, vp.y1, Math.max(4, Math.round(h / 70)))) {
      const py = Math.round(s.y(ty)) + 0.5
      ctx.moveTo(0, py)
      ctx.lineTo(w, py)
    }
    ctx.stroke()

    ctx.strokeStyle = axis
    ctx.lineWidth = 1.5
    ctx.beginPath()
    if (vp.x0 <= 0 && vp.x1 >= 0) {
      const px = Math.round(s.x(0)) + 0.5
      ctx.moveTo(px, 0)
      ctx.lineTo(px, h)
    }
    if (vp.y0 <= 0 && vp.y1 >= 0) {
      const py = Math.round(s.y(0)) + 0.5
      ctx.moveTo(0, py)
      ctx.lineTo(w, py)
    }
    ctx.stroke()

    ctx.fillStyle = text
    ctx.font = '11px ui-monospace, SFMono-Regular, Menlo, monospace'
    ctx.textBaseline = 'top'
    for (const tx of ticks(vp.x0, vp.x1, Math.max(4, Math.round(w / 90)))) {
      const px = s.x(tx)
      if (px < 24 || px > w - 24) continue
      ctx.textAlign = 'center'
      ctx.fillText(formatTick(tx), px, h - 16)
    }
    ctx.textBaseline = 'middle'
    for (const ty of ticks(vp.y0, vp.y1, Math.max(4, Math.round(h / 70)))) {
      const py = s.y(ty)
      if (py < 12 || py > h - 12) continue
      ctx.textAlign = 'left'
      ctx.fillText(formatTick(ty), 6, py)
    }

    ctx.save()
    ctx.beginPath()
    ctx.rect(0, 0, w, h)
    ctx.clip()

    if (shaded) {
      ctx.globalAlpha = 0.18
      ctx.fillStyle = shaded.color
      ctx.beginPath()
      const p = shaded.path
      for (let i = 0; i + 1 < p.length; i += 2) {
        const px = s.x(p[i] as number)
        const py = s.y(p[i + 1] as number)
        if (i === 0) ctx.moveTo(px, py)
        else ctx.lineTo(px, py)
      }
      ctx.closePath()
      ctx.fill()
      ctx.globalAlpha = 1
    }

    ctx.lineWidth = 2
    ctx.lineJoin = 'round'
    ctx.lineCap = 'round'
    for (const { color, data } of series) {
      ctx.strokeStyle = color
      ctx.beginPath()
      for (const seg of data) {
        for (let i = 0; i + 1 < seg.length; i += 2) {
          const px = s.x(seg[i] as number)
          const py = s.y(seg[i + 1] as number)
          if (i === 0) ctx.moveTo(px, py)
          else ctx.lineTo(px, py)
        }
      }
      ctx.stroke()
    }

    ctx.setLineDash([5, 4])
    ctx.lineWidth = 1.5
    ctx.globalAlpha = 0.85
    for (const { color, data } of derivatives) {
      ctx.strokeStyle = color
      ctx.beginPath()
      for (const seg of data) {
        for (let i = 0; i + 1 < seg.length; i += 2) {
          const px = s.x(seg[i] as number)
          const py = s.y(seg[i + 1] as number)
          if (i === 0) ctx.moveTo(px, py)
          else ctx.lineTo(px, py)
        }
      }
      ctx.stroke()
    }
    ctx.setLineDash([])
    ctx.globalAlpha = 1

    if (intercepts.length) {
      const axisY = s.y(0)
      const onAxis = vp.y0 <= 0 && vp.y1 >= 0
      const placed: { x: number; y: number; w: number; h: number }[] = []

      for (const { color, roots, yIntercept } of intercepts) {
        for (const root of roots) {
          const px = s.x(root)
          if (px < -20 || px > w + 20) continue

          if (onAxis) {
            ctx.fillStyle = color
            ctx.beginPath()
            ctx.arc(px, axisY, 4, 0, Math.PI * 2)
            ctx.fill()
            ctx.strokeStyle = bg
            ctx.lineWidth = 2
            ctx.stroke()
          }

          const label = formatRoot(root)
          ctx.font = '11px ui-monospace, SFMono-Regular, Menlo, monospace'
          const textWidth = ctx.measureText(label).width
          const boxW = textWidth + 8
          const boxH = 15

          for (let attempt = 0; attempt < 4; attempt++) {
            const above = attempt % 2 === 0
            const ly = onAxis ? (above ? axisY - 20 : axisY + 9) : 14 + attempt * 17
            const lx = Math.min(Math.max(px - boxW / 2, 2), w - boxW - 2)
            const clash = placed.some(
              (p) => lx < p.x + p.w && lx + boxW > p.x && ly - boxH / 2 < p.y + p.h / 2 && ly + boxH / 2 > p.y - p.h / 2,
            )
            if (!clash) {
              placed.push({ x: lx, y: ly, w: boxW, h: boxH })
              ctx.fillStyle = color
              ctx.globalAlpha = 0.18
              ctx.fillRect(lx, ly - boxH / 2, boxW, boxH)
              ctx.globalAlpha = 1
              ctx.fillStyle = color
              ctx.textAlign = 'left'
              ctx.textBaseline = 'middle'
              ctx.fillText(label, lx + 4, ly)
              break
            }
          }
        }

        if (yIntercept !== undefined) {
          const py = s.y(yIntercept)
          if (py > -20 && py < h + 20) {
            ctx.fillStyle = color
            ctx.beginPath()
            ctx.arc(s.x(0), py, 4, 0, Math.PI * 2)
            ctx.fill()
            ctx.strokeStyle = bg
            ctx.lineWidth = 2
            ctx.stroke()

            const label = formatRoot(yIntercept)
            ctx.font = '11px ui-monospace, SFMono-Regular, Menlo, monospace'
            const textWidth = ctx.measureText(label).width
            const boxW = textWidth + 8
            const boxH = 15
            const lx = Math.min(s.x(0) + 9, w - boxW - 2)
            const ly = py
            const clash = placed.some(
              (p) => lx < p.x + p.w && lx + boxW > p.x && ly - boxH / 2 < p.y + p.h / 2 && ly + boxH / 2 > p.y - p.h / 2,
            )
            if (!clash) {
              placed.push({ x: lx, y: ly, w: boxW, h: boxH })
              ctx.fillStyle = color
              ctx.globalAlpha = 0.18
              ctx.fillRect(lx, ly - boxH / 2, boxW, boxH)
              ctx.globalAlpha = 1
              ctx.fillStyle = color
              ctx.textAlign = 'left'
              ctx.textBaseline = 'middle'
              ctx.fillText(label, lx + 4, ly)
            }
          }
        }
      }
    }

    ctx.restore()

    if (hover) {
      const hit = traceAt(vp, w, h, hover.px, hover.py, series.map((s2) => s2.data), 16)
      if (hit) {
        const px = s.x(hit.x)
        const py = s.y(hit.y)
        ctx.strokeStyle = dark ? '#4b5468' : '#b9c0d0'
        ctx.setLineDash([4, 4])
        ctx.lineWidth = 1
        ctx.beginPath()
        ctx.moveTo(px, 0)
        ctx.lineTo(px, h)
        ctx.moveTo(0, py)
        ctx.lineTo(w, py)
        ctx.stroke()
        ctx.setLineDash([])
        ctx.fillStyle = series[hit.index]?.color ?? '#6366f1'
        ctx.beginPath()
        ctx.arc(px, py, 5, 0, Math.PI * 2)
        ctx.fill()
        ctx.strokeStyle = bg
        ctx.lineWidth = 2
        ctx.stroke()
      }
    }
  }, [series, derivatives, shaded, intercepts, vp, size, theme, hover])

  useEffect(() => {
    cancelAnimationFrame(rafRef.current)
    rafRef.current = requestAnimationFrame(draw)
    return () => cancelAnimationFrame(rafRef.current)
  }, [draw])

  const trace = useMemo<TraceHit | undefined>(() => {
    if (!hover) return undefined
    return traceAt(vp, size.w, size.h, hover.px, hover.py, series.map((s) => s.data), 16)
  }, [hover, vp, size, series])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const onWheel = (e: WheelEvent) => {
      e.preventDefault()
      const rect = canvas.getBoundingClientRect()
      const factor = e.deltaY > 0 ? 1.15 : 1 / 1.15
      setVp((prev) => zoomAt(prev, rect.width, rect.height, e.clientX - rect.left, e.clientY - rect.top, factor))
    }
    canvas.addEventListener('wheel', onWheel, { passive: false })
    return () => canvas.removeEventListener('wheel', onWheel)
  }, [])

  const pointerDown = (e: React.PointerEvent<HTMLCanvasElement>) => {
    ;(e.target as HTMLCanvasElement).setPointerCapture(e.pointerId)
    pointers.current.set(e.pointerId, { x: e.clientX, y: e.clientY })
    if (pointers.current.size === 1) setDragging(true)
    if (pointers.current.size === 2) {
      const [a, b] = [...pointers.current.values()]
      if (a && b) pinchRef.current = { dist: Math.hypot(a.x - b.x, a.y - b.y), cx: (a.x + b.x) / 2, cy: (a.y + b.y) / 2 }
    }
  }

  const pointerMove = (e: React.PointerEvent<HTMLCanvasElement>) => {
    const rect = e.currentTarget.getBoundingClientRect()
    const px = e.clientX - rect.left
    const py = e.clientY - rect.top

    if (pointers.current.has(e.pointerId)) pointers.current.set(e.pointerId, { x: e.clientX, y: e.clientY })

    if (pointers.current.size === 2 && pinchRef.current) {
      const [a, b] = [...pointers.current.values()]
      if (!a || !b) return
      const pinch = pinchRef.current
      const dist = Math.hypot(a.x - b.x, a.y - b.y)
      const cx = (a.x + b.x) / 2
      const cy = (a.y + b.y) / 2
      setVp((prev) => {
        const zoomed = zoomAt(prev, rect.width, rect.height, cx - rect.left, cy - rect.top, pinch.dist / dist)
        return pan(zoomed, rect.width, rect.height, cx - pinch.cx, cy - pinch.cy)
      })
      pinchRef.current = { dist, cx, cy }
      return
    }

    if (dragging && pointers.current.size === 1) {
      const prev = pointers.current.get(e.pointerId)
      if (prev) {
        setVp((p) => pan(p, rect.width, rect.height, e.clientX - prev.x, e.clientY - prev.y))
        pointers.current.set(e.pointerId, { x: e.clientX, y: e.clientY })
      }
    } else {
      setHover({ px, py })
    }
  }

  const pointerUp = (e: React.PointerEvent<HTMLCanvasElement>) => {
    pointers.current.delete(e.pointerId)
    if (pointers.current.size < 2) pinchRef.current = null
    if (pointers.current.size === 0) setDragging(false)
  }

  const reset = () => {
    setVp(clampViewport(graph.viewport ?? DEFAULT_VP))
    setExprs(initial)
  }

  const zoomButtons = (factor: number) => () =>
    setVp((prev) => zoomAt(prev, size.w, size.h, size.w / 2, size.h / 2, factor))

  const category = CATEGORY_MAP.get(tool.category)
  const used = new Set(Object.values(exprs).map((v) => v.trim()).filter(Boolean))

  return (
    <article className="tool tool-graph">
      <header className="tool-head">
        <button className="crumb" onClick={() => navigate(`#/c/${tool.category}`)}>
          {category?.icon} {category?.name}
        </button>
        <h1>{tool.name}</h1>
        {tool.tagline && <p className="tool-tagline">{tool.tagline}</p>}
      </header>

      {tool.examples && (
        <div className="examples">
          <span className="examples-label">Try</span>
          {tool.examples.map((ex) => (
            <button
              key={ex.label}
              className="chip"
              onClick={() => {
                setExprs({ ...initial, ...ex.inputs })
                setVp(graph.viewport ?? DEFAULT_VP)
              }}
            >
              {ex.label}
            </button>
          ))}
        </div>
      )}

      <div className="graph-bar">
        {graph.functions.map((fn, i) => {
          const off = hidden.has(fn.key)
          const isParam = slots.defs[fn.key]
          const error = errors[fn.key]
          return (
            <div key={fn.key} className={`graph-fn${off ? ' off' : ''}${isParam ? ' is-param' : ''}`}>
              {isParam ? (
                <span className="graph-swatch param" aria-hidden />
              ) : (
                <button
                  className="graph-swatch"
                  style={{ background: off ? 'var(--muted)' : (fn.color ?? PALETTE[i % PALETTE.length]) }}
                  onClick={() =>
                    setHidden((prev) => {
                      const next = new Set(prev)
                      if (next.has(fn.key)) next.delete(fn.key)
                      else next.add(fn.key)
                      return next
                    })
                  }
                  aria-label={off ? `Show ${fn.label}` : `Hide ${fn.label}`}
                />
              )}
              <span className="graph-fn-label">{isParam ? 'range' : fn.label}</span>
              <input
                className="mono"
                type="text"
                spellCheck={false}
                autoComplete="off"
                aria-invalid={error ? true : undefined}
                placeholder={
                  isParam
                    ? 'name = [1...5]'
                    : graph.mode === 'implicit'
                      ? 'x² + y² - 4'
                      : graph.mode === 'polar'
                        ? 'r(θ)'
                        : 'y = …'
                }
                value={exprs[fn.key] ?? ''}
                onChange={(e) => setExpr(fn.key, e.target.value)}
              />
              {error && <span className="graph-error">{error}</span>}
            </div>
          )
        })}
      </div>

      <div className="graph-analysis">
        {graph.mode === 'cartesian' ? (
          <>
            <label className="graph-check">
              <input type="checkbox" checked={showDeriv} onChange={(e) => setShowDeriv(e.target.checked)} />
              d/dx overlay
            </label>
            <label className="graph-check">
              <input type="checkbox" checked={showIntercepts} onChange={(e) => setShowIntercepts(e.target.checked)} />
              intercepts
            </label>
            <label className="graph-check">
              <input type="checkbox" checked={shadeOn} onChange={(e) => setShadeOn(e.target.checked)} />
              shade ∫
            </label>
            {shadeOn && (
              <div className="graph-integral">
                <select
                  value={shadeKey || shadeOptions[0]?.[0] || ''}
                  onChange={(e) => setShadeKey(e.target.value)}
                >
                  {shadeOptions.map(([k, src]) => (
                    <option key={k} value={k}>
                      {src}
                    </option>
                  ))}
                </select>
                <input type="number" step="any" value={shadeA} onChange={(e) => setShadeA(e.target.value)} aria-label="Lower bound" />
                <span className="to">to</span>
                <input type="number" step="any" value={shadeB} onChange={(e) => setShadeB(e.target.value)} aria-label="Upper bound" />
                <span className={`graph-integral-value${shaded ? '' : ' missing'}`}>
                  ∫ = {shaded ? formatNumber(shaded.value, 6) : '—'}
                </span>
              </div>
            )}
          </>
        ) : (
          <span className="graph-check-hint">Derivative and integral shading apply to Cartesian plots.</span>
        )}
      </div>

      <div className="graph-stage">
        <div className="graph-canvas-wrap" ref={wrapRef}>
          <canvas
            ref={canvasRef}
            onPointerDown={pointerDown}
            onPointerMove={pointerMove}
            onPointerUp={pointerUp}
            onPointerCancel={pointerUp}
            onPointerLeave={() => {
              if (!dragging) setHover(null)
            }}
          />
          <div className="graph-readout">
            {trace ? (
              <>
                <span className="dot" style={{ background: series[trace.index]?.color }} />
                <span className="mono">
                  x = {formatNumber(trace.x, 5)} &nbsp; y = {formatNumber(trace.y, 5)}
                </span>
              </>
            ) : (
              <span className="hint">
                drag to pan · scroll or pinch to zoom{graph.mode === 'implicit' ? ' · hover to trace' : ''}
              </span>
            )}
          </div>
        </div>

        <div className="graph-tools">
          <span className="graph-mode">{MODE_LABEL[graph.mode]}</span>
          <span className="graph-axes mono">
            {slots.params.length > 0
              ? `sweep · ${describeSweep(slots.params)}`
              : `x [${formatNumber(vp.x0, 4)}, ${formatNumber(vp.x1, 4)}] · y [${formatNumber(vp.y0, 4)}, ${formatNumber(vp.y1, 4)}]`}
          </span>
          <span className="graph-count">{series.length} series</span>
          <div className="graph-tool-buttons">
            <button className="btn-ghost" onClick={zoomButtons(1 / 1.4)} aria-label="Zoom in">
              +
            </button>
            <button className="btn-ghost" onClick={zoomButtons(1.4)} aria-label="Zoom out">
              -
            </button>
            <button className="btn-ghost" onClick={reset}>
              Reset
            </button>
          </div>
        </div>
      </div>

      {graph.reference && (
        <div className="graph-ref mono">{graph.reference.join('   ·   ')}</div>
      )}
      <span className="sr-only">{used.size} active expressions</span>
    </article>
  )
}