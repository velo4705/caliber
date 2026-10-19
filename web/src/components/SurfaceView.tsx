import { useCallback, useEffect, useMemo, useRef, useState } from 'react'
import type { ChartTool } from '../registry/types'
import { compileExpression, evaluateNode } from '../lib/expr'
import { CATEGORY_MAP } from '../registry/categories'
import { navigate } from '../lib/router'
import { formatNumber } from '../lib/format'
import {
  buildSurface,
  clampPitch,
  DEFAULT_CAMERA,
  heightColor,
  normalise,
  project,
  shade,
  sortByViewDepth,
  type Camera,
} from '../lib/graph3d'

interface Props {
  tool: ChartTool
  params: URLSearchParams
}

const DEFAULT_EXPR = 'sin(x)*cos(y) + 0.25*(x^2 - y^2)'

export function SurfaceView({ tool, params }: Props) {
  const { graph } = tool
  const key = graph.functions[0]?.key ?? 'z'

  const [expr, setExpr] = useState(() => params.get(key) ?? graph.functions[0]?.default ?? DEFAULT_EXPR)
  const [cam, setCam] = useState<Camera>(DEFAULT_CAMERA)
  const [wireframe, setWireframe] = useState(false)
  const [size, setSize] = useState({ w: 800, h: 520 })
  const [theme, setTheme] = useState(() => document.documentElement.dataset.theme ?? 'light')

  const wrapRef = useRef<HTMLDivElement>(null)
  const canvasRef = useRef<HTMLCanvasElement>(null)
  const dragRef = useRef<{ x: number; y: number } | null>(null)
  const rafRef = useRef(0)

  const bounds = useMemo(() => {
    const vp = graph.viewport
    return {
      x0: vp?.x0 ?? -5,
      x1: vp?.x1 ?? 5,
      y0: vp?.y0 ?? -5,
      y1: vp?.y1 ?? 5,
    }
  }, [graph.viewport])

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

  const mesh = useMemo(() => {
    let evaluate: (x: number, y: number) => number
    try {
      const node = compileExpression(expr)
      evaluate = (x, y) => {
        try {
          return evaluateNode(node, { x, y }, 'rad')
        } catch {
          return NaN
        }
      }
    } catch {
      evaluate = () => NaN
    }
    return buildSurface(evaluate, { ...bounds, resolution: 40 })
  }, [expr, bounds])

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
    ctx.fillStyle = bg
    ctx.fillRect(0, 0, w, h)

    const fitScale = cam.zoom * (Math.min(w, h) / 2 - 28) / Math.max(mesh.maxRadius, 1e-6)
    const projected = mesh.points.map((p) => project(p, cam, w, h, fitScale))

    ctx.lineJoin = 'round'

    const drawList = sortByViewDepth(mesh.quads, projected)
      .map((quad) => {
        const { a: ia, b: ib, c: ic, d: id } = quad.corners
        const pa = projected[ia]
        const pb = projected[ib]
        const pc = projected[ic]
        const pd = projected[id]
        if (!pa || !pb || !pc || !pd) return undefined
        return { quad, pa, pb, pc, pd }
      })
      .filter((d): d is NonNullable<typeof d> => d !== undefined)

    for (const { quad, pa, pb, pc, pd } of drawList) {
      const t = normalise(quad.height, mesh.zMin, mesh.zMax)
      const [r, g, b] = heightColor(t)

      if (wireframe) {
        ctx.strokeStyle = `rgba(${r},${g},${b},0.45)`
        ctx.lineWidth = 1
        ctx.beginPath()
        ctx.moveTo(pa.x, pa.y)
        ctx.lineTo(pb.x, pb.y)
        ctx.lineTo(pd.x, pd.y)
        ctx.lineTo(pc.x, pc.y)
        ctx.closePath()
        ctx.stroke()
      } else {
        const lit = shade(quad)
        ctx.fillStyle = `rgb(${Math.round(r * lit)},${Math.round(g * lit)},${Math.round(b * lit)})`
        ctx.beginPath()
        ctx.moveTo(pa.x, pa.y)
        ctx.lineTo(pb.x, pb.y)
        ctx.lineTo(pd.x, pd.y)
        ctx.lineTo(pc.x, pc.y)
        ctx.closePath()
        ctx.fill()
        ctx.strokeStyle = ctx.fillStyle
        ctx.lineWidth = 0.6
        ctx.stroke()
      }
    }

    const axis = dark ? '#3a4256' : '#c8cddb'
    const origin = project({ x: 0, y: 0, z: 0 }, cam, w, h, fitScale)
    const xEnd = project({ x: (bounds.x1 - bounds.x0) / 2, y: 0, z: 0 }, cam, w, h, fitScale)
    const yEnd = project({ x: 0, y: (bounds.y1 - bounds.y0) / 2, z: 0 }, cam, w, h, fitScale)
    ctx.strokeStyle = axis
    ctx.lineWidth = 1.5
    ctx.beginPath()
    ctx.moveTo(origin.x, origin.y)
    ctx.lineTo(xEnd.x, xEnd.y)
    ctx.moveTo(origin.x, origin.y)
    ctx.lineTo(yEnd.x, yEnd.y)
    ctx.stroke()

    ctx.fillStyle = dark ? '#9299ad' : '#626a7d'
    ctx.font = '11px ui-monospace, SFMono-Regular, Menlo, monospace'
    ctx.textAlign = 'center'
    ctx.textBaseline = 'middle'
    ctx.fillText('x', xEnd.x + 10, xEnd.y)
    ctx.fillText('y', yEnd.x - 10, yEnd.y)
  }, [mesh, cam, size, theme, wireframe, bounds])

  useEffect(() => {
    cancelAnimationFrame(rafRef.current)
    rafRef.current = requestAnimationFrame(draw)
    return () => cancelAnimationFrame(rafRef.current)
  }, [draw])

  useEffect(() => {
    const canvas = canvasRef.current
    if (!canvas) return
    const onWheel = (e: WheelEvent) => {
      e.preventDefault()
      setCam((c) => ({ ...c, zoom: Math.max(0.3, Math.min(4, c.zoom * (e.deltaY > 0 ? 0.92 : 1.087))) }))
    }
    canvas.addEventListener('wheel', onWheel, { passive: false })
    return () => canvas.removeEventListener('wheel', onWheel)
  }, [])

  const pointerDown = (e: React.PointerEvent<HTMLCanvasElement>) => {
    ;(e.target as HTMLCanvasElement).setPointerCapture(e.pointerId)
    dragRef.current = { x: e.clientX, y: e.clientY }
  }

  const pointerMove = (e: React.PointerEvent<HTMLCanvasElement>) => {
    const prev = dragRef.current
    if (!prev) return
    const dx = e.clientX - prev.x
    const dy = e.clientY - prev.y
    dragRef.current = { x: e.clientX, y: e.clientY }
    setCam((c) => ({ ...c, yaw: c.yaw + dx * 0.01, pitch: clampPitch(c.pitch + dy * 0.01) }))
  }

  const pointerUp = () => {
    dragRef.current = null
  }

  const category = CATEGORY_MAP.get(tool.category)

  return (
    <article className="tool tool-graph">
      <header className="tool-head">
        <button className="crumb" onClick={() => navigate(`#/c/${tool.category}`)}>
          {category?.icon} {category?.name}
        </button>
        <h1>{tool.name}</h1>
        {tool.tagline && <p className="tool-tagline">{tool.tagline}</p>}
      </header>

      <div className="graph-bar">
        <div className="graph-fn">
          <span className="graph-swatch param" aria-hidden />
          <span className="graph-fn-label">z(x, y)</span>
          <input
            className="mono"
            type="text"
            spellCheck={false}
            autoComplete="off"
            placeholder="z = f(x, y)"
            value={expr}
            onChange={(e) => {
              setExpr(e.target.value)
              history.replaceState(null, '', `#/t/${tool.id}?${key}=${encodeURIComponent(e.target.value)}`)
            }}
          />
        </div>
      </div>

      <div className="graph-stage">
        <div className="graph-canvas-wrap" ref={wrapRef}>
          <canvas
            ref={canvasRef}
            onPointerDown={pointerDown}
            onPointerMove={pointerMove}
            onPointerUp={pointerUp}
            onPointerCancel={pointerUp}
          />
          <div className="graph-readout">
            <span className="hint">drag to rotate · scroll to zoom</span>
          </div>
        </div>

        <div className="graph-tools">
          <span className="graph-mode">z = f(x, y)</span>
          <span className="graph-axes mono">
            x [{formatNumber(bounds.x0, 3)}, {formatNumber(bounds.x1, 3)}] · y [{formatNumber(bounds.y0, 3)},{' '}
            {formatNumber(bounds.y1, 3)}] · z [{formatNumber(mesh.zMin, 3)}, {formatNumber(mesh.zMax, 3)}]
          </span>
          <span className="graph-count">{mesh.quads.length} quads</span>
          <div className="graph-tool-buttons">
            <button className="btn-ghost" onClick={() => setWireframe((v) => !v)}>
              {wireframe ? 'Solid' : 'Wireframe'}
            </button>
            <button className="btn-ghost" onClick={() => setCam(DEFAULT_CAMERA)}>
              Reset
            </button>
          </div>
        </div>
      </div>

      {graph.reference && <div className="graph-ref mono">{graph.reference.join('   ·   ')}</div>}
    </article>
  )
}