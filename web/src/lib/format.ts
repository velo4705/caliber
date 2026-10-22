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

/**
 * A number that will be multiplied or raised to a power.
 * Negatives are parenthesised, so "-2" never reads as the base of "-2²".
 */
export function formatFactor(value: number | string | undefined, precision = 6): string {
  const s = formatNumber(value, precision)
  return s.startsWith('-') ? `(${s})` : s
}

/** A product of factors: "4 × 4 × 1". */
export function formatProduct(values: (number | string | undefined)[], precision = 6): string {
  const parts = values.map((v) => formatNumber(v, precision))
  if (!parts.length) return '—'
  return parts.join(' × ')
}

/** A signed coefficient for a sum: "+2" or "-2". */
export function formatSigned(value: number | string | undefined, precision = 6): string {
  if (value === undefined || typeof value === 'string') return formatNumber(value, precision)
  if (!Number.isFinite(value)) return '—'
  return `${value < 0 ? '-' : '+'}${formatNumber(Math.abs(value), precision)}`
}

/** An approximation: "≈ 3.84". */
export function formatApprox(value: number | string | undefined, precision = 6): string {
  if (value === undefined) return '—'
  return `≈ ${formatNumber(value, precision)}`
}

/** A value with its unit: "3.84 V". */
export function formatWithUnit(
  value: number | string | undefined,
  unit: string | undefined,
  precision?: number,
): string {
  const n = formatNumber(value, precision)
  if (unit && n !== '—') return `${n} ${unit}`
  return n
}

/** A quantity with a unit, for use inside a sentence. */
export function formatQuantity(
  value: number | string | undefined,
  unit: string | undefined,
  precision?: number,
): string {
  const n = formatNumber(value, precision)
  if (n === '—') return '—'
  return unit ? `${n} ${unit}` : n
}

export function formatList(values: number[]): string {
  if (!values.length) return '—'
  return values.map((v) => formatNumber(v)).join(', ')
}
