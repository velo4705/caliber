import type { Tool } from '../registry/types'
import { num, derive, str } from '../lib/values'

const ELEMENTS: Record<string, number> = {
  H: 1.008, He: 4.003, Li: 6.94, Be: 9.012, B: 10.81, C: 12.011, N: 14.007, O: 15.999,
  F: 18.998, Ne: 20.180, Na: 22.990, Mg: 24.305, Al: 26.982, Si: 28.085, P: 30.974, S: 32.06,
  Cl: 35.45, Ar: 39.948, K: 39.098, Ca: 40.078, Sc: 44.956, Ti: 47.867, V: 50.942, Cr: 51.996,
  Mn: 54.938, Fe: 55.845, Co: 58.933, Ni: 58.693, Cu: 63.546, Zn: 65.38, Ga: 69.723, Ge: 72.630,
  As: 74.922, Se: 78.971, Br: 79.904, Kr: 83.798, Rb: 85.468, Sr: 87.62, Y: 88.906, Zr: 91.224,
  Nb: 92.906, Mo: 95.95, Ag: 107.868, Cd: 112.414, In: 114.818, Sn: 118.710, Sb: 121.760,
  Te: 127.60, I: 126.904, Xe: 131.293, Cs: 132.905, Ba: 137.327, Pt: 195.084, Au: 196.967,
  Hg: 200.592, Tl: 204.38, Pb: 207.2, Bi: 208.980, Ra: 226.025, Th: 232.038, Pa: 231.036, U: 238.029,
}

function parseNumber(f: string, i: number): [number, number] {
  let j = i
  while (j < f.length && f[j]! >= '0' && f[j]! <= '9') j++
  return [j === i ? 1 : parseInt(f.slice(i, j), 10), j]
}

export function parseFormula(f: string): Map<string, number> | undefined {
  let i = 0
  const readNumber = () => {
    const [n, j] = parseNumber(f, i)
    i = j
    return n
  }
  const group = (): Map<string, number> => {
    const acc = new Map<string, number>()
    while (i < f.length && f[i] !== ')') {
      if (f[i] === '(') {
        i++
        const inner = group()
        i++
        const mult = readNumber()
        for (const [k, v] of inner) acc.set(k, (acc.get(k) ?? 0) + v * mult)
      } else {
        const m = /^[A-Z][a-z]?/.exec(f.slice(i))
        if (!m) return acc
        const sym = m[0]
        if (!(sym in ELEMENTS)) return new Map()
        i += sym.length
        const count = readNumber()
        acc.set(sym, (acc.get(sym) ?? 0) + count)
      }
    }
    return acc
  }
  const result = group()
  if (i !== f.length) return undefined
  return result
}

const KW = 1e-14

