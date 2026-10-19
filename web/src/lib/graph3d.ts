export interface Vec3 {
  x: number
  y: number
  z: number
}

export interface Camera {
  yaw: number
  pitch: number
  zoom: number
  panX: number
  panY: number
}

export interface Projected {
  x: number
  y: number
  depth: number
}

export const DEFAULT_CAMERA: Camera = { yaw: 0.6, pitch: 0.5, zoom: 1, panX: 0, panY: 0 }

export function rotate(v: Vec3, yaw: number, pitch: number): Vec3 {
  const cy = Math.cos(yaw)
  const sy = Math.sin(yaw)
  const x1 = v.x * cy + v.z * sy
  const z1 = -v.x * sy + v.z * cy
  const cp = Math.cos(pitch)
  const sp = Math.sin(pitch)
  return { x: x1, y: v.y * cp - z1 * sp, z: v.y * sp + z1 * cp }
}

export function project(p: Vec3, cam: Camera, width: number, height: number, scale?: number): Projected {
  const r = rotate(p, cam.yaw, cam.pitch)
  const s = scale ?? cam.zoom * Math.min(width, height) * 0.36
  return {
    x: width / 2 + r.x * s + cam.panX,
    y: height / 2 - r.y * s + cam.panY,
    depth: r.z,
  }
}

export interface SurfaceOptions {
  x0: number
  x1: number
  y0: number
  y1: number
  resolution?: number
}

export interface SurfaceQuad {
  corners: { a: number; b: number; c: number; d: number }
  height: number
  nx: number
  ny: number
  nz: number
}

export interface SurfaceMesh {
  points: Vec3[]
  quads: SurfaceQuad[]
  zMin: number
  zMax: number
  maxRadius: number
}

export function buildSurface(
  evaluate: (x: number, y: number) => number,
  opts: SurfaceOptions,
): SurfaceMesh {
  const res = opts.resolution ?? 34
  const points: Vec3[] = []
  const raw: number[] = []
  const valid: boolean[] = []
  let zMin = Infinity
  let zMax = -Infinity

  for (let j = 0; j <= res; j++) {
    const y = opts.y0 + ((opts.y1 - opts.y0) * j) / res
    for (let i = 0; i <= res; i++) {
      const x = opts.x0 + ((opts.x1 - opts.x0) * i) / res
      const z = evaluate(x, y)
      const usable = Number.isFinite(z)
      valid.push(usable)
      if (usable) {
        if (z < zMin) zMin = z
        if (z > zMax) zMax = z
      }
      raw.push(usable ? z : 0)
      points.push({ x, y, z: usable ? z : 0 })
    }
  }

  const span = opts.x1 - opts.x0
  const zRange = zMax - zMin
  const zScale = zRange > 1e-9 ? (span * 0.62) / zRange : 1
  const zCentre = (zMax + zMin) / 2

  for (const p of points) p.z = (p.z - zCentre) * zScale

  const quads: SurfaceQuad[] = []
  const row = res + 1
  for (let j = 0; j < res; j++) {
    for (let i = 0; i < res; i++) {
      const a = j * row + i
      const b = a + 1
      const c = a + row
      const d = c + 1
      if (!(valid[a] && valid[b] && valid[c] && valid[d])) continue

      const pa = points[a] as Vec3
      const pb = points[b] as Vec3
      const pc = points[c] as Vec3

      const ux = pb.x - pa.x
      const uy = pb.y - pa.y
      const uz = pb.z - pa.z
      const vx = pc.x - pa.x
      const vy = pc.y - pa.y
      const vz = pc.z - pa.z
      let nx = uy * vz - uz * vy
      let ny = uz * vx - ux * vz
      let nz = ux * vy - uy * vx
      const len = Math.hypot(nx, ny, nz)
      if (len < 1e-12) continue
      nx /= len
      ny /= len
      nz /= len

      const heights = [raw[a] as number, raw[b] as number, raw[c] as number, raw[d] as number]
      const mid = (heights[0] as number) + (heights[1] as number) + (heights[2] as number) + (heights[3] as number)

      quads.push({
        corners: { a, b, c, d },
        height: mid / 4,
        nx,
        ny,
        nz,
      })
    }
  }

  let maxRadius = 0
  for (const p of points) {
    const r = Math.hypot(p.x, p.y, p.z)
    if (r > maxRadius) maxRadius = r
  }

  return {
    points,
    quads,
    zMin: Number.isFinite(zMin) ? zMin : 0,
    zMax: Number.isFinite(zMax) ? zMax : 0,
    maxRadius,
  }
}

const LIGHT = (() => {
  const v = { x: -0.4, y: 0.75, z: 0.6 }
  const len = Math.hypot(v.x, v.y, v.z)
  return { x: v.x / len, y: v.y / len, z: v.z / len }
})()

export function shade(normal: { nx: number; ny: number; nz: number }): number {
  const d = normal.nx * LIGHT.x + normal.ny * LIGHT.y + normal.nz * LIGHT.z
  return 0.38 + 0.62 * Math.max(0, d)
}

const RAMP: [number, number, number][] = [
  [68, 1, 84],
  [59, 82, 139],
  [33, 145, 140],
  [94, 201, 98],
  [253, 231, 37],
]

export function heightColor(t: number): [number, number, number] {
  const c = Math.max(0, Math.min(1, Number.isFinite(t) ? t : 0))
  const scaled = c * (RAMP.length - 1)
  const i = Math.min(Math.floor(scaled), RAMP.length - 2)
  const f = scaled - i
  const a = RAMP[i] as [number, number, number]
  const b = RAMP[i + 1] as [number, number, number]
  return [
    Math.round(a[0] + (b[0] - a[0]) * f),
    Math.round(a[1] + (b[1] - a[1]) * f),
    Math.round(a[2] + (b[2] - a[2]) * f),
  ]
}

export function normalise(value: number, min: number, max: number): number {
  const range = max - min
  if (range <= 1e-12) return 0.5
  return (value - min) / range
}

export function quadDepth(quad: SurfaceQuad, projected: Projected[]): number {
  const { a, b, c, d } = quad.corners
  const pa = projected[a]
  const pb = projected[b]
  const pc = projected[c]
  const pd = projected[d]
  if (!pa || !pb || !pc || !pd) return 0
  return (pa.depth + pb.depth + pc.depth + pd.depth) / 4
}

export function sortByViewDepth(quads: SurfaceQuad[], projected: Projected[]): SurfaceQuad[] {
  return [...quads]
    .map((quad) => ({ quad, depth: quadDepth(quad, projected) }))
    .sort((a, b) => a.depth - b.depth)
    .map((entry) => entry.quad)
}

export function clampPitch(pitch: number): number {
  return Math.max(-1.5, Math.min(1.5, pitch))
}
