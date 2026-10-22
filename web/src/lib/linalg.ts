export interface LinearSolution {
  x: number[]
  determinant: number
  rank: number
  rref: number[][]
  singular: boolean
}

/** Solves A·x = b by Gaussian elimination with partial pivoting. */
export function solveLinearSystem(a: number[][], b: number[]): LinearSolution | undefined {
  const n = a.length
  if (n === 0 || a.some((row) => row.length !== n) || b.length !== n) return undefined

  const m: number[][] = a.map((row, i) => [...row, b[i] as number])
  let det = 1
  let singular = false

  for (let col = 0; col < n; col++) {
    let best = col
    for (let r = col + 1; r < n; r++) {
      if (Math.abs(m[r]![col] as number) > Math.abs(m[best]![col] as number)) best = r
    }
    if (Math.abs(m[best]![col] as number) < 1e-12) {
      singular = true
      break
    }
    if (best !== col) {
      const t = m[best]!
      m[best] = m[col]!
      m[col] = t
      det = -det
    }
    const pivot = m[col]![col] as number
    det *= pivot
    for (let r = col + 1; r < n; r++) {
      const factor = (m[r]![col] as number) / pivot
      if (factor === 0) continue
      for (let c = col; c <= n; c++) {
        m[r]![c] = (m[r]![c] as number) - factor * (m[col]![c] as number)
      }
    }
  }

  const x = new Array<number>(n).fill(0)
  for (let i = n - 1; i >= 0; i--) {
    let sum = m[i]![n] as number
    for (let j = i + 1; j < n; j++) sum -= (m[i]![j] as number) * (x[j] as number)
    const pivot = m[i]![i] as number
    if (Math.abs(pivot) < 1e-12) {
      singular = true
      break
    }
    x[i] = sum / pivot
  }

  const rref = reduceToRref(m.map((row) => [...row]))
  let rank = 0
  for (const row of rref) {
    if (row.some((v, i) => i < n && Math.abs(v) > 1e-12)) rank++
  }

  return { x, determinant: singular ? 0 : det, rank, rref, singular }
}

function reduceToRref(m: number[][]): number[][] {
  const rows = m.length
  if (!rows) return []
  const cols = m[0]!.length
  let r = 0
  for (let c = 0; c < cols && r < rows; c++) {
    let best = r
    for (let i = r + 1; i < rows; i++) {
      if (Math.abs(m[i]![c] as number) > Math.abs(m[best]![c] as number)) best = i
    }
    if (Math.abs(m[best]![c] as number) < 1e-12) continue
    const t = m[best]!
    m[best] = m[r]!
    m[r] = t
    const pivot = m[r]![c] as number
    for (let j = 0; j < cols; j++) m[r]![j] = (m[r]![j] as number) / pivot
    for (let i = 0; i < rows; i++) {
      if (i === r) continue
      const factor = m[i]![c] as number
      if (factor === 0) continue
      for (let j = 0; j < cols; j++) m[i]![j] = (m[i]![j] as number) - factor * (m[r]![j] as number)
    }
    r++
  }
  return m
}

export function formatMatrix(m: number[][], digits = 4): string {
  return m
    .map((row) => row.map((v) => (Number.isFinite(v) ? v.toFixed(digits).replace(/\.?0+$/, '') : '—')).join('  '))
    .join('\n')
}