export const chemistryTools: Tool[] = [
  {
    id: 'molar-mass',
    category: 'chemistry',
    name: 'Molar Mass',
    tagline: 'Mass of one mole from a chemical formula',
    keywords: ['molar mass', 'molecular weight', 'formula', 'mole', 'atomic mass', 'grams per mole'],
    inputs: [
      { key: 'formula', label: 'Chemical formula', kind: 'text', default: 'C6H12O6', placeholder: 'Ca(OH)2', help: 'Supports parentheses and decimal counts, e.g. CuSO4·5H2O → CuSO4(H2O)5' },
      { key: 'mass', label: 'Sample mass', unit: 'g' },
    ],
    outputs: [
      { key: 'molar', label: 'Molar mass', unit: 'g/mol', formula: 'Σ atomic masses', emphasis: 'primary' },
      { key: 'moles', label: 'Moles in sample', unit: 'mol', formula: 'm / M', emphasis: 'primary' },
      { key: 'massPerMole', label: 'Mass of one mole', unit: 'g', formula: '—' },
      { key: 'elements', label: 'Distinct elements', formula: '—' },
    ],
    compute: (v) => {
      const raw = str(v, 'formula').trim()
      if (!raw) return {}
      const counts = parseFormula(raw)
      if (!counts || counts.size === 0) return {}
      let total = 0
      for (const [sym, count] of counts) total += (ELEMENTS[sym] as number) * count
      const m = num(v, 'mass')
      return {
        molar: total,
        moles: m !== undefined && total > 0 ? m / total : undefined,
        massPerMole: total,
        elements: counts.size,
      }
    },
    steps: (_v, r) => {
      if (r.molar === undefined) return []
      return [`Summed atomic masses × counts = ${r.molar} g/mol`, `One mole therefore weighs ${r.molar} g`]
    },
    reference: ['M = Σ (atomic mass × count) for each element'],
    examples: [
      { label: 'Glucose', inputs: { formula: 'C6H12O6' } },
      { label: 'Water', inputs: { formula: 'H2O' } },
      { label: 'Copper sulfate pentahydrate', inputs: { formula: 'CuSO4(H2O)5' } },
    ],
  },
  {
    id: 'ph',
    category: 'chemistry',
    name: 'pH Calculator',
    tagline: 'Convert between pH, [H⁺] and [OH⁻]',
    keywords: ['ph', 'acid', 'base', 'hydrogen', 'hydroxide', 'alkaline', 'kw', 'h+'],
    inputs: [
      { key: 'ph', label: 'pH', default: '7' },
      { key: 'h', label: '[H⁺]', unit: 'mol/L' },
      { key: 'temp', label: 'Temperature', unit: 'K', default: '298.15', help: '25 °C uses Kw = 1.0×10⁻¹⁴' },
    ],
    outputs: [
      { key: 'ph', label: 'pH', formula: '−log₁₀[H⁺]', emphasis: 'primary' },
      { key: 'h', label: '[H⁺]', unit: 'mol/L', formula: '10⁻ᵖᴴ', emphasis: 'primary' },
      { key: 'oh', label: '[OH⁻]', unit: 'mol/L', formula: 'Kw / [H⁺]', emphasis: 'primary' },
      { key: 'poh', label: 'pOH', formula: '−log₁₀[OH⁻]' },
    ],
    compute: (v) => {
      const phIn = num(v, 'ph')
      const hIn = num(v, 'h')
      const T = num(v, 'temp') ?? 298.15
      const kw = T > 0 ? Math.pow(10, -14) * Math.pow(298.15 / T, 5800 / T - 5800 / 298.15) : KW
      const hOut = derive(hIn, phIn !== undefined ? Math.pow(10, -phIn) : undefined)
      const phOut = derive(phIn, hOut !== undefined && hOut > 0 ? -Math.log10(hOut) : undefined)
      const oh = hOut !== undefined && hOut > 0 ? kw / hOut : undefined
      return { ph: phOut, h: hOut, oh, poh: oh !== undefined && oh > 0 ? -Math.log10(oh) : undefined }
    },
    reference: ['pH = −log₁₀[H⁺]', 'Kw = [H⁺][OH⁻] = 1.0×10⁻¹⁴ at 25 °C'],
    examples: [{ label: 'pH 3', inputs: { ph: '3' } }],
  },
  {
    id: 'dilution',
    category: 'chemistry',
    name: 'Dilution (C₁V₁ = C₂V₂)',
    tagline: 'Solve any one term of the dilution equation',
    keywords: ['dilution', 'm1v1', 'concentration', 'volume', 'solution', 'serial dilution'],
    inputs: [
      { key: 'c1', label: 'C₁ (stock)', unit: 'mol/L', default: '1' },
      { key: 'v1', label: 'V₁ (stock volume)', unit: 'L', default: '0.1' },
      { key: 'c2', label: 'C₂ (final)', unit: 'mol/L', default: '0.1' },
      { key: 'v2', label: 'V₂ (final volume)', unit: 'L' },
    ],
    outputs: [
      { key: 'c1', label: 'C₁', unit: 'mol/L', formula: 'C₂V₂ / V₁', emphasis: 'primary' },
      { key: 'v1', label: 'V₁', unit: 'L', formula: 'C₂V₂ / C₁', emphasis: 'primary' },
      { key: 'c2', label: 'C₂', unit: 'mol/L', formula: 'C₁V₁ / V₂', emphasis: 'primary' },
      { key: 'v2', label: 'V₂', unit: 'L', formula: 'C₁V₁ / C₂', emphasis: 'primary' },
    ],
    compute: (v) => {
      const c1 = num(v, 'c1')
      const v1 = num(v, 'v1')
      const c2 = num(v, 'c2')
      const v2 = num(v, 'v2')
      const c1o = derive(c1, c2 !== undefined && v1 !== undefined && v1 !== 0 && v2 !== undefined ? (c2 * v2) / v1 : undefined)
      const v1o = derive(v1, c1 !== undefined && c1 !== 0 && c2 !== undefined && v2 !== undefined ? (c2 * v2) / c1 : undefined)
      const c2o = derive(c2, c1 !== undefined && v1 !== undefined && v2 !== undefined && v2 !== 0 ? (c1 * v1) / v2 : undefined)
      const v2o = derive(v2, c1 !== undefined && v1 !== undefined && c2 !== undefined && c2 !== 0 ? (c1 * v1) / c2 : undefined)
      return { c1: c1o, v1: v1o, c2: c2o, v2: v2o }
    },
    steps: (v, r) => {
      if (r.c2 === undefined && r.v2 === undefined) return []
      return [`Moles conserved: C₁V₁ = ${num(v, 'c1') ?? '?'} × ${num(v, 'v1') ?? '?'}`, `C₂V₂ = ${r.c2 ?? '?'} × ${r.v2 ?? '?'}`]
    },
    reference: ['C₁V₁ = C₂V₂'],
    examples: [
      { label: '1 M → 0.1 M', inputs: { c1: '1', v1: '0.1', c2: '0.1' } },
      { label: 'Serial 1:10 dilution', inputs: { c1: '1', v1: '1', c2: '0.1' } },
    ],
  },
  {
    id: 'ideal-gas',
    category: 'chemistry',
    name: 'Ideal Gas Law',
    tagline: 'Solve any term of PV = nRT',
    keywords: ['ideal gas', 'pv=nrt', 'moles', 'pressure', 'volume', 'temperature', 'gas law'],
    inputs: [
      { key: 'p', label: 'Pressure (P)', unit: 'atm' },
      { key: 'v', label: 'Volume (V)', unit: 'L', default: '5' },
      { key: 'n', label: 'Moles (n)', unit: 'mol', default: '2' },
      { key: 't', label: 'Temperature (T)', unit: 'K', default: '300' },
    ],
    outputs: [
      { key: 'p', label: 'Pressure', unit: 'atm', formula: 'P = nRT / V', emphasis: 'primary' },
      { key: 'v', label: 'Volume', unit: 'L', formula: 'V = nRT / P', emphasis: 'primary' },
      { key: 'n', label: 'Moles', unit: 'mol', formula: 'n = PV / RT', emphasis: 'primary' },
      { key: 't', label: 'Temperature', unit: 'K', formula: 'T = PV / nR', emphasis: 'primary' },
      { key: 'tC', label: 'Temperature', unit: '°C', formula: 'T − 273.15' },
    ],
    compute: (v) => {
      const R = 0.082057
      const p = num(v, 'p')
      const vol = num(v, 'v')
      const n = num(v, 'n')
      const t = num(v, 't')
      const tOut = derive(t, p !== undefined && vol !== undefined && n !== undefined && n !== 0 ? (p * vol) / (n * R) : undefined)
      const pOut = derive(p, vol !== undefined && vol !== 0 && n !== undefined && tOut !== undefined ? (n * R * tOut) / vol : undefined)
      const vOut = derive(vol, pOut !== undefined && pOut !== 0 && n !== undefined && tOut !== undefined ? (n * R * tOut) / pOut : undefined)
      const nOut = derive(n, pOut !== undefined && vOut !== undefined && tOut !== undefined && tOut !== 0 ? (pOut * vOut) / (R * tOut) : undefined)
      return { p: pOut, v: vOut, n: nOut, t: tOut, tC: tOut !== undefined ? tOut - 273.15 : undefined }
    },
    reference: ['PV = nRT', 'R = 0.082057 L·atm / (mol·K)'],
    examples: [{ label: '2 mol at 300 K, 5 L', inputs: { v: '5', n: '2', t: '300' } }],
  },
]