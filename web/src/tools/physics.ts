import type { Tool } from '../registry/types'
import { num, derive } from '../lib/values'
import { formatNumber as fmt } from '../lib/format'

const RAD = Math.PI / 180
const G_DEFAULT = 9.80665

export const physicsTools: Tool[] = [
  {
    id: 'kinematics',
    category: 'physics',
    name: 'Kinematics (SUVAT)',
    tagline: 'Constant-acceleration motion',
    keywords: ['kinematics', 'suvat', 'acceleration', 'velocity', 'displacement', 'motion', 'kinematic'],
    inputs: [
      { key: 'u', label: 'Initial velocity (u)', unit: 'm/s', default: '0' },
      { key: 'v', label: 'Final velocity (v)', unit: 'm/s' },
      { key: 'a', label: 'Acceleration (a)', unit: 'm/s²' },
      { key: 't', label: 'Time (t)', unit: 's' },
    ],
    outputs: [
      { key: 'v', label: 'Final velocity', formula: 'v = u + at', unit: 'm/s', emphasis: 'primary' },
      { key: 'a', label: 'Acceleration', formula: 'a = (v − u) / t', unit: 'm/s²', emphasis: 'primary' },
      { key: 't', label: 'Time', formula: 't = (v − u) / a', unit: 's', emphasis: 'primary' },
      { key: 's', label: 'Displacement', formula: 's = ut + ½at²', unit: 'm', emphasis: 'primary' },
    ],
    compute: (v) => {
      const u = num(v, 'u')
      const vv = num(v, 'v')
      const a = num(v, 'a')
      const t = num(v, 't')
      const safeDiv = (x: number | undefined, y: number | undefined) =>
        x === undefined || y === undefined || y === 0 ? undefined : x / y
      const vOut = derive(vv, u !== undefined && a !== undefined && t !== undefined ? u + a * t : undefined)
      const aOut = derive(a, u !== undefined && vv !== undefined ? safeDiv(vv - u, t) : undefined)
      const tOut = derive(t, u !== undefined && vv !== undefined && aOut !== undefined && aOut !== 0 ? (vv - u) / aOut : undefined)
      const s = u !== undefined && aOut !== undefined && t !== undefined ? u * t + 0.5 * aOut * t * t : undefined
      return { v: vOut, a: aOut, t: tOut, s }
    },
    steps: (v, r) => {
      const out: string[] = []
      if (v.u !== undefined) out.push(`u = ${num(v, 'u')} m/s`)
      if (r.a !== undefined) out.push(`a = ${r.a} m/s²`)
      if (r.t !== undefined) out.push(`t = ${r.t} s`)
      if (r.v !== undefined && r.s !== undefined) {
        const rf = fmt(r.v)
        const sf = fmt(r.s)
        out.push(`v = u + at = ${rf} m/s`)
        out.push(`s = ut + ½at² = ${sf} m`)
        out.push(`Check: s = ½(u+v)t = ${fmt(0.5 * ((num(v, 'u') ?? 0) + Number(r.v)) * (num(v, 't') ?? 0))} m`)
      }
      return out
    },
    reference: ['v = u + at', 's = ut + ½at²', 'v² = u² + 2as'],
    examples: [
      { label: 'Free fall from rest', inputs: { u: '0', a: '9.81', t: '3' } },
      { label: 'Car braking', inputs: { u: '20', v: '0', t: '4' } },
    ],
  },
  {
    id: 'projectile',
    category: 'physics',
    name: 'Projectile Motion',
    tagline: 'Range, peak height and flight time',
    keywords: ['projectile', 'trajectory', 'range', 'launch', 'angle', 'parabola', 'ballistic'],
    inputs: [
      { key: 'v0', label: 'Launch speed (v₀)', unit: 'm/s', default: '20' },
      { key: 'theta', label: 'Launch angle (θ)', unit: '°', default: '45' },
      { key: 'g', label: 'Gravity (g)', unit: 'm/s²', default: String(G_DEFAULT) },
    ],
    outputs: [
      { key: 'range', label: 'Horizontal range', formula: 'v₀² sin(2θ) / g', unit: 'm', emphasis: 'primary' },
      { key: 'height', label: 'Maximum height', formula: 'v₀² sin²θ / (2g)', unit: 'm', emphasis: 'primary' },
      { key: 'flight', label: 'Time of flight', formula: '2 v₀ sinθ / g', unit: 's', emphasis: 'primary' },
      { key: 'vx', label: 'Horizontal velocity', formula: 'v₀ cosθ', unit: 'm/s' },
      { key: 'vy', label: 'Initial vertical velocity', formula: 'v₀ sinθ', unit: 'm/s' },
    ],
    compute: (v) => {
      const v0 = num(v, 'v0')
      const th = num(v, 'theta')
      const g = num(v, 'g') ?? G_DEFAULT
      if (v0 === undefined || th === undefined || g === 0) return {}
      const rad = th * RAD
      return {
        range: (v0 * v0 * Math.sin(2 * rad)) / g,
        height: (v0 * v0 * Math.sin(rad) ** 2) / (2 * g),
        flight: (2 * v0 * Math.sin(rad)) / g,
        vx: v0 * Math.cos(rad),
        vy: v0 * Math.sin(rad),
      }
    },
    reference: ['R = v₀² sin(2θ) / g', 'H = v₀² sin²θ / 2g', 'T = 2 v₀ sinθ / g'],
    examples: [{ label: 'Football, 20 m/s at 45°', inputs: { v0: '20', theta: '45', g: '9.80665' } }],
  },
  {
    id: 'kinetic-energy',
    category: 'physics',
    name: 'Energy & Momentum',
    tagline: 'Kinetic energy, momentum and speed from either',
    keywords: ['kinetic', 'energy', 'momentum', 'mass', 'speed', 'work', 'joules'],
    inputs: [
      { key: 'm', label: 'Mass (m)', unit: 'kg', default: '2' },
      { key: 'v', label: 'Speed (v)', unit: 'm/s', default: '10' },
      { key: 'ke', label: 'Kinetic energy', unit: 'J' },
    ],
    outputs: [
      { key: 'ke', label: 'Kinetic energy', formula: 'E = ½mv²', unit: 'J', emphasis: 'primary' },
      { key: 'p', label: 'Momentum', formula: 'p = mv', unit: 'kg·m/s', emphasis: 'primary' },
      { key: 'v', label: 'Speed', formula: 'v = √(2E / m)', unit: 'm/s', emphasis: 'primary' },
    ],
    compute: (v) => {
      const m = num(v, 'm')
      const vv = num(v, 'v')
      const keIn = num(v, 'ke')
      const vOut = derive(vv, m !== undefined && keIn !== undefined && m > 0 ? Math.sqrt((2 * keIn) / m) : undefined)
      const keOut = derive(keIn, m !== undefined && vOut !== undefined ? 0.5 * m * vOut * vOut : undefined)
      const p = m !== undefined && vOut !== undefined ? m * vOut : undefined
      return { ke: keOut, p, v: vOut }
    },
    reference: ['E = ½mv²', 'p = mv'],
    examples: [{ label: '2 kg at 10 m/s', inputs: { m: '2', v: '10' } }],
  },
  {
    id: 'wave',
    category: 'physics',
    name: 'Wave Properties',
    tagline: 'Speed, frequency, wavelength and period',
    keywords: ['wave', 'frequency', 'wavelength', 'period', 'hz', 'amplitude', 'sound', 'light'],
    inputs: [
      { key: 'f', label: 'Frequency (f)', unit: 'Hz', default: '440' },
      { key: 'lambda', label: 'Wavelength (λ)', unit: 'm' },
      { key: 'v', label: 'Speed (v)', unit: 'm/s' },
    ],
    outputs: [
      { key: 'v', label: 'Wave speed', formula: 'v = fλ', unit: 'm/s', emphasis: 'primary' },
      { key: 'lambda', label: 'Wavelength', formula: 'λ = v / f', unit: 'm', emphasis: 'primary' },
      { key: 'f', label: 'Frequency', formula: 'f = v / λ', unit: 'Hz', emphasis: 'primary' },
      { key: 'period', label: 'Period', formula: 'T = 1 / f', unit: 's' },
    ],
    compute: (v) => {
      const f = num(v, 'f')
      const lam = num(v, 'lambda')
      const sp = num(v, 'v')
      const vOut = derive(sp, f !== undefined && lam !== undefined ? f * lam : undefined)
      const lamOut = derive(lam, f !== undefined && f !== 0 && vOut !== undefined ? vOut / f : undefined)
      const fOut = derive(f, lamOut !== undefined && lamOut !== 0 && vOut !== undefined ? vOut / lamOut : undefined)
      return { v: vOut, lambda: lamOut, f: fOut, period: fOut !== undefined && fOut !== 0 ? 1 / fOut : undefined }
    },
    reference: ['v = fλ', 'T = 1 / f'],
    examples: [
      { label: 'A440 concert pitch', inputs: { f: '440', lambda: '0.78' } },
      { label: 'Visible light', inputs: { f: '5e14', lambda: '6e-7' } },
    ],
  },
  {
    id: 'newton',
    category: 'physics',
    name: 'Newton’s Laws',
    tagline: 'F = ma and weight',
    keywords: ['newton', 'force', 'mass', 'acceleration', 'weight', 'fma', 'gravity'],
    inputs: [
      { key: 'm', label: 'Mass (m)', unit: 'kg', default: '10' },
      { key: 'f', label: 'Force (F)', unit: 'N' },
      { key: 'a', label: 'Acceleration (a)', unit: 'm/s²' },
      { key: 'g', label: 'Gravity (g)', unit: 'm/s²', default: String(G_DEFAULT) },
    ],
    outputs: [
      { key: 'f', label: 'Force', formula: 'F = ma', unit: 'N', emphasis: 'primary' },
      { key: 'a', label: 'Acceleration', formula: 'a = F / m', unit: 'm/s²', emphasis: 'primary' },
      { key: 'weight', label: 'Weight', formula: 'W = mg', unit: 'N', emphasis: 'primary' },
    ],
    compute: (v) => {
      const m = num(v, 'm')
      const f = num(v, 'f')
      const a = num(v, 'a')
      const g = num(v, 'g') ?? G_DEFAULT
      const fOut = derive(f, m !== undefined && a !== undefined ? m * a : undefined)
      const aOut = derive(a, m !== undefined && m !== 0 && fOut !== undefined ? fOut / m : undefined)
      return { f: fOut, a: aOut, weight: m !== undefined ? m * g : undefined }
    },
    reference: ['F = ma', 'W = mg'],
    examples: [{ label: '50 N on 10 kg', inputs: { m: '10', f: '50' } }],
  },
]