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

export type Node =
  | { t: 'num'; v: number }
  | { t: 'var'; name: string; pos: number }
  | { t: 'bin'; op: '+' | '-' | '*' | '/' | '%'; l: Node; r: Node }
  | { t: 'neg'; e: Node }
  | { t: 'pos'; e: Node }
  | { t: 'pow'; b: Node; e: Node }
  | { t: 'fact'; e: Node }
  | { t: 'call'; name: string; args: Node[] }

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

import { gcd, lcm } from './num'

export { gcd, lcm }

function toRadians(v: number, mode: AngleMode): number {
  if (mode === 'deg') return (v * Math.PI) / 180
  if (mode === 'grad') return (v * Math.PI) / 200
  return v
}

function fromRadians(v: number, mode: AngleMode): number {
  if (mode === 'deg') return (v * 180) / Math.PI
  if (mode === 'grad') return (v * 200) / Math.PI
  return v
}

/* ── Parser: expression string → AST ─────────────────────────────────────── */

class AstParser {
  private expr = ''
  private pos = 0

  constructor(expression: string) {
    this.expr = expression
  }

  parse(): Node {
    if (this.expr.trim() === '') throw new ExprError('Empty expression', 0)
    const node = this.parseExpression()
    this.skipWhitespace()
    if (!this.isEnd()) throw new ExprError(`Unexpected "${this.current()}"`, this.pos)
    return node
  }

  private parseExpression(): Node {
    let left = this.parseTerm()
    for (;;) {
      this.skipWhitespace()
      const c = this.current()
      if (c === '+') {
        this.pos++
        left = { t: 'bin', op: '+', l: left, r: this.parseTerm() }
      } else if (c === '-') {
        this.pos++
        left = { t: 'bin', op: '-', l: left, r: this.parseTerm() }
      } else return left
    }
  }

  private parseTerm(): Node {
    let left = this.parseUnary()
    for (;;) {
      this.skipWhitespace()
      const c = this.current()
      if (c === '*') {
        this.pos++
        left = { t: 'bin', op: '*', l: left, r: this.parseUnary() }
      } else if (c === '/') {
        this.pos++
        left = { t: 'bin', op: '/', l: left, r: this.parseUnary() }
      } else if (c === '%') {
        this.pos++
        left = { t: 'bin', op: '%', l: left, r: this.parseUnary() }
      } else if (c !== '' && (c === '(' || /[0-9.]/.test(c) || /[a-zA-Z_π]/.test(c))) {
        left = { t: 'bin', op: '*', l: left, r: this.parseUnary() }
      } else return left
    }
  }

  private parseUnary(): Node {
    this.skipWhitespace()
    const c = this.current()
    if (c === '-') {
      this.pos++
      return { t: 'neg', e: this.parseUnary() }
    }
    if (c === '+') {
      this.pos++
      return { t: 'pos', e: this.parseUnary() }
    }
    return this.parsePower()
  }

  private parsePower(): Node {
    const base = this.parsePostfix()
    this.skipWhitespace()
    if (this.current() === '^') {
      this.pos++
      return { t: 'pow', b: base, e: this.parseUnary() }
    }
    return base
  }

  private parsePostfix(): Node {
    let v = this.parsePrimary()
    for (;;) {
      this.skipWhitespace()
      if (this.current() === '!') {
        this.pos++
        v = { t: 'fact', e: v }
      } else return v
    }
  }

  private parsePrimary(): Node {
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
      return { t: 'num', v: n }
    }

    if (/[a-zA-Z_π]/.test(c)) {
      const namePos = this.pos
      const name = this.parseIdentifier()
      this.skipWhitespace()
      const lower = name.toLowerCase()

      if (this.current() === '(') {
        if (!F1.has(lower) && !F2.has(lower)) {
          throw new ExprError(`Unknown function "${name}"`, this.pos)
        }
        this.pos++
        const args = this.parseArgs()
        return { t: 'call', name: lower, args }
      }

      return { t: 'var', name, pos: namePos }
    }

