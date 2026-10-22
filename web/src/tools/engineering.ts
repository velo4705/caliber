import type { Tool } from '../registry/types'
import { num, derive, str } from '../lib/values'
import { formatNumber as fmt } from '../lib/format'

const SHAPES = [
  { value: 'round', label: 'Solid round' },
  { value: 'hollow', label: 'Hollow round' },
  { value: 'square', label: 'Solid square' },
  { value: 'rect', label: 'Rectangle' },
  { value: 'ibeam', label: 'I-beam' },
]

const E_STEEL = 200e9
export const engineeringTools: Tool[] = [
  {
    id: 'beam-deflection',
    category: 'engineering',
    name: 'Beam Deflection',
    tagline: 'Cantilever with a central point load',
    keywords: ['beam', 'deflection', 'cantilever', 'bending', 'stiffness', 'flexural', 'euler'],
    inputs: [
      { key: 'P', label: 'Point load (P)', unit: 'N', default: '500' },
      { key: 'L', label: 'Length (L)', unit: 'm', default: '1.5' },
      { key: 'shape', label: 'Cross-section', kind: 'select', default: 'rect', options: SHAPES },
      { key: 'b', label: 'Width / Ø (b)', unit: 'm', default: '0.05' },
      { key: 'h', label: 'Height / Ø (h)', unit: 'm', default: '0.1' },
      { key: 't', label: 'Wall thickness (t)', unit: 'm', default: '0.005' },
      { key: 'E', label: 'Young’s modulus (E)', unit: 'Pa' },
    ],
    outputs: [
      { key: 'I', label: 'Second moment (I)', formula: 'from section', unit: 'm⁴', emphasis: 'primary' },
      { key: 'delta', label: 'Max deflection', formula: 'PL³ / (48EI)', unit: 'm', emphasis: 'primary' },
      { key: 'deltaMm', label: 'Max deflection', unit: 'mm', formula: 'δ × 1000', emphasis: 'primary' },
      { key: 'sigma', label: 'Max bending stress', formula: 'Mc / I', unit: 'Pa', emphasis: 'primary' },
      { key: 'slope', label: 'End slope', formula: 'PL² / (16EI)', unit: 'rad' },
      { key: 'stiffness', label: 'Stiffness', formula: '48EI / L³', unit: 'N/m' },
    ],
    compute: (v) => {
      const P = num(v, 'P')
      const L = num(v, 'L')
      const b = num(v, 'b')
      const h = num(v, 'h')
      const t = num(v, 't')
      const shape = str(v, 'shape') || 'rect'
      const E = derive(num(v, 'E'), 200e9)
      if (P === undefined || L === undefined || b === undefined || h === undefined || E === undefined || E === 0) return {}

      let I: number | undefined
      let c: number | undefined
      switch (shape) {
        case 'round': I = (Math.PI * Math.pow(b, 4)) / 64; c = b / 2; break
        case 'hollow': {
          const th = t ?? b / 10
          I = (Math.PI * (Math.pow(b, 4) - Math.pow(Math.max(b - 2 * th, 0), 4))) / 64
          c = b / 2
          break
        }
        case 'square': I = Math.pow(b, 4) / 12; c = b / 2; break
        case 'rect': I = (b * Math.pow(h, 3)) / 12; c = h / 2; break
        case 'ibeam': {
          const th = t ?? h / 20
          const inner = Math.max(h - 2 * th, 0)
          I = (b * th * (Math.pow(h, 2) - Math.pow(inner, 2))) / 12
          c = h / 2
          break
        }
        default: I = undefined
      }
      if (I === undefined) return {}
      const cVal = derive(c, h / 2)
      return {
        I,
        delta: (P * Math.pow(L, 3)) / (48 * E * I),
        deltaMm: ((P * Math.pow(L, 3)) / (48 * E * I)) * 1000,
        sigma: cVal !== undefined ? (P * cVal) / I : undefined,
        slope: (P * Math.pow(L, 2)) / (16 * E * I),
        stiffness: (48 * E * I) / Math.pow(L, 3),
      }
    },
    reference: ['δ = PL³ / 48EI', 'σ = Mc / I', 'θ = PL² / 16EI'],
    examples: [{ label: 'Steel channel', inputs: { P: '500', L: '1.5', shape: 'rect', b: '0.05', h: '0.1', t: '0.005' } }],
  },
  {
    id: 'stress-strain',
    category: 'engineering',
    name: 'Stress & Strain',
    tagline: 'Normal stress, strain and safety factor',
    keywords: ['stress', 'strain', 'young', 'modulus', 'safety factor', 'yield', 'axial', 'mechanical'],
    inputs: [
      { key: 'F', label: 'Force (F)', unit: 'N', default: '5000' },
      { key: 'd', label: 'Diameter / side (d)', unit: 'm', default: '0.02' },
      { key: 'shape', label: 'Shape', kind: 'select', default: 'round', options: [
        { value: 'round', label: 'Round bar' },
        { value: 'square', label: 'Square bar' },
      ] },
      { key: 'E', label: 'Young’s modulus (E)', unit: 'Pa', default: String(E_STEEL) },
      { key: 'L', label: 'Gauge length (L)', unit: 'm', default: '0.5' },
      { key: 'sy', label: 'Yield strength', unit: 'Pa', default: '250e6' },
    ],
    outputs: [
      { key: 'A', label: 'Cross-sectional area', unit: 'm²', formula: 'A' },
      { key: 'sigma', label: 'Normal stress', unit: 'Pa', formula: 'σ = F / A', emphasis: 'primary' },
      { key: 'sigmaMPa', label: 'Normal stress', unit: 'MPa', formula: 'σ / 10⁶', emphasis: 'primary' },
      { key: 'epsilon', label: 'Strain', formula: 'ε = σ / E', emphasis: 'primary' },
      { key: 'elongation', label: 'Elongation', unit: 'm', formula: 'εL' },
      { key: 'safety', label: 'Safety factor', formula: 'σy / σ', emphasis: 'primary' },
    ],
    compute: (v) => {
      const F = num(v, 'F')
      const d = num(v, 'd')
      const shape = str(v, 'shape') || 'round'
      const E = num(v, 'E') ?? E_STEEL
      const L = num(v, 'L')
      const sy = num(v, 'sy')
      if (F === undefined || d === undefined) return {}
      const A = shape === 'square' ? d * d : (Math.PI * d * d) / 4
      if (A === 0) return {}
      const sigma = F / A
      const epsilon = E !== 0 ? sigma / E : undefined
      return {
        A,
        sigma,
        sigmaMPa: sigma / 1e6,
        epsilon,
        elongation: epsilon !== undefined && L !== undefined ? epsilon * L : undefined,
        safety: sy !== undefined && sigma !== 0 ? sy / sigma : undefined,
      }
    },
    reference: ['σ = F / A', 'ε = σ / E', 'n = σy / σ'],
    examples: [{ label: '5 kN on Ø20 bar', inputs: { F: '5000', d: '0.02', shape: 'round', E: '200000000000', L: '0.5', sy: '250000000' } }],
  },
  {
    id: 'material-select',
    category: 'engineering',
    name: 'Material Properties',
    tagline: 'Young’s modulus, density and melting point',
    keywords: ['material', 'modulus', 'density', 'steel', 'aluminium', 'titanium', 'stiffness', 'properties'],
    inputs: [
      {
        key: 'material',
        label: 'Material',
        kind: 'select',
        default: 'steel',
        options: [
          { value: 'steel', label: 'Steel' },
          { value: 'alu', label: 'Aluminium' },
          { value: 'titanium', label: 'Titanium' },
          { value: 'copper', label: 'Copper' },
          { value: 'timber', label: 'Timber' },
          { value: 'concrete', label: 'Concrete' },
          { value: 'abs', label: 'ABS plastic' },
        ],
      },
      { key: 'mass', label: 'Mass', unit: 'kg' },
    ],
    outputs: [
      { key: 'E', label: 'Young’s modulus', unit: 'GPa', formula: '—', emphasis: 'primary' },
      { key: 'density', label: 'Density', unit: 'kg/m³', formula: '—', emphasis: 'primary' },
      { key: 'melting', label: 'Melting point', unit: '°C', formula: '—' },
      { key: 'volume', label: 'Volume at that mass', unit: 'm³', formula: 'm / ρ' },
      { key: 'cm3', label: 'Volume at that mass', unit: 'cm³', formula: 'm / ρ × 10⁶' },
    ],
    compute: (v) => {
      const table: Record<string, { E: number; rho: number; melt: number }> = {
        steel: { E: 200, rho: 7850, melt: 1510 },
        alu: { E: 70, rho: 2700, melt: 660 },
        titanium: { E: 114, rho: 4430, melt: 1668 },
        copper: { E: 117, rho: 8960, melt: 1085 },
        timber: { E: 11, rho: 500, melt: 0 },
        concrete: { E: 30, rho: 2400, melt: 0 },
        abs: { E: 2, rho: 1040, melt: 105 },
      }
      const key = str(v, 'material') || 'steel'
      const m = table[key]
      if (!m) return {}
      const mass = num(v, 'mass')
      return {
        E: m.E,
        density: m.rho,
        melting: m.melt || undefined,
        volume: mass !== undefined ? mass / m.rho : undefined,
        cm3: mass !== undefined ? (mass / m.rho) * 1e6 : undefined,
      }
    },
    examples: [{ label: 'Steel', inputs: { material: 'steel' } }],
  },
  {
    id: 'gear-ratio',
    category: 'engineering',
    name: 'Gear Ratio',
    tagline: 'Speed, torque and pitch geometry of a gear pair',
    keywords: ['gear', 'gearbox', 'ratio', 'teeth', 'pitch', 'module', 'transmission', 'reduction'],
    inputs: [
      { key: 'z1', label: 'Driver teeth (z₁)', default: '20', step: 1 },
      { key: 'z2', label: 'Driven teeth (z₂)', default: '50', step: 1 },
      { key: 'rpm', label: 'Input speed', unit: 'rpm', default: '1200' },
      { key: 'torque', label: 'Input torque', unit: 'N·m', default: '5' },
      { key: 'efficiency', label: 'Efficiency', unit: '%', default: '95' },
      { key: 'module', label: 'Module (m)', unit: 'mm', default: '2' },
    ],
    outputs: [
      { key: 'ratio', label: 'Gear ratio', formula: 'z₂/z₁', emphasis: 'primary' },
      { key: 'rpmOut', label: 'Output speed', formula: 'rpm/ratio', unit: 'rpm', emphasis: 'primary' },
      { key: 'torqueOut', label: 'Output torque', formula: 'τ·ratio·η', unit: 'N·m', emphasis: 'primary' },
      { key: 'pitch1', label: 'Pitch diameter (z₁)', formula: 'm·z₁', unit: 'mm' },
      { key: 'pitch2', label: 'Pitch diameter (z₂)', formula: 'm·z₂', unit: 'mm' },
      { key: 'centre', label: 'Centre distance', formula: 'm(z₁+z₂)/2', unit: 'mm' },
    ],
    compute: (v) => {
      const z1 = num(v, 'z1')
      const z2 = num(v, 'z2')
      const rpm = num(v, 'rpm')
      const torque = num(v, 'torque')
      const eff = (num(v, 'efficiency') ?? 95) / 100
      const mod = num(v, 'module') ?? 2
      if (z1 === undefined || z2 === undefined || z1 === 0) return {}
      const ratio = z2 / z1
      return {
        ratio,
        rpmOut: rpm === undefined ? undefined : rpm / ratio,
        torqueOut: torque === undefined ? undefined : torque * ratio * eff,
        pitch1: mod * z1,
        pitch2: mod * z2,
        centre: (mod * (z1 + z2)) / 2,
      }
    },
    steps: (v, r) => {
      const z1 = num(v, 'z1')
      const z2 = num(v, 'z2')
      if (z1 === undefined || z2 === undefined) return []
      const eff = (num(v, 'efficiency') ?? 95) / 100
      return [
        `Ratio: z₂ / z₁ = ${fmt(z2, 5)} / ${fmt(z1, 5)} = ${fmt(r.ratio, 5)} : 1`,
        `Output speed: n₂ = n₁ / ratio = ${fmt(num(v, 'rpm'), 5)} / ${fmt(r.ratio, 5)} = ${fmt(r.rpmOut, 5)} rpm`,
        `Output torque: τ₂ = τ₁ × ratio × η = ${fmt(num(v, 'torque'), 5)} × ${fmt(r.ratio, 5)} × ${fmt(eff, 4)} = ${fmt(r.torqueOut, 5)} N·m`,
        `Pitch diameters: d₁ = ${fmt(r.pitch1, 4)} mm, d₂ = ${fmt(r.pitch2, 4)} mm, centre distance ${fmt(r.centre, 4)} mm`,
      ]
    },
    reference: ['Ratio = z₂/z₁', 'Torque out = torque in × ratio × efficiency', 'Pitch diameter = module × teeth'],
    examples: [{ label: '20 → 50 teeth', inputs: { z1: '20', z2: '50', rpm: '1200', torque: '5', efficiency: '95', module: '2' } }],
  },
  {
    id: 'lever',
    category: 'engineering',
    name: 'Lever & Moments',
    tagline: 'Force balance about a pivot',
    keywords: ['lever', 'moment', 'torque', 'pivot', 'fulcrum', 'mechanical advantage', 'principle of moments'],
    inputs: [
      { key: 'effortArm', label: 'Effort arm', unit: 'm', default: '0.4' },
      { key: 'loadArm', label: 'Load arm', unit: 'm', default: '1.2' },
      { key: 'effort', label: 'Effort', unit: 'N', default: '30' },
    ],
    outputs: [
      { key: 'load', label: 'Load', formula: 'F·e = W·l', unit: 'N', emphasis: 'primary' },
      { key: 'advantage', label: 'Mechanical advantage', formula: 'l/e', emphasis: 'primary' },
      { key: 'effortArm', label: 'Effort arm for a given load', formula: 'W·l/F', unit: 'm' },
      { key: 'reaction', label: 'Pivot reaction', formula: 'F + W', unit: 'N' },
    ],
    compute: (v) => {
      const e = num(v, 'effortArm')
      const l = num(v, 'loadArm')
      const F = num(v, 'effort')
      if (e === undefined || l === undefined || e === 0) return {}
      const advantage = l / e
      const load = F === undefined ? undefined : F * advantage
      return {
        load,
        advantage,
        effortArm: load === undefined ? undefined : (load * l) / (F as number),
        reaction: F === undefined || load === undefined ? undefined : F + load,
      }
    },
    steps: (v, r) => {
      const e = num(v, 'effortArm')
      const l = num(v, 'loadArm')
      const F = num(v, 'effort')
      if (e === undefined || l === undefined) return []
      const out = [
        `Mechanical advantage: l / e = ${fmt(l, 5)} / ${fmt(e, 5)} = ${fmt(r.advantage, 5)}`,
        `Load: W = F × advantage = ${fmt(F, 5)} × ${fmt(r.advantage, 5)} = ${fmt(r.load, 5)} N`,
      ]
      if (r.reaction !== undefined) {
        out.push(`Pivot reaction: F + W = ${fmt(F, 5)} + ${fmt(r.load, 5)} = ${fmt(r.reaction, 5)} N`)
      }
      return out
    },
    reference: ['Principle of moments: F·e = W·l', 'Mechanical advantage = load arm / effort arm'],
    examples: [{ label: '0.4 m / 1.2 m, 30 N', inputs: { effortArm: '0.4', loadArm: '1.2', effort: '30' } }],
  },
]
