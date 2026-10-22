import type { Tool } from '../registry/types'
import { num } from '../lib/values'
import { Parser } from '../lib/expr'
import { formatNumber as fmt } from '../lib/format'

export const computingTools: Tool[] = [
  {
    id: 'number-base',
    category: 'computing',
    name: 'Number Base Converter',
    tagline: 'Decimal, binary, octal and hexadecimal',
    keywords: ['binary', 'hex', 'hexadecimal', 'octal', 'base', 'convert', 'radix', '0xff'],
    inputs: [
      { key: 'value', label: 'Decimal value', default: '255' },
      { key: 'width', label: 'Bit width', default: '8', min: 4, max: 64, step: 1, help: 'Used for the two’s complement view of negatives.' },
    ],
    outputs: [
      { key: 'hex', label: 'Hexadecimal', formula: 'base 16', emphasis: 'primary' },
      { key: 'bin', label: 'Binary', formula: 'base 2', emphasis: 'primary' },
      { key: 'oct', label: 'Octal', formula: 'base 8', emphasis: 'primary' },
      { key: 'twos', label: 'Two’s complement', formula: 'n-bit', emphasis: 'primary' },
      { key: 'ascii', label: 'ASCII character', formula: '—' },
    ],
    compute: (v) => {
      const n = num(v, 'value')
      const width = Math.round(num(v, 'width') ?? 8)
      if (n === undefined || !Number.isInteger(n)) return {}
      const w = Math.min(Math.max(width, 4), 64)
      const unsigned = ((n % 2 ** w) + 2 ** w) % 2 ** w
      return {
        hex: (unsigned >>> 0).toString(16).toUpperCase().padStart(Math.ceil(w / 4), '0'),
        bin: unsigned.toString(2).padStart(w, '0'),
        oct: unsigned.toString(8),
        twos: unsigned.toString(2).padStart(w, '0'),
        ascii: n >= 32 && n <= 126 ? String.fromCharCode(n) : undefined,
      }
    },
    steps: (v, r) => {
      const n = num(v, 'value')
      if (n === undefined) return []
      return [
        `Decimal: ${fmt(n, 8)}`,
        r.bin !== undefined ? `Binary ${r.bin}` : '',
        r.hex !== undefined ? `Hex 0x${r.hex}` : '',
      ].filter(Boolean)
    },
    reference: ['radix conversion via repeated division by the target base'],
    examples: [{ label: '255', inputs: { value: '255', width: '8' } }],
  },
  {
    id: 'bitwise',
    category: 'computing',
    name: 'Bitwise Operations',
    tagline: 'AND, OR, XOR, NOT and shifts',
    keywords: ['bitwise', 'and', 'or', 'xor', 'not', 'shift', 'mask', 'boolean', 'binary'],
    inputs: [
      { key: 'a', label: 'a', default: '12', step: 1 },
      { key: 'b', label: 'b', default: '10', step: 1 },
    ],
    outputs: [
      { key: 'and', label: 'a AND b', formula: 'a & b', emphasis: 'primary' },
      { key: 'or', label: 'a OR b', formula: 'a | b', emphasis: 'primary' },
      { key: 'xor', label: 'a XOR b', formula: 'a ^ b', emphasis: 'primary' },
      { key: 'not', label: 'NOT a', formula: '~a', emphasis: 'primary' },
      { key: 'shl', label: 'a << b', formula: 'shift left', emphasis: 'primary' },
      { key: 'shr', label: 'a >> b', formula: 'shift right', emphasis: 'primary' },
    ],
    compute: (v) => {
      const a = num(v, 'a')
      const b = num(v, 'b')
      if (a === undefined || b === undefined) return {}
      return {
        and: a & b,
        or: a | b,
        xor: a ^ b,
        not: ~a,
        shl: b < 0 || b > 31 ? undefined : a << b,
        shr: b < 0 || b > 31 ? undefined : a >> b,
      }
    },
    reference: ['a AND b', 'a OR b', 'a XOR b', 'NOT a', 'a << b (shift left)', 'a >> b (shift right)'],
    examples: [{ label: '12 and 10', inputs: { a: '12', b: '10' } }],
  },
  {
    id: 'expression-eval',
    category: 'computing',
    name: 'Expression Evaluator',
    tagline: 'Test the Caliber expression engine',
    keywords: ['expression', 'parser', 'evaluate', 'formula', 'test', 'syntax', 'math'],
    inputs: [
      {
        key: 'expr',
        label: 'Expression',
        kind: 'expression',
        default: 'sin(45) * sqrt(2) + 2^10',
        placeholder: 'e.g. (3 + 4) * 2 / ln(e)',
        help: 'Supports + - * / % ^, factorial, constants (pi, e, tau) and 40+ functions.',
      },
    ],
    outputs: [
      { key: 'value', label: 'Result', formula: '—', emphasis: 'primary' },
      { key: 'radians', label: 'Result (radians)', formula: '—' },
      { key: 'error', label: 'Error', formula: '—', optional: true },
    ],
    compute: (v) => {
      const src = typeof v.expr === 'string' ? v.expr.trim() : ''
      if (!src) return { value: undefined, radians: undefined, error: undefined }
      try {
        return {
          value: new Parser('rad').evaluate(src),
          radians: new Parser('deg').evaluate(src),
          error: undefined,
        }
      } catch (e) {
        return { value: undefined, radians: undefined, error: e instanceof Error ? e.message : 'Invalid expression' }
      }
    },
    reference: ['sin cos tan asin acos atan sinh cosh tanh', 'ln log log10 log2 sqrt cbrt exp abs sign', 'min max pow hypot gcd lcm mod atan2'],
    examples: [
      { label: 'Trig', inputs: { expr: 'sin(30)^2 + cos(30)^2' } },
      { label: 'Factorial', inputs: { expr: '10! / 8!' } },
      { label: 'Scientific', inputs: { expr: '6.022e23 * 1.602e-19' } },
    ],
  },
]