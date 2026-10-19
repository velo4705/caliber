import type { Tool } from '../registry/types'
import { num } from '../lib/values'
import { formatNumber as fmt } from '../lib/format'

export const mathTools: Tool[] = [
  {
    id: 'quadratic',
    category: 'math',
    name: 'Quadratic Roots',
    tagline: 'Solve ax² + bx + c = 0',
    keywords: ['quadratic', 'polynomial', 'roots', 'discriminant', 'parabola', 'factorise'],
    inputs: [
      { key: 'a', label: 'a', default: '1', step: 0.1 },
      { key: 'b', label: 'b', default: '-3', step: 0.1 },
      { key: 'c', label: 'c', default: '2', step: 0.1 },
    ],
    outputs: [
      { key: 'd', label: 'Discriminant', formula: 'b² − 4ac', needs: 'a, b and c', emphasis: 'primary' },
      { key: 'x1', label: 'Root x₁', formula: '(−b + √d) / 2a', needs: 'a, b and c' },
      { key: 'x2', label: 'Root x₂', formula: '(−b − √d) / 2a', needs: 'a, b and c' },
      { key: 'realPart', label: 'Root (real)', formula: '−b / 2a', needs: 'a and b' },
      { key: 'imagPart', label: 'Root (imaginary)', formula: '√|d| / 2a', needs: 'a and b' },
      { key: 'vertexY', label: 'Vertex y', formula: 'f(−b / 2a)', needs: 'a, b and c' },
    ],
    compute: (v) => {
      const a = num(v, 'a')
      const b = num(v, 'b')
      const c = num(v, 'c')
      if (a === undefined || a === 0) return {}
      const d = b !== undefined && c !== undefined ? b * b - 4 * a * c : undefined
      const realPart = b !== undefined ? -b / (2 * a) : undefined
      const imagPart = d !== undefined ? Math.sqrt(Math.abs(d)) / (2 * a) : undefined
      const hasRealRoots = d !== undefined && d >= 0
      return {
        d,
        x1: hasRealRoots ? ((b as number) + Math.sqrt(d as number)) / (2 * a) : undefined,
        x2: hasRealRoots ? (-(b as number) - Math.sqrt(d as number)) / (2 * a) : undefined,
        realPart,
        imagPart,
        vertexY:
          realPart !== undefined && c !== undefined
            ? a * realPart * realPart + (b as number) * realPart + c
            : undefined,
      }
    },
    steps: (v, r) => {
      const a = num(v, 'a')
      const b = num(v, 'b')
      const c = num(v, 'c')
      const s: string[] = []
      s.push(`Equation: ${fmt(a)}x² ${num(v, 'b')! < 0 ? '−' : '+'} ${fmt(Math.abs(num(v, 'b') ?? 0))}x ${num(v, 'c')! < 0 ? '−' : '+'} ${fmt(Math.abs(num(v, 'c') ?? 0))} = 0`)
      if (r.d !== undefined) s.push(`Discriminant d = b² − 4ac = ${fmt(b)}² − 4(${fmt(a)})(${fmt(c)}) = ${fmt(r.d)}`)
      const dNum = Number(r.d)
      if (dNum > 0) s.push('d > 0, so two distinct real roots')
      if (dNum === 0) s.push('d = 0, so one repeated real root')
      if (dNum < 0) s.push('d < 0, so two complex conjugate roots')
      if (r.x1 !== undefined) s.push(`x = (−b ± √d) / 2a = (${fmt(-(b ?? 0))} ± ${fmt(Math.sqrt(dNum))}) / ${fmt(2 * (a ?? 0))}`)
      return s
    },
    reference: ['ax² + bx + c = 0', 'x = [−b ± √(b² − 4ac)] / 2a', 'd = b² − 4ac'],
    examples: [
      { label: 'Two roots', inputs: { a: '1', b: '-3', c: '2' } },
      { label: 'Repeated root', inputs: { a: '1', b: '-2', c: '1' } },
      { label: 'Complex roots', inputs: { a: '1', b: '0', c: '1' } },
    ],
  },
  {
    id: 'divisors',
    category: 'math',
    name: 'Divisors & Factors',
    tagline: 'List divisors of an integer',
    keywords: ['divisor', 'factor', 'factors', 'prime', 'aliquot', 'sigma', 'tau'],
    inputs: [{ key: 'n', label: 'Integer n', default: '60', min: 1, step: 1 }],
    outputs: [
      { key: 'count', label: 'Number of divisors (τ)', formula: 'τ(n)', emphasis: 'primary' },
      { key: 'sum', label: 'Sum of divisors (σ)', formula: 'σ(n)' },
      { key: 'largest', label: 'Largest proper divisor', formula: 'n / p' },
      { key: 'factorial', label: 'n!', formula: 'n!' },
      { key: 'prime', label: 'Is prime', formula: 'trial division' },
    ],
    compute: (v) => {
      const n = num(v, 'n')
      if (n === undefined || n < 1 || !Number.isInteger(n)) return {}
      const divs: number[] = []
      for (let i = 1; i * i <= n; i++) {
        if (n % i === 0) {
          divs.push(i)
          if (i !== n / i) divs.push(n / i)
        }
      }
      divs.sort((a, b) => a - b)
      let fact = 1
      for (let i = 2; i <= n && i < 171; i++) fact *= i
      return {
        count: divs.length,
        sum: divs.reduce((a, b) => a + b, 0),
        largest: n === 1 ? undefined : divs[divs.length - 2],
        factorial: n < 171 ? fact : undefined,
        prime: divs.length === 2 ? 1 : 0,
      }
    },
    reference: ['τ(n) = count of positive divisors', 'σ(n) = sum of positive divisors'],
    examples: [{ label: 'n = 60', inputs: { n: '60' } }],
  },
  {
    id: 'combinations',
    category: 'math',
    name: 'Permutations & Combinations',
    tagline: 'nCr, nPr and factorials',
    keywords: ['combination', 'permutation', 'ncr', 'npr', 'factorial', 'arrangement', 'binomial'],
    inputs: [
      { key: 'n', label: 'n', default: '10', min: 0, step: 1 },
      { key: 'r', label: 'r', default: '3', min: 0, step: 1 },
    ],
    outputs: [
      { key: 'ncr', label: 'Combinations nCr', formula: 'n! / [r!(n−r)!]', emphasis: 'primary' },
      { key: 'npr', label: 'Permutations nPr', formula: 'n! / (n−r)!', emphasis: 'primary' },
      { key: 'factN', label: 'n!', formula: 'n!' },
      { key: 'factR', label: 'r!', formula: 'r!' },
    ],
    compute: (v) => {
      const n = num(v, 'n')
      const r = num(v, 'r')
      if (n === undefined || r === undefined || n < 0 || r < 0 || r > n || !Number.isInteger(n) || !Number.isInteger(r)) return {}
      const fact = (k: number) => {
        let o = 1
        for (let i = 2; i <= k; i++) o *= i
        return o
      }
      const fn = fact(n)
      const fr = fact(r)
      const fp = fact(n - r)
      return { ncr: fn / (fr * fp), npr: fn / fp, factN: fn, factR: fr }
    },
    reference: ['nCr = n! / [r!(n−r)!]', 'nPr = n! / (n−r)!'],
    examples: [{ label: 'Lottery: 6 from 49', inputs: { n: '49', r: '6' } }],
  },
  {
    id: 'percent-change',
    category: 'math',
    name: 'Percentage Change',
    tagline: 'Forward and reverse percent change',
    keywords: ['percent', 'percentage', 'change', 'increase', 'decrease', 'growth'],
    inputs: [
      { key: 'from', label: 'From', default: '80' },
      { key: 'to', label: 'To', default: '100' },
    ],
    outputs: [
      { key: 'pct', label: 'Percent change', formula: '(to − from) / from × 100', emphasis: 'primary' },
      { key: 'factor', label: 'Multiplier', formula: 'to / from' },
      { key: 'delta', label: 'Difference', formula: 'to − from' },
      { key: 'reverse', label: 'Original if target is 100%', formula: 'to / (1 + pct/100)' },
    ],
    compute: (v) => {
      const from = num(v, 'from')
      const to = num(v, 'to')
      if (from === undefined || to === undefined || from === 0) return {}
      const pct = ((to - from) / from) * 100
      return { pct, factor: to / from, delta: to - from, reverse: to / (1 + pct / 100) }
    },
    reference: ['% change = (new − old) / old × 100'],
    examples: [{ label: '80 → 100', inputs: { from: '80', to: '100' } }],
  },
  {
    id: 'logarithm',
    category: 'math',
    name: 'Logarithm',
    tagline: 'log, ln and inverse logs',
    keywords: ['log', 'ln', 'logarithm', 'natural log', 'exponent', 'base'],
    inputs: [
      { key: 'x', label: 'Value', default: '1000' },
      { key: 'base', label: 'Base', default: '10', min: 0.0001, step: 0.1 },
    ],
    outputs: [
      { key: 'log', label: `log base b`, formula: 'log_b(x)', emphasis: 'primary' },
      { key: 'ln', label: 'Natural log', formula: 'ln(x)' },
      { key: 'log10', label: 'Common log', formula: 'log₁₀(x)' },
      { key: 'inverse', label: 'b to the power log', formula: 'b^(log_b x)' },
    ],
    compute: (v) => {
      const x = num(v, 'x')
      const base = num(v, 'base') ?? 10
      if (x === undefined || x <= 0 || base <= 0) return {}
      const lg = Math.log(x) / Math.log(base)
      return { log: lg, ln: Math.log(x), log10: Math.log10(x), inverse: Math.pow(base, lg) }
    },
    reference: ['log_b(x) = ln(x) / ln(b)'],
    examples: [{ label: 'log₂(1024)', inputs: { x: '1024', base: '2' } }],
  },
]