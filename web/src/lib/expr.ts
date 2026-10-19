export type AngleMode = 'deg' | 'rad' | 'grad'

export class ExprError extends Error {
  constructor(
    message: string,
    readonly position: number,
  ) {
    super(message)
    this.name = 'ExprError'
  }
}

const CONSTANTS: Record<string, number> = {
  pi: Math.PI,
  π: Math.PI,
  e: Math.E,
  tau: Math.PI * 2,
  phi: (1 + Math.sqrt(5)) / 2,
}

const LANCZOS = [
  0.99999999999980993, 676.5203681218851, -1259.1392167224028, 771.32342877765313,
  -176.61502916214059, 12.507343278686905, -0.13857109526572012, 9.9843695780195716e-6,
  1.5056327351493116e-7,
]

export function gamma(z: number): number {
  if (z < 0.5) return Math.PI / (Math.sin(Math.PI * z) * gamma(1 - z))
  const n = z - 1
  let x = LANCZOS[0] as number
  for (let i = 1; i < LANCZOS.length; i++) x += (LANCZOS[i] as number) / (n + i)
  const t = n + 7.5
  return Math.sqrt(2 * Math.PI) * Math.pow(t, n + 0.5) * Math.exp(-t) * x
}

function factorial(n: number): number {
  if (!Number.isFinite(n) || n < 0) return NaN
  if (Number.isInteger(n)) {
    if (n > 170) return Infinity
    let out = 1
    for (let i = 2; i <= n; i++) out *= i
    return out
  }
  return gamma(n + 1)
}

const F1 = new Set([
  'sin', 'cos', 'tan', 'sinh', 'cosh', 'tanh', 'asin', 'acos', 'atan',
  'asinh', 'acosh', 'atanh', 'ln', 'log', 'log10', 'log2', 'exp',
  'sqrt', 'cbrt', 'abs', 'sign', 'ceil', 'floor', 'round', 'trunc',
  'fact', 'exp2', 'deg', 'rad',
])

const F2 = new Set(['atan2', 'pow', 'min', 'max', 'mod', 'hypot', 'gcd', 'lcm', 'logb'])

const TRIG_DEG = new Set(['sin', 'cos', 'tan', 'sinh', 'cosh', 'tanh'])

export class Parser {
  private expr = ''
  private pos = 0
  private vars: Record<string, number> = {}

  constructor(angleMode: AngleMode = 'rad', vars: Record<string, number> = {}) {
    this.angleMode = angleMode
    this.vars = vars
  }

  private angleMode: AngleMode

  setAngleMode(m: AngleMode) {
    this.angleMode = m
  }

  evaluate(expression: string): number {
    this.expr = expression
    this.pos = 0
    if (this.expr.trim() === '') throw new ExprError('Empty expression', 0)
    const result = this.parseExpression()
    this.skipWhitespace()
    if (!this.isEnd()) throw new ExprError(`Unexpected "${this.current()}"`, this.pos)
    if (!Number.isFinite(result)) throw new ExprError('Result is not finite', 0)
    return result
  }

  private toRadians(v: number): number {
    if (this.angleMode === 'deg') return (v * Math.PI) / 180
    if (this.angleMode === 'grad') return (v * Math.PI) / 200
    return v
  }

  private fromRadians(v: number): number {
    if (this.angleMode === 'deg') return (v * 180) / Math.PI
    if (this.angleMode === 'grad') return (v * 200) / Math.PI
    return v
  }

  private parseExpression(): number {
    let left = this.parseTerm()
    for (;;) {
      this.skipWhitespace()
      const c = this.current()
      if (c === '+') {
        this.pos++
        left += this.parseTerm()
      } else if (c === '-') {
        this.pos++
        left -= this.parseTerm()
      } else return left
    }
  }

  private parseTerm(): number {
    let left = this.parseUnary()
    for (;;) {
      this.skipWhitespace()
      const c = this.current()
      if (c === '*') {
        this.pos++
        left *= this.parseUnary()
      } else if (c === '/') {
        this.pos++
        left /= this.parseUnary()
      } else if (c === '%') {
        this.pos++
        left %= this.parseUnary()
      } else if (c !== '' && (c === '(' || /[0-9.]/.test(c) || /[a-zA-Z_π]/.test(c))) {
        left *= this.parseUnary()
      } else return left
    }
  }

  private parseUnary(): number {
    this.skipWhitespace()
    const c = this.current()
    if (c === '-') {
      this.pos++
      return -this.parseUnary()
    }
    if (c === '+') {
      this.pos++
      return this.parseUnary()
    }
    return this.parsePower()
  }

  private parsePower(): number {
    const base = this.parsePostfix()
    this.skipWhitespace()
    if (this.current() === '^') {
      this.pos++
      return Math.pow(base, this.parseUnary())
    }
    return base
  }

  private parsePostfix(): number {
    let v = this.parsePrimary()
    for (;;) {
      this.skipWhitespace()
      if (this.current() === '!') {
        this.pos++
        v = factorial(v)
      } else return v
    }
  }

