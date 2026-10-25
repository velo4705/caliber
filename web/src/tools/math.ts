import type { Tool } from '../registry/types'
import { num, str } from '../lib/values'
import { formatNumber as fmt } from '../lib/format'
import {
  addFraction,
  divideFraction,
  euclidSteps,
  formatFraction,
  fractionToDecimal,
  gcd,
  lcm,
  modInverse,
  multiplyFraction,
  parseFraction,
  powMod,
  subtractFraction,
  totient,
  type Fraction,
} from '../lib/num'
import { formatMatrix, solveLinearSystem } from '../lib/linalg'

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
      { key: 'd', label: 'Discriminant', formula: 'b² - 4ac', needs: 'a, b and c', emphasis: 'primary' },
      { key: 'x1', label: 'Root x₁', formula: '(-b + √d) / 2a', needs: 'a, b and c' },
      { key: 'x2', label: 'Root x₂', formula: '(-b - √d) / 2a', needs: 'a, b and c' },
      { key: 'realPart', label: 'Root (real)', formula: '-b / 2a', needs: 'a and b' },
      { key: 'imagPart', label: 'Root (imaginary)', formula: '√|d| / 2a', needs: 'a and b' },
      { key: 'vertexY', label: 'Vertex y', formula: 'f(-b / 2a)', needs: 'a, b and c' },
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
        x1: hasRealRoots ? (-(b as number) + Math.sqrt(d as number)) / (2 * a) : undefined,
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
      const a = num(v, 'a') ?? 0
      const b = num(v, 'b') ?? 0
      const c = num(v, 'c') ?? 0
      const s: string[] = []
      const aTerm = a === 1 ? '' : a === -1 ? '-' : `${fmt(a)}`
      const bTerm = b === 0 ? '' : ` ${b < 0 ? '- ' : '+ '}${fmt(Math.abs(b))}x`
      const cTerm = c === 0 ? '' : ` ${c < 0 ? '- ' : '+ '}${fmt(Math.abs(c))}`
      s.push(`Equation: ${aTerm}x²${bTerm}${cTerm} = 0`)
      if (r.d !== undefined) {
        s.push(
          `Discriminant d = b² - 4ac = (${fmt(b)})² - 4 × ${fmt(a)} × ${fmt(c)} = ${fmt(b * b)} - ${fmt(4 * a * c)} = ${fmt(r.d)}`,
        )
      }
      const dNum = Number(r.d)
      if (dNum > 0) s.push('d > 0, so two distinct real roots')
      if (dNum === 0) s.push('d = 0, so one repeated real root')
      if (dNum < 0) s.push('d < 0, so two complex conjugate roots')
      if (r.x1 !== undefined) {
        const root = fmt(Math.sqrt(dNum))
        s.push(`x₁ = (-b + √d) / 2a = (${fmt(-b)} + ${root}) / ${fmt(2 * a)} = ${fmt(r.x1)}`)
        s.push(`x₂ = (-b - √d) / 2a = (${fmt(-b)} - ${root}) / ${fmt(2 * a)} = ${fmt(r.x2)}`)
      }
      if (dNum < 0 && r.realPart !== undefined) {
        s.push(`Complex roots: x = ${fmt(r.realPart)} ± i·${fmt(r.imagPart)}`)
      }
      return s
    },
    reference: ['ax² + bx + c = 0', 'x = [-b ± √(b² - 4ac)] / 2a', 'd = b² - 4ac'],
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
      { key: 'ncr', label: 'Combinations nCr', formula: 'n! / [r!(n-r)!]', emphasis: 'primary' },
      { key: 'npr', label: 'Permutations nPr', formula: 'n! / (n-r)!', emphasis: 'primary' },
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
    reference: ['nCr = n! / [r!(n-r)!]', 'nPr = n! / (n-r)!'],
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
      { key: 'pct', label: 'Percent change', formula: '(to - from) / from × 100', emphasis: 'primary' },
      { key: 'factor', label: 'Multiplier', formula: 'to / from' },
      { key: 'delta', label: 'Difference', formula: 'to - from' },
      { key: 'reverse', label: 'Original if target is 100%', formula: 'to / (1 + pct/100)' },
    ],
    compute: (v) => {
      const from = num(v, 'from')
      const to = num(v, 'to')
      if (from === undefined || to === undefined || from === 0) return {}
      const pct = ((to - from) / from) * 100
      return { pct, factor: to / from, delta: to - from, reverse: to / (1 + pct / 100) }
    },
    reference: ['% change = (new - old) / old × 100'],
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
  {
    id: 'linear-system',
    category: 'math',
    name: 'Linear System Solver',
    tagline: 'Solve simultaneous equations by Gaussian elimination',
    keywords: ['linear', 'system', 'simultaneous', 'gaussian', 'elimination', 'matrix', 'equations', 'cramer'],
    inputs: [
      {
        key: 'matrix',
        label: 'Augmented matrix',
        kind: 'matrix',
        default: '[[2,1,3],[1,-1,1]]',
        rows: 2,
        cols: 3,
        help: 'Enter as JSON array: [[a,b,c],[d,e,f]] where each inner array is a row (coefficients + RHS).',
        placeholder: '[[2,1,3],[1,-1,1]]',
      },
    ],
    outputs: [
      { key: 'solution', label: 'Solution', formula: 'Ax = b', emphasis: 'primary' },
      { key: 'determinant', label: 'Determinant', formula: 'det(A)' },
      { key: 'rank', label: 'Rank', formula: '—' },
      { key: 'rref', label: 'Reduced row echelon form', formula: 'rref([A|b])' },
    ],
    compute: (v) => {
      const raw = str(v, 'matrix')
      let r: number[][]
      try {
        r = JSON.parse(raw)
      } catch {
        return {}
      }
      if (r.length < 2) return {}
      const n = r.length
      if (!r.every((row) => row.length === n + 1)) return {}
      const a = r.map((row) => row.slice(0, n))
      const b = r.map((row) => row[n] as number)
      const solution = solveLinearSystem(a, b)
      if (!solution) return {}
      return {
        solution: solution.singular
          ? 'no unique solution'
          : solution.x.map((v2) => fmt(v2, 6)).join(',  '),
        determinant: solution.determinant,
        rank: solution.rank,
        rref: formatMatrix(solution.rref),
      }
    },
    steps: (v) => {
      const raw = str(v, 'matrix')
      let r: number[][]
      try {
        r = JSON.parse(raw)
      } catch {
        return []
      }
      if (r.length < 2) return []
      const n = r.length
      const header = r.map((row) => row.join('  ')).join('\n')
      return [
        `Augmented matrix [A|b] (${n}×${n + 1}):\n${header}`,
        'Solve by Gaussian elimination with partial pivoting, then back-substitute.',
      ]
    },
    reference: ['Ax = b', 'Solved by Gaussian elimination with partial pivoting'],
    examples: [
      { label: '2×2', inputs: { matrix: '[[2,1,3],[1,-1,1]]' } },
      { label: '3×3', inputs: { matrix: '[[1,1,1,6],[2,3,4,15],[1,0,1,3]]' } },
    ],
  },
  {
    id: 'gcd-lcm',
    category: 'math',
    name: 'GCD & LCM',
    tagline: 'Greatest common divisor and lowest common multiple',
    keywords: ['gcd', 'lcm', 'greatest common divisor', 'lowest common multiple', 'euclid', 'coprime'],
    inputs: [
      { key: 'a', label: 'a', default: '12', step: 1 },
      { key: 'b', label: 'b', default: '18', step: 1 },
    ],
    outputs: [
      { key: 'gcd', label: 'GCD', formula: 'gcd(a, b)', emphasis: 'primary' },
      { key: 'lcm', label: 'LCM', formula: 'lcm(a, b)', emphasis: 'primary' },
      { key: 'coprime', label: 'Coprime', formula: 'gcd = 1' },
      { key: 'euclid', label: 'Euclid steps', formula: '—' },
    ],
    compute: (v) => {
      const a = num(v, 'a')
      const b = num(v, 'b')
      if (a === undefined || b === undefined || !Number.isInteger(a) || !Number.isInteger(b)) return {}
      const g = gcd(a, b)
      return {
        gcd: g,
        lcm: lcm(a, b),
        coprime: g === 1 ? 1 : 0,
        euclid: euclidSteps(a, b).join('\n'),
      }
    },
    reference: ['gcd via the Euclidean algorithm', 'lcm = |a·b| / gcd(a, b)'],
    examples: [
      { label: '12 and 18', inputs: { a: '12', b: '18' } },
      { label: 'Coprime', inputs: { a: '13', b: '17' } },
    ],
  },
  {
    id: 'modular-arithmetic',
    category: 'math',
    name: 'Modular Arithmetic',
    tagline: 'Modular exponentiation, inverses and Euler’s totient',
    keywords: ['modular', 'mod', 'modulo', 'modpow', 'inverse', 'totient', 'congruence', 'cryptography', 'prime'],
    inputs: [
      { key: 'a', label: 'Base (a)', default: '3', step: 1 },
      { key: 'b', label: 'Exponent (b)', default: '8', step: 1 },
      { key: 'm', label: 'Modulus (m)', default: '13', step: 1 },
    ],
    outputs: [
      { key: 'pow', label: 'aᵇ mod m', formula: 'powmod(a, b, m)', emphasis: 'primary' },
      { key: 'aMod', label: 'a mod m', formula: 'a - m·⌊a/m⌋' },
      { key: 'inverse', label: 'Modular inverse of a', formula: 'a⁻¹ mod m' },
      { key: 'totient', label: 'Euler’s totient φ(m)', formula: 'φ(m)' },
      { key: 'gcd', label: 'gcd(a, m)', formula: '—' },
    ],
    compute: (v) => {
      const a = num(v, 'a')
      const b = num(v, 'b')
      const m = num(v, 'm')
      if (a === undefined || b === undefined || m === undefined || m <= 0) return {}
      const inverse = modInverse(a, m)
      return {
        pow: powMod(a, b, m),
        aMod: ((a % m) + m) % m,
        inverse: inverse === undefined ? 'does not exist' : inverse,
        totient: totient(m),
        gcd: gcd(a, m),
      }
    },
    steps: (v) => {
      const a = num(v, 'a')
      const b = num(v, 'b')
      const m = num(v, 'm')
      if (a === undefined || b === undefined || m === undefined) return []
      const out = [`Reduce the base: ${fmt(a)} mod ${fmt(m)} = ${fmt(((a % m) + m) % m)}`]
      out.push(`Repeated squaring: ${fmt(a)}^${fmt(b)} mod ${fmt(m)} = ${fmt(powMod(a, b, m))}`)
      const inv = modInverse(a, m)
      out.push(
        inv === undefined
          ? `No modular inverse: gcd(${fmt(a)}, ${fmt(m)}) = ${fmt(gcd(a, m))}, not 1`
          : `Modular inverse: ${fmt(a)}⁻¹ mod ${fmt(m)} = ${fmt(inv)}, since ${fmt(a)} × ${fmt(inv)} ≡ 1 (mod ${fmt(m)})`,
      )
      out.push(`Euler's totient: φ(${fmt(m)}) = ${fmt(totient(m))}`)
      return out
    },
    reference: ['aᵇ mod m by repeated squaring', 'a⁻¹ mod m from the extended Euclidean algorithm', 'φ(m) = m·∏(1 - 1/p)'],
    examples: [
      { label: '3⁸ mod 13', inputs: { a: '3', b: '8', m: '13' } },
      { label: 'Inverse of 3 mod 11', inputs: { a: '3', b: '1', m: '11' } },
    ],
  },
  {
    id: 'fraction-arithmetic',
    category: 'math',
    name: 'Fraction Arithmetic',
    tagline: 'Add, subtract, multiply and divide fractions',
    keywords: ['fraction', 'rational', 'numerator', 'denominator', 'mixed number', 'simplify', 'reduce'],
    inputs: [
      { key: 'x', label: 'First fraction', kind: 'text', default: '3/4', placeholder: '3/4 or 2 1/2' },
      { key: 'y', label: 'Second fraction', kind: 'text', default: '5/6', placeholder: '5/6' },
      {
        key: 'op',
        label: 'Operation',
        kind: 'select',
        default: '+',
        options: [
          { value: '+', label: 'Add' },
          { value: '-', label: 'Subtract' },
          { value: '*', label: 'Multiply' },
          { value: '/', label: 'Divide' },
        ],
      },
    ],
    outputs: [
      { key: 'result', label: 'Result', formula: '—', emphasis: 'primary' },
      { key: 'decimal', label: 'Decimal', formula: 'n / d' },
      { key: 'numerator', label: 'Numerator', formula: '—' },
      { key: 'denominator', label: 'Denominator', formula: '—' },
    ],
    compute: (v) => {
      const fx = parseFraction(str(v, 'x'))
      const fy = parseFraction(str(v, 'y'))
      const op = str(v, 'op') || '+'
      if (!fx || !fy) return {}
      let result: Fraction | undefined
      if (op === '+') result = addFraction(fx, fy)
      else if (op === '-') result = subtractFraction(fx, fy)
      else if (op === '*') result = multiplyFraction(fx, fy)
      else result = divideFraction(fx, fy)
      if (!result) return {}
      return {
        result: formatFraction(result),
        decimal: fractionToDecimal(result),
        numerator: result.n,
        denominator: result.d,
      }
    },
    steps: (v) => {
      const fx = parseFraction(str(v, 'x'))
      const fy = parseFraction(str(v, 'y'))
      const op = str(v, 'op') || '+'
      if (!fx || !fy) return []
      const out = [`First: ${formatFraction(fx)} = ${fmt(fractionToDecimal(fx), 8)}`]
      out.push(`Second: ${formatFraction(fy)} = ${fmt(fractionToDecimal(fy), 8)}`)
      if (op === '+') {
        out.push(`Common denominator: ${fx.d} × ${fy.d} = ${fx.d * fy.d}`)
        out.push(`Numerator: (${fx.n} × ${fy.d}) + (${fy.n} × ${fx.d}) = ${fx.n * fy.d + fy.n * fx.d}`)
      } else if (op === '-') {
        out.push(`Common denominator: ${fx.d} × ${fy.d} = ${fx.d * fy.d}`)
        out.push(`Numerator: (${fx.n} × ${fy.d}) - (${fy.n} × ${fx.d}) = ${fx.n * fy.d - fy.n * fx.d}`)
      } else if (op === '*') {
        out.push(`Numerator: ${fx.n} × ${fy.n} = ${fx.n * fy.n}`)
        out.push(`Denominator: ${fx.d} × ${fy.d} = ${fx.d * fy.d}`)
      } else {
        out.push(`Divide by flipping: ${fx.n}/${fx.d} × ${fy.d}/${fy.n}`)
      }
      const result = op === '+' ? addFraction(fx, fy) : op === '-' ? subtractFraction(fx, fy) : op === '*' ? multiplyFraction(fx, fy) : divideFraction(fx, fy)
      if (result) {
        out.push(`Reduced result: ${formatFraction(result)} = ${fmt(fractionToDecimal(result), 8)}`)
      }
      return out
    },
    reference: ['a/b + c/d = (ad + bc)/bd', 'a/b ÷ c/d = (a·d)/(b·c)'],
    examples: [
      { label: '3/4 + 5/6', inputs: { x: '3/4', y: '5/6', op: '+' } },
      { label: '2 1/2 × 4/5', inputs: { x: '2 1/2', y: '4/5', op: '*' } },
    ],
  },
]
