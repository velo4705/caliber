import type { Tool } from '../registry/types'
import { num, derive, list } from '../lib/values'

const RAD = Math.PI / 180

export const electricalTools: Tool[] = [
  {
    id: 'ohm',
    category: 'electrical',
    name: 'Ohm’s Law',
    tagline: 'Solve for V, I or R — plus power',
    keywords: ['ohm', 'voltage', 'current', 'resistance', "ohm's law", 'circuit', 'v=ir', 'power'],
    inputs: [
      { key: 'V', label: 'Voltage (V)', unit: 'V', default: '12' },
      { key: 'I', label: 'Current (I)', unit: 'A' },
      { key: 'R', label: 'Resistance (R)', unit: 'Ω', default: '470' },
    ],
    outputs: [
      { key: 'V', label: 'Voltage', unit: 'V', formula: 'V = I × R', emphasis: 'primary' },
      { key: 'I', label: 'Current', unit: 'A', formula: 'I = V / R', emphasis: 'primary' },
      { key: 'R', label: 'Resistance', unit: 'Ω', formula: 'R = V / I', emphasis: 'primary' },
      { key: 'P', label: 'Power', unit: 'W', formula: 'P = V × I', emphasis: 'primary' },
    ],
    compute: (v) => {
      const V = num(v, 'V')
      const I = num(v, 'I')
      const R = num(v, 'R')
      const Vo = derive(V, I !== undefined && R !== undefined ? I * R : undefined)
      const Io = derive(I, Vo !== undefined && R !== undefined && R !== 0 ? Vo / R : undefined)
      const Ro = derive(R, Vo !== undefined && Io !== undefined && Io !== 0 ? Vo / Io : undefined)
      const P = derive(
        Vo !== undefined && Io !== undefined ? Vo * Io : undefined,
        Io !== undefined && Ro !== undefined ? Io * Io * Ro : undefined,
        Vo !== undefined && Ro !== undefined && Ro !== 0 ? (Vo * Vo) / Ro : undefined,
      )
      return { V: Vo, I: Io, R: Ro, P }
    },
    steps: (v, r) => {
      const out: string[] = []
      const which = ['V', 'I', 'R'].filter((k) => num(v, k) === undefined)
      out.push(`Given ${(['V', 'I', 'R'].filter((k) => num(v, k) !== undefined).join(' and ') || 'nothing yet')}, solving for ${which.join(', ') || 'nothing new'}`)
      if (r.P !== undefined) out.push(`P = VI = ${r.P} W`)
      return out
    },
    reference: ['V = I × R', 'P = V × I = I²R = V²/R'],
    examples: [
      { label: '12 V across 470 Ω', inputs: { V: '12', R: '470' } },
      { label: '5 V at 20 mA', inputs: { V: '5', I: '0.02' } },
    ],
  },
  {
    id: 'resistor-network',
    category: 'electrical',
    name: 'Resistor Network',
    tagline: 'Combine resistors in series or parallel',
    keywords: ['resistor', 'series', 'parallel', 'combination', 'network', 'equivalent resistance'],
    inputs: [
      {
        key: 'values',
        label: 'Resistor values (Ω)',
        kind: 'number-list',
        default: '100, 220, 330',
        help: 'Enter as many as you need, comma separated.',
      },
      {
        key: 'mode',
        label: 'Arrangement',
        kind: 'select',
        default: 'series',
        options: [
          { value: 'series', label: 'Series' },
          { value: 'parallel', label: 'Parallel' },
        ],
      },
    ],
    outputs: [
      { key: 'total', label: 'Equivalent resistance', unit: 'Ω', formula: '—', emphasis: 'primary' },
      { key: 'count', label: 'Resistors', formula: '—' },
      { key: 'tolerance', label: 'Total tolerance (equal)', unit: '±%' },
    ],
    compute: (v) => {
      const r = list(v, 'values')
      const mode = num(v, 'mode') === undefined ? (String(v.mode ?? 'series')) : 'series'
      if (!r.length) return {}
      let total: number
      if (mode === 'parallel') {
        total = 1 / r.reduce((acc, x) => acc + 1 / x, 0)
      } else {
        total = r.reduce((acc, x) => acc + x, 0)
      }
      return { total, count: r.length, tolerance: (r.length * 5) }
    },
    reference: ['Series: Rt = R₁ + R₂ + …', 'Parallel: 1/Rt = 1/R₁ + 1/R₂ + …'],
    examples: [
      { label: 'Series', inputs: { values: '100, 220, 330', mode: 'series' } },
      { label: 'Parallel', inputs: { values: '100, 220, 330', mode: 'parallel' } },
    ],
  },
  {
    id: 'ac-impedance',
    category: 'electrical',
    name: 'AC Impedance (RLC)',
    tagline: 'Magnitude and phase of a series RLC circuit',
    keywords: ['impedance', 'rlc', 'ac', 'capacitor', 'inductor', 'reactance', 'phase', 'frequency'],
    inputs: [
      { key: 'r', label: 'Resistance (R)', unit: 'Ω', default: '100' },
      { key: 'l', label: 'Inductance (L)', unit: 'H', default: '0.1' },
      { key: 'c', label: 'Capacitance (C)', unit: 'F', default: '1e-5' },
      { key: 'f', label: 'Frequency (f)', unit: 'Hz', default: '1000' },
      { key: 'v', label: 'Supply voltage', unit: 'V', default: '10' },
    ],
    outputs: [
      { key: 'z', label: 'Impedance |Z|', unit: 'Ω', formula: '√(R² + (Xᴸ − Xᶜ)²)', emphasis: 'primary' },
      { key: 'phase', label: 'Phase angle', unit: '°', formula: 'atan((Xᴸ − Xᶜ) / R)', emphasis: 'primary' },
      { key: 'xl', label: 'Inductive reactance', unit: 'Ω', formula: '2πfL' },
      { key: 'xc', label: 'Capacitive reactance', unit: 'Ω', formula: '1 / (2πfC)' },
      { key: 'current', label: 'Current', unit: 'A', formula: 'V / |Z|', emphasis: 'primary' },
      { key: 'resonance', label: 'Resonant frequency', unit: 'Hz', formula: '1 / (2π√(LC))' },
    ],
    compute: (v) => {
      const r = num(v, 'r')
      const l = num(v, 'l')
      const c = num(v, 'c')
      const f = num(v, 'f')
      const vs = num(v, 'v')
      if (r === undefined || f === undefined || f === 0) return {}
      const w = 2 * Math.PI * f
      const xl = l !== undefined ? w * l : undefined
      const xc = c !== undefined && c !== 0 ? 1 / (w * c) : undefined
      const x = (xl ?? 0) - (xc ?? 0)
      const z = Math.sqrt(r * r + x * x)
      return {
        z,
        phase: Math.atan2(x, r) * RAD,
        xl,
        xc,
        current: vs !== undefined && z !== 0 ? vs / z : undefined,
        resonance: l !== undefined && c !== undefined && l > 0 && c > 0 ? 1 / (2 * Math.PI * Math.sqrt(l * c)) : undefined,
      }
    },
    reference: ['|Z| = √(R² + (Xᴸ − Xᶜ)²)', 'Xᴸ = 2πfL', 'Xᶜ = 1/(2πfC)', 'f₀ = 1/(2π√(LC))'],
    examples: [{ label: 'RLC at 1 kHz', inputs: { r: '100', l: '0.1', c: '1e-5', f: '1000', v: '10' } }],
  },
  {
    id: 'voltage-divider',
    category: 'electrical',
    name: 'Voltage Divider',
    tagline: 'Split a supply across two resistors, with or without a load',
    keywords: ['voltage divider', 'potential divider', 'resistive divider', 'tap', 'attenuator'],
    inputs: [
      { key: 'Vs', label: 'Supply voltage (Vs)', unit: 'V', default: '12' },
      { key: 'R1', label: 'Top resistor (R1)', unit: 'Ω', default: '10000' },
      { key: 'R2', label: 'Bottom resistor (R2)', unit: 'Ω', default: '4700' },
      { key: 'RL', label: 'Load resistance (RL)', unit: 'Ω' },
    ],
    outputs: [
      { key: 'vout', label: 'Output voltage', formula: 'Vs·R2/(R1+R2)', unit: 'V', emphasis: 'primary' },
      { key: 'current', label: 'Series current', formula: 'Vs/(R1+R2)', unit: 'A' },
      { key: 'pR1', label: 'Power in R1', formula: 'I²R1', unit: 'W' },
      { key: 'pR2', label: 'Power in R2', formula: 'I²R2', unit: 'W' },
      { key: 'voutLoaded', label: 'Output under load', formula: 'Vs·(R2∥RL)/(R1+R2∥RL)', unit: 'V' },
    ],
    compute: (v) => {
      const Vs = num(v, 'Vs')
      const R1 = num(v, 'R1')
      const R2 = num(v, 'R2')
      const RL = num(v, 'RL')
      if (Vs === undefined || R1 === undefined || R2 === undefined || R1 + R2 === 0) return {}
      const current = Vs / (R1 + R2)
      const rParallel = RL !== undefined && RL > 0 ? (R2 * RL) / (R2 + RL) : undefined
      return {
        vout: (Vs * R2) / (R1 + R2),
        current,
        pR1: current * current * R1,
        pR2: current * current * R2,
        voutLoaded: rParallel === undefined ? undefined : (Vs * rParallel) / (R1 + rParallel),
      }
    },
    reference: ['Vout = Vs · R2 / (R1 + R2)', 'With a load: R2∥RL replaces R2'],
    examples: [
      { label: '12 V, 10k / 4.7k', inputs: { Vs: '12', R1: '10000', R2: '4700', RL: '' } },
      { label: 'With a 1k load', inputs: { Vs: '12', R1: '10000', R2: '4700', RL: '1000' } },
    ],
  },
]