    throw new ExprError(c === '' ? 'Unexpected end of expression' : `Unexpected "${c}"`, this.pos)
  }

  private parseArgs(): Node[] {
    const args: Node[] = []
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

/* ── Evaluator: AST → number ─────────────────────────────────────────────── */

export function evaluateNode(
  node: Node,
  vars: Record<string, number> = {},
  angleMode: AngleMode = 'rad',
): number {
  switch (node.t) {
    case 'num':
      return node.v
    case 'var': {
      const bound = vars[node.name.toLowerCase()]
      if (bound !== undefined) return bound
      const constant = CONSTANTS[node.name.toLowerCase()]
      if (constant !== undefined) return constant
      throw new ExprError(`Unknown name "${node.name}"`, node.pos)
    }
    case 'bin': {
      const l = evaluateNode(node.l, vars, angleMode)
      const r = evaluateNode(node.r, vars, angleMode)
      switch (node.op) {
        case '+': return l + r
        case '-': return l - r
        case '*': return l * r
        case '/': return l / r
        case '%': return l % r
      }
      return NaN
    }
    case 'neg':
      return -evaluateNode(node.e, vars, angleMode)
    case 'pos':
      return evaluateNode(node.e, vars, angleMode)
    case 'pow':
      return Math.pow(
        evaluateNode(node.b, vars, angleMode),
        evaluateNode(node.e, vars, angleMode),
      )
    case 'fact':
      return factorial(evaluateNode(node.e, vars, angleMode))
    case 'call':
      return callNode(node.name, node.args, vars, angleMode)
  }
}

function callNode(
  name: string,
  argNodes: Node[],
  vars: Record<string, number>,
  angleMode: AngleMode,
): number {
  const need = F1.has(name) ? 1 : 2
  if (argNodes.length !== need) {
    throw new ExprError(`${name}() expects ${need} argument${need > 1 ? 's' : ''}`, 0)
  }
  const x = evaluateNode(argNodes[0] as Node, vars, angleMode)
  const y = need === 2 ? evaluateNode(argNodes[1] as Node, vars, angleMode) : 0

  if (name === 'mod') return x % y
  if (name === 'pow') return Math.pow(x, y)
  if (name === 'min') return Math.min(x, y)
  if (name === 'max') return Math.max(x, y)
  if (name === 'atan2') return fromRadians(Math.atan2(x, y), angleMode)
  if (name === 'hypot') return Math.hypot(x, y)
  if (name === 'logb') return Math.log(x) / Math.log(y)
  if (name === 'gcd') return gcd(x, y)
  if (name === 'lcm') return lcm(x, y)

  const trig = TRIG_DEG.has(name) ? toRadians(x, angleMode) : x

  switch (name) {
    case 'sin': return Math.sin(trig)
    case 'cos': return Math.cos(trig)
    case 'tan': return Math.tan(trig)
    case 'sinh': return Math.sinh(x)
    case 'cosh': return Math.cosh(x)
    case 'tanh': return Math.tanh(x)
    case 'asin': return fromRadians(Math.asin(x), angleMode)
    case 'acos': return fromRadians(Math.acos(x), angleMode)
    case 'atan': return fromRadians(Math.atan(x), angleMode)
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

/* ── Public API ──────────────────────────────────────────────────────────── */

const astCache = new Map<string, Node>()

export function compileExpression(expression: string): Node {
  const hit = astCache.get(expression)
  if (hit) return hit
  const node = new AstParser(expression).parse()
  if (astCache.size > 500) astCache.clear()
  astCache.set(expression, node)
  return node
}

export class Parser {
  private angleMode: AngleMode
  private vars: Record<string, number>

  constructor(angleMode: AngleMode = 'rad', vars: Record<string, number> = {}) {
    this.angleMode = angleMode
    this.vars = vars
  }

  setAngleMode(m: AngleMode) {
    this.angleMode = m
  }

  setVars(vars: Record<string, number>) {
    this.vars = vars
  }

  evaluate(expression: string): number {
    const value = evaluateNode(compileExpression(expression), this.vars, this.angleMode)
    if (!Number.isFinite(value)) throw new ExprError('Result is not finite', 0)
    return value
  }
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
  const result = new Parser(angleMode, vars).evaluate(expression)
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
