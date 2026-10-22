export function num(values: Record<string, unknown>, key: string): number | undefined {
  const v = values[key]
  if (typeof v === 'number') return Number.isFinite(v) ? v : undefined
  if (typeof v === 'string' && v.trim() !== '') {
    const n = Number(v)
    return Number.isFinite(n) ? n : undefined
  }
  return undefined
}

export function derive<T extends number | undefined>(...candidates: (number | undefined)[]): T | undefined {
  for (const c of candidates) {
    if (typeof c === 'number' && Number.isFinite(c)) return c as T
  }
  return undefined
}

export function str(values: Record<string, unknown>, key: string): string {
  const v = values[key]
  return typeof v === 'string' ? v : ''
}

export function list(values: Record<string, unknown>, key: string): number[] {
  const v = values[key]
  if (Array.isArray(v)) return v.filter((n): n is number => typeof n === 'number' && Number.isFinite(n))
  if (typeof v === 'string') {
    return v
      .split(/[,\s;]+/)
      .map((s) => s.trim())
      .filter(Boolean)
      .map(Number)
      .filter((n) => Number.isFinite(n))
  }
  return []
}

export function rows(values: Record<string, unknown>, key: string): number[][] {
  const v = values[key]
  if (typeof v !== 'string') return []
  return v
    .split(/\r?\n/)
    .map((line) => line.split('|')[0] ?? '')
    .map((line) =>
      line
        .split(/[,\s]+/)
        .map((s) => s.trim())
        .filter(Boolean)
        .map(Number)
        .filter((n) => Number.isFinite(n)),
    )
    .filter((r) => r.length > 0)
}

export function requireAll(vals: (number | undefined)[]): number | undefined {
  for (const v of vals) if (typeof v !== 'number' || !Number.isFinite(v)) return undefined
  return 1
}