import type { Tool } from '../registry/types'
import { num, derive, str } from '../lib/values'
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
      { key: 'a', label: 'Acceleration (a)', unit: 'm/s²', default: '9.81' },
      { key: 't', label: 'Time (t)', unit: 's', default: '3' },
    ],
    outputs: [
      { key: 'v', label: 'Final velocity', formula: 'v = u + at', unit: 'm/s', emphasis: 'primary' },
      { key: 'a', label: 'Acceleration', formula: 'a = (v - u) / t', unit: 'm/s²', emphasis: 'primary' },
      { key: 't', label: 'Time', formula: 't = (v - u) / a', unit: 's', emphasis: 'primary' },
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
      if (v.u !== undefined) out.push(`u = ${fmt(num(v, 'u'), 5)} m/s`)
      if (r.a !== undefined) out.push(`a = ${fmt(r.a, 5)} m/s²`)
      if (r.t !== undefined) out.push(`t = ${fmt(r.t, 5)} s`)
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
  {
    id: 'circular-motion',
    category: 'physics',
    name: 'Circular Motion',
    tagline: 'Centripetal acceleration, force and period',
    keywords: ['circular', 'centripetal', 'revolution', 'angular velocity', 'banked', 'centrifugal'],
    inputs: [
      { key: 'v', label: 'Tangential speed (v)', unit: 'm/s', default: '10' },
      { key: 'r', label: 'Radius (r)', unit: 'm', default: '5' },
      { key: 'm', label: 'Mass (m)', unit: 'kg', default: '2' },
      { key: 'g', label: 'Gravity (g)', unit: 'm/s²', default: '9.80665' },
    ],
    outputs: [
      { key: 'ac', label: 'Centripetal acceleration', formula: 'a = v²/r', unit: 'm/s²', emphasis: 'primary' },
      { key: 'fc', label: 'Centripetal force', formula: 'F = mv²/r', unit: 'N', emphasis: 'primary' },
      { key: 'omega', label: 'Angular velocity', formula: 'ω = v/r', unit: 'rad/s' },
      { key: 'period', label: 'Period', formula: 'T = 2πr/v', unit: 's' },
      { key: 'frequency', label: 'Frequency', formula: 'f = v/2πr', unit: 'Hz' },
      { key: 'bank', label: 'Banked curve angle', formula: 'tanθ = v²/rg', unit: '°' },
    ],
    compute: (v) => {
      const vel = num(v, 'v')
      const r = num(v, 'r')
      const mass = num(v, 'm')
      const g = num(v, 'g') ?? 9.80665
      if (vel === undefined || r === undefined || r === 0) return {}
      return {
        ac: (vel * vel) / r,
        fc: mass === undefined ? undefined : (mass * vel * vel) / r,
        omega: vel / r,
        period: (2 * Math.PI * r) / vel,
        frequency: vel / (2 * Math.PI * r),
        bank: (Math.atan((vel * vel) / (r * g)) * 180) / Math.PI,
      }
    },
    steps: (v, r) => {
      const vel = num(v, 'v')
      const rad = num(v, 'r')
      if (vel === undefined || rad === undefined) return []
      return [
        `Centripetal acceleration: a = v²/r = ${fmt(vel)}² / ${fmt(rad)} = ${fmt(r.ac, 5)} m/s²`,
        `Centripetal force: F = m·a = ${fmt(num(v, 'm'), 5)} × ${fmt(r.ac, 5)} = ${fmt(r.fc, 5)} N`,
        `Angular velocity: ω = v/r = ${fmt(vel)} / ${fmt(rad)} = ${fmt(r.omega, 5)} rad/s`,
        `Period: T = 2πr/v = 2π × ${fmt(rad)} / ${fmt(vel)} = ${fmt(r.period, 5)} s`,
      ]
    },
    reference: ['a = v²/r', 'F = mv²/r', 'ω = v/r', 'T = 2πr/v'],
    examples: [{ label: '10 m/s on a 5 m circle', inputs: { v: '10', r: '5', m: '2', g: '9.80665' } }],
  },
  {
    id: 'rotational-dynamics',
    category: 'physics',
    name: 'Rotational Dynamics',
    tagline: 'Torque, moment of inertia and angular acceleration',
    keywords: ['rotational', 'torque', 'moment of inertia', 'angular acceleration', 'rotation', 'swing', 'spin'],
    inputs: [
      { key: 'F', label: 'Force (F)', unit: 'N', default: '10' },
      { key: 'r', label: 'Lever arm (r)', unit: 'm', default: '0.5' },
      { key: 'theta', label: 'Angle (θ)', unit: '°', default: '90' },
      { key: 'mass', label: 'Mass (m)', unit: 'kg', default: '2' },
      { key: 'radius', label: 'Radius / length', unit: 'm', default: '0.3' },
      { key: 'offset', label: 'Offset from axis (d)', unit: 'm', default: '0' },
      {
        key: 'shape',
        label: 'Body shape',
        kind: 'select',
        default: 'disk',
        options: [
          { value: 'point', label: 'Point mass' },
          { value: 'disk', label: 'Solid disc / cylinder' },
          { value: 'sphere', label: 'Solid sphere' },
          { value: 'rod', label: 'Rod about its centre' },
          { value: 'ring', label: 'Thin ring' },
        ],
      },
    ],
    outputs: [
      { key: 'torque', label: 'Torque (τ)', formula: 'τ = F·r·sinθ', unit: 'N·m', emphasis: 'primary' },
      { key: 'inertia', label: 'Moment of inertia (I)', formula: 'from shape', unit: 'kg·m²', emphasis: 'primary' },
      { key: 'totalI', label: 'I about the given axis', formula: 'I = Icm + md²', unit: 'kg·m²' },
      { key: 'alpha', label: 'Angular acceleration (α)', formula: 'α = τ/I', unit: 'rad/s²' },
    ],
    compute: (v) => {
      const F = num(v, 'F')
      const r = num(v, 'r')
      const theta = num(v, 'theta') ?? 90
      const mass = num(v, 'mass')
      const radius = num(v, 'radius')
      const offset = num(v, 'offset') ?? 0
      const shape = str(v, 'shape') || 'disk'
      if (F === undefined || r === undefined) return {}
      const torque = F * r * Math.sin((theta * Math.PI) / 180)
      let I: number | undefined
      if (mass !== undefined && radius !== undefined) {
        const m = mass
        const rr = radius
        if (shape === 'point') I = m * rr * rr
        else if (shape === 'disk') I = 0.5 * m * rr * rr
        else if (shape === 'sphere') I = 0.4 * m * rr * rr
        else if (shape === 'rod') I = (m * rr * rr) / 12
        else if (shape === 'ring') I = m * rr * rr
      }
      const totalI = I === undefined ? undefined : I + (mass ?? 0) * offset * offset
      return {
        torque,
        inertia: I,
        totalI,
        alpha: totalI && totalI !== 0 ? torque / totalI : undefined,
      }
    },
    steps: (v, r) => {
      const F = num(v, 'F')
      const rad = num(v, 'r')
      const theta = num(v, 'theta') ?? 90
      if (F === undefined || rad === undefined) return []
      const out = [`Torque: τ = F·r·sinθ = ${fmt(F, 5)} × ${fmt(rad, 5)} × sin(${fmt(theta, 4)}°) = ${fmt(r.torque, 5)} N·m`]
      if (r.inertia !== undefined) out.push(`Moment of inertia: I = ${fmt(r.inertia, 6)} kg·m²`)
      if (r.totalI !== undefined) {
        out.push(`About the given axis: I = Icm + m·d² = ${fmt(r.totalI, 6)} kg·m²`)
      }
      if (r.alpha !== undefined) out.push(`Angular acceleration: α = τ/I = ${fmt(r.alpha, 5)} rad/s²`)
      return out
    },
    reference: [
      'τ = F·r·sinθ',
      'I point = mr²',
      'I disc = ½mr²',
      'I sphere = ⅖mr²',
      'I rod = ¹⁄₁₂mL²',
      'I ring = mr²',
      'Parallel axis: I = Icm + md²',
    ],
    examples: [{ label: '10 N at 0.5 m on a disc', inputs: { F: '10', r: '0.5', theta: '90', mass: '2', radius: '0.3', offset: '0', shape: 'disk' } }],
  },
  {
    id: 'hooke-spring',
    category: 'physics',
    name: "Hooke's Law & Springs",
    tagline: 'Force, extension, energy and oscillation period',
    keywords: ['hooke', 'spring', 'elastic', 'stiffness', 'oscillation', 'suspended', 'elasticity'],
    inputs: [
      { key: 'k', label: 'Spring constant (k)', unit: 'N/m', default: '200' },
      { key: 'x', label: 'Extension (x)', unit: 'm', default: '0.1' },
      { key: 'm', label: 'Mass (m)', unit: 'kg', default: '2' },
    ],
    outputs: [
      { key: 'F', label: 'Force (F)', formula: 'F = kx', unit: 'N', emphasis: 'primary' },
      { key: 'energy', label: 'Elastic energy', formula: 'E = ½kx²', unit: 'J', emphasis: 'primary' },
      { key: 'period', label: 'Oscillation period', formula: 'T = 2π√(m/k)', unit: 's' },
      { key: 'frequency', label: 'Frequency', formula: 'f = 1/T', unit: 'Hz' },
      { key: 'equilibrium', label: 'Static extension', formula: 'x = mg/k', unit: 'm' },
    ],
    compute: (v) => {
      const k = num(v, 'k')
      const x = num(v, 'x')
      const m = num(v, 'm')
      if (k === undefined || x === undefined || k === 0) return {}
      const g = 9.80665
      const period = m !== undefined ? 2 * Math.PI * Math.sqrt(m / k) : undefined
      return {
        F: k * x,
        energy: 0.5 * k * x * x,
        period,
        frequency: period ? 1 / period : undefined,
        equilibrium: m !== undefined ? (m * g) / k : undefined,
      }
    },
    steps: (v, r) => {
      const k = num(v, 'k')
      const x = num(v, 'x')
      if (k === undefined || x === undefined) return []
      const out = [`Force: F = k·x = ${fmt(k, 5)} × ${fmt(x, 5)} = ${fmt(r.F, 5)} N`]
      out.push(`Elastic energy: E = ½kx² = 0.5 × ${fmt(k, 5)} × ${fmt(x, 5)}² = ${fmt(r.energy, 5)} J`)
      if (r.period !== undefined) {
        out.push(`Oscillation period: T = 2π√(m/k) = ${fmt(r.period, 5)} s`)
      }
      if (r.equilibrium !== undefined) {
        out.push(`Static extension under its own weight: x = mg/k = ${fmt(r.equilibrium, 5)} m`)
      }
      return out
    },
    reference: ['F = -kx', 'E = ½kx²', 'T = 2π√(m/k)'],
    examples: [{ label: 'k = 200 N/m, x = 0.1 m', inputs: { k: '200', x: '0.1', m: '2' } }],
  },
]
