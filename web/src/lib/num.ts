export function gcd(a: number, b: number): number {
  a = Math.abs(Math.trunc(a))
  b = Math.abs(Math.trunc(b))
  while (b) {
    const t = b
    b = a % b
    a = t
  }
  return a
}

export function lcm(a: number, b: number): number {
  if (!a || !b) return 0
  return Math.abs((a * b) / gcd(a, b))
}

export function extendedGcd(a: number, b: number): { g: number; x: number; y: number } {
  let oldR = Math.trunc(a)
  let r = Math.trunc(b)
  let oldX = 1
  let x = 0
  let oldY = 0
  let y = 1
  while (r !== 0) {
    const q = Math.trunc(oldR / r)
    ;[oldR, r] = [r, oldR - q * r]
    ;[oldX, x] = [x, oldX - q * x]
    ;[oldY, y] = [y, oldY - q * y]
  }
  return { g: oldR, x: oldX, y: oldY }
}

export function modInverse(a: number, m: number): number | undefined {
  if (m <= 0) return undefined
  const { g, x } = extendedGcd(a, m)
  if (g !== 1) return undefined
  return ((x % m) + m) % m
}

export function totient(n: number): number {
  n = Math.trunc(n)
  if (n <= 0) return 0
  let result = n
  let p = 2
  let remaining = n
  while (p * p <= remaining) {
    if (remaining % p === 0) {
      while (remaining % p === 0) remaining /= p
      result -= result / p
    }
    p++
  }
  if (remaining > 1) result -= result / remaining
  return result
}

export function powMod(base: number, exponent: number, modulus: number): number {
  if (modulus <= 0) return NaN
  let result = 1
  let b = ((Math.trunc(base) % modulus) + modulus) % modulus
  let e = Math.trunc(exponent)
  while (e > 0) {
    if (e & 1) result = (result * b) % modulus
    b = (b * b) % modulus
    e = Math.floor(e / 2)
  }
  return result
}

export function euclidSteps(a: number, b: number): string[] {
  const steps: string[] = []
  let x = Math.abs(Math.trunc(a))
  let y = Math.abs(Math.trunc(b))
  if (x === 0 && y === 0) return steps
  steps.push(`${x} = ${Math.floor(x / Math.max(y, 1))}·${y} + ${x % Math.max(y, 1)}`)
  while (y !== 0) {
    const q = Math.floor(x / y)
    const r = x % y
    steps.push(`${x} = ${q}·${y} + ${r}`)
    x = y
    y = r
  }
  return steps
}

export interface Fraction {
  n: number
  d: number
}

export function parseFraction(text: string): Fraction | undefined {
  const trimmed = text.trim()
  if (!trimmed) return undefined
  const mixed = /^(-?\d+)\s+(\d+)\s*\/\s*(\d+)$/.exec(trimmed)
  if (mixed) {
    const whole = Number(mixed[1])
    const n = Number(mixed[2])
    const d = Number(mixed[3])
    if (d === 0) return undefined
    const sign = whole < 0 ? -1 : 1
    return { n: sign * (Math.abs(whole) * d + n), d }
  }
  const frac = /^(-?\d+)\s*\/\s*(-?\d+)$/.exec(trimmed)
  if (frac) {
    const n = Number(frac[1])
    const d = Number(frac[2])
    if (d === 0) return undefined
    return { n, d }
  }
  const whole = /^-?\d+(\.\d+)?$/.exec(trimmed)
  if (whole) {
    const v = Number(trimmed)
    const d = 10 ** (trimmed.split('.')[1]?.length ?? 0)
    return { n: Math.round(v * d), d }
  }
  return undefined
}

export function reduceFraction(f: Fraction): Fraction {
  if (f.d === 0) return f
  const sign = f.d < 0 ? -1 : 1
  const n = f.n * sign
  const d = Math.abs(f.d)
  const g = gcd(n, d) || 1
  return { n: n / g, d: d / g }
}

export function formatFraction(f: Fraction): string {
  const r = reduceFraction(f)
  if (r.d === 1) return String(r.n)
  return `${r.n}/${r.d}`
}

export function addFraction(a: Fraction, b: Fraction): Fraction {
  return reduceFraction({ n: a.n * b.d + b.n * a.d, d: a.d * b.d })
}

export function subtractFraction(a: Fraction, b: Fraction): Fraction {
  return reduceFraction({ n: a.n * b.d - b.n * a.d, d: a.d * b.d })
}

export function multiplyFraction(a: Fraction, b: Fraction): Fraction {
  return reduceFraction({ n: a.n * b.n, d: a.d * b.d })
}

export function divideFraction(a: Fraction, b: Fraction): Fraction | undefined {
  if (b.n === 0) return undefined
  return reduceFraction({ n: a.n * b.d, d: a.d * b.n })
}

export function fractionToDecimal(f: Fraction): number {
  return f.d === 0 ? NaN : f.n / f.d
}
