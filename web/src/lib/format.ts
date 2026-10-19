export function formatNumber(value: number | string | undefined, precision = 6): string {
  if (value === undefined) return '—'
  if (typeof value === 'string') return value === '' ? '—' : value
  if (!Number.isFinite(value)) return '—'
  if (Number.isInteger(value) && Math.abs(value) < 1e15) {
    return value.toLocaleString('en-US', { maximumFractionDigits: 0 })
  }
  const abs = Math.abs(value)
  if (abs !== 0 && (abs < 1e-6 || abs >= 1e12)) {
    return value.toExponential(Math.max(0, precision - 1))
  }
  const rounded = Number(value.toPrecision(precision))
  return rounded.toLocaleString('en-US', { maximumFractionDigits: precision })
}

export function formatWithUnit(value: number | undefined, unit: string | undefined, precision?: number): string {
  const n = formatNumber(value, precision)
  if (unit && n !== '—') return `${n} ${unit}`
  return n
}

export function formatList(values: number[]): string {
  if (!values.length) return '—'
  return values.map((v) => formatNumber(v)).join(', ')
}