  private parsePrimary(): number {
    this.skipWhitespace()
    const c = this.current()

    if (c === '(') {
      this.pos++
      const v = this.parseExpression()
      this.skipWhitespace()
      if (this.current() !== ')') throw new ExprError('Expected ")"', this.pos)
      this.pos++
      return v
    }

    if (/[0-9.]/.test(c)) {
      const start = this.pos
      while (/[0-9.]/.test(this.current())) this.pos++
      const n = Number(this.expr.slice(start, this.pos))
      if (Number.isNaN(n)) throw new ExprError('Malformed number', start)
      return n
    }

    if (/[a-zA-Z_π]/.test(c)) {
      const name = this.parseIdentifier()
      this.skipWhitespace()
      const upper = name.toLowerCase()

      if (this.current() === '(') {
        const isKnown = F1.has(upper) || F2.has(upper)
        if (!isKnown) throw new ExprError(`Unknown function "${name}"`, this.pos)
        this.pos++
        const args = this.parseArgs()
        return this.callFunction(upper, args)
      }

      if (this.vars[upper] !== undefined) return this.vars[upper] as number
      if (CONSTANTS[lower(name)] !== undefined) return CONSTANTS[lower(name)] as number
      throw new ExprError(`Unknown name "${name}"`, this.pos)
    }

    throw new ExprError(c === '' ? 'Unexpected end of expression' : `Unexpected "${c}"`, this.pos)
  }

  private parseArgs(): number[] {
    const args: number[] = []
    this.skipWhitespace()
    if (this.current() === ')') {
      this.pos++
      return args
    }
    for (;;) {
      args.push(this.parseExpression())
      this.skipWhitespace()
      const c = this.current()
      if (c === ',') {
        this.pos++
        continue
      }
      if (c === ')') {
        this.pos++
        return args
      }
      throw new ExprError('Expected "," or ")"', this.pos)
    }
  }

  private parseIdentifier(): string {
    const start = this.pos
    while (/[a-zA-Z0-9_π]/.test(this.current())) this.pos++
    return this.expr.slice(start, this.pos)
  }

  private callFunction(name: string, a: number[]): number {
    const need = F1.has(name) ? 1 : 2
    if (a.length !== need) {
      throw new ExprError(`${name}() expects ${need} argument${need > 1 ? 's' : ''}`, 0)
    }
    const x = a[0] as number
    const y = a[1] as number

    if (name === 'mod') return x % y
    if (name === 'pow') return Math.pow(x, y)
    if (name === 'min') return Math.min(x, y)
    if (name === 'max') return Math.max(x, y)
    if (name === 'atan2') return this.fromRadians(Math.atan2(x, y))
    if (name === 'hypot') return Math.hypot(x, y)
    if (name === 'logb') return Math.log(x) / Math.log(y)
    if (name === 'gcd') return gcd(Math.abs(x), Math.abs(y))
    if (name === 'lcm') return lcm(Math.abs(x), Math.abs(y))

    const trig = TRIG_DEG.has(name) ? this.toRadians(x) : x

    switch (name) {
      case 'sin': return Math.sin(trig)
      case 'cos': return Math.cos(trig)
      case 'tan': return Math.tan(trig)
      case 'sinh': return Math.sinh(x)
      case 'cosh': return Math.cosh(x)
      case 'tanh': return Math.tanh(x)
      case 'asin': return this.fromRadians(Math.asin(x))
      case 'acos': return this.fromRadians(Math.acos(x))
      case 'atan': return this.fromRadians(Math.atan(x))
      case 'asinh': return Math.asinh(x)
      case 'acosh': return Math.acosh(x)
      case 'atanh': return Math.atanh(x)
      case 'ln': return Math.log(x)
      case 'log':
      case 'log10': return Math.log10(x)
      case 'log2': return Math.log2(x)
      case 'exp': return Math.exp(x)
      case 'exp2': return Math.pow(2, x)
      case 'sqrt': return Math.sqrt(x)
      case 'cbrt': return Math.cbrt(x)
      case 'abs': return Math.abs(x)
      case 'sign': return Math.sign(x)
      case 'ceil': return Math.ceil(x)
      case 'floor': return Math.floor(x)
      case 'round': return Math.round(x)
      case 'trunc': return Math.trunc(x)
      case 'fact': return factorial(x)
      case 'deg': return (x * 180) / Math.PI
      case 'rad': return (x * Math.PI) / 180
      default: throw new ExprError(`Unknown function "${name}"`, 0)
    }
  }

  private skipWhitespace() {
    while (this.pos < this.expr.length && /\s/.test(this.expr[this.pos] as string)) this.pos++
  }

  private isEnd(): boolean {
    return this.pos >= this.expr.length
  }

  private current(): string {
    return this.pos < this.expr.length ? (this.expr[this.pos] as string) : ''
  }
}

function lower(s: string) {
  return s.toLowerCase()
}

export function gcd(a: number, b: number): number {
  a = Math.abs(a)
  b = Math.abs(b)
  while (b) {
    const t = b
    b = a % b
    a = t
  }
  return a
}

export function lcm(a: number, b: number): number {
  if (!a || !b) return 0
  return Math.abs(a * b) / gcd(a, b)
}

const cache = new Map<string, number>()

export function evalExpression(
  expression: string,
  angleMode: AngleMode = 'rad',
  vars: Record<string, number> = {},
): number {
  const key = `${angleMode}|${JSON.stringify(vars)}|${expression}`
  const hit = cache.get(key)
  if (hit !== undefined) return hit
  const parser = new Parser(angleMode, vars)
  const result = parser.evaluate(expression)
  if (cache.size > 500) cache.clear()
  cache.set(key, result)
  return result
}

export function tryEval(
  expression: string,
  angleMode: AngleMode = 'rad',
): { value?: number; error?: string } {
  try {
    return { value: evalExpression(expression, angleMode) }
  } catch (e) {
    return { error: e instanceof Error ? e.message : 'Invalid expression' }
  }
}