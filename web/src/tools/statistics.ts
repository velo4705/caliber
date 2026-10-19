import type { Tool } from '../registry/types'
import { num, list } from '../lib/values'

function erf(x: number): number {
  const sign = x < 0 ? -1 : 1
  const ax = Math.abs(x)
  const t = 1 / (1 + 0.3275911 * ax)
  const y =
    1 -
    ((((1.061405429 * t - 1.453152027) * t + 1.421413741) * t - 0.284496736) * t + 0.254829592) *
      t *
      Math.exp(-ax * ax)
  return sign * y
}

export function normalCdf(z: number): number {
  return 0.5 * (1 + erf(z / Math.SQRT2))
}

function mean(a: number[]) {
  return a.reduce((x, y) => x + y, 0) / a.length
}

export const statisticsTools: Tool[] = [
  {
    id: 'descriptive',
    category: 'statistics',
    name: 'Descriptive Statistics',
    tagline: 'Central tendency and spread for a dataset',
    keywords: ['mean', 'median', 'mode', 'standard deviation', 'variance', 'average', 'summary', 'spread'],
    inputs: [
      {
        key: 'data',
        label: 'Data (comma or space separated)',
        kind: 'number-list',
        default: '2, 4, 4, 4, 5, 5, 7, 9',
        help: 'One row works fine — you only need the numbers.',
      },
    ],
    outputs: [
      { key: 'n', label: 'Count (n)', formula: '—', emphasis: 'primary' },
      { key: 'mean', label: 'Mean (x̄)', formula: 'Σx / n', emphasis: 'primary' },
      { key: 'median', label: 'Median', formula: 'middle value' },
      { key: 'mode', label: 'Mode', formula: 'most frequent' },
      { key: 'min', label: 'Minimum', formula: '—' },
      { key: 'max', label: 'Maximum', formula: '—' },
      { key: 'range', label: 'Range', formula: 'max − min' },
      { key: 'variance', label: 'Variance (population)', formula: 'Σ(x−x̄)² / n' },
      { key: 'stddev', label: 'Std deviation (population)', formula: '√variance' },
      { key: 'stddevS', label: 'Std deviation (sample)', formula: '√[Σ(x−x̄)² / (n−1)]' },
    ],
    compute: (v) => {
      const d = list(v, 'data')
      if (!d.length) return {}
      const sorted = [...d].sort((a, b) => a - b)
      const n = d.length
      const m = mean(d)
      const mid = Math.floor(n / 2)
      const median = n % 2 ? (sorted[mid] as number) : (((sorted[mid - 1] as number) + (sorted[mid] as number)) / 2)
      const counts = new Map<number, number>()
      for (const x of d) counts.set(x, (counts.get(x) ?? 0) + 1)
      let best = 1
      let mode: number | undefined
      for (const [k, c] of counts) {
        if (c > best) {
          best = c
          mode = k
        }
      }
      const ss = d.reduce((acc, x) => acc + (x - m) ** 2, 0)
      const min = sorted[0] as number
      const max = sorted[n - 1] as number
      return {
        n,
        mean: m,
        median,
        mode,
        min,
        max,
        range: max - min,
        variance: ss / n,
        stddev: Math.sqrt(ss / n),
        stddevS: n > 1 ? Math.sqrt(ss / (n - 1)) : undefined,
      }
    },
    reference: ['x̄ = Σx / n', 'σ = √[Σ(x−x̄)² / n]', 's = √[Σ(x−x̄)² / (n−1)]'],
    examples: [
      { label: 'Class marks', inputs: { data: '2, 4, 4, 4, 5, 5, 7, 9' } },
      { label: 'Sensor readings', inputs: { data: '12.4 13.1 12.9 13.8 12.2 13.5' } },
    ],
  },
  {
    id: 'zscore',
    category: 'statistics',
    name: 'Z-Score & Percentile',
    tagline: 'Standardise a value against a distribution',
    keywords: ['z-score', 'z score', 'standard score', 'normalise', 'percentile', 'sigma'],
    inputs: [
      { key: 'x', label: 'Value x', default: '85' },
      { key: 'mean', label: 'Mean (μ)', default: '75' },
      { key: 'sd', label: 'Std deviation (σ)', default: '8' },
    ],
    outputs: [
      { key: 'z', label: 'Z-score', formula: '(x − μ) / σ', emphasis: 'primary' },
      { key: 'percentile', label: 'Percentile', formula: 'Φ(z) × 100', emphasis: 'primary' },
      { key: 'inverse', label: 'Value at that z', formula: 'μ + zσ' },
    ],
    compute: (v) => {
      const x = num(v, 'x')
      const m = num(v, 'mean')
      const sd = num(v, 'sd')
      if (x === undefined || m === undefined || sd === undefined || sd === 0) return {}
      const z = (x - m) / sd
      return { z, percentile: normalCdf(z) * 100, inverse: m + z * sd }
    },
    reference: ['z = (x − μ) / σ', 'percentile = Φ(z) × 100'],
    examples: [{ label: 'Exam mark', inputs: { x: '85', mean: '75', sd: '8' } }],
  },
  {
    id: 'regression',
    category: 'statistics',
    name: 'Linear Regression',
    tagline: 'Least-squares fit y = mx + c',
    keywords: ['regression', 'least squares', 'trend', 'fit', 'slope', 'intercept', 'r squared', 'correlation'],
    inputs: [
      { key: 'x', label: 'x values', kind: 'number-list', default: '1, 2, 3, 4, 5' },
      { key: 'y', label: 'y values', kind: 'number-list', default: '2.1, 4.2, 5.8, 8.1, 9.9' },
    ],
    outputs: [
      { key: 'slope', label: 'Slope (m)', formula: 'Cov(x,y) / Var(x)', emphasis: 'primary' },
      { key: 'intercept', label: 'Intercept (c)', formula: 'ȳ − m·x̄', emphasis: 'primary' },
      { key: 'r2', label: 'R²', formula: 'r²', emphasis: 'primary' },
      { key: 'r', label: 'Pearson r', formula: 'r' },
      { key: 'n', label: 'Pairs (n)', formula: '—' },
    ],
    compute: (v) => {
      const xs = list(v, 'x')
      const ys = list(v, 'y')
      const n = Math.min(xs.length, ys.length)
      if (n < 2) return {}
      const mx = mean(xs.slice(0, n))
      const my = mean(ys.slice(0, n))
      let sxy = 0
      let sxx = 0
      let syy = 0
      for (let i = 0; i < n; i++) {
        const dx = (xs[i] as number) - mx
        const dy = (ys[i] as number) - my
        sxy += dx * dy
        sxx += dx * dx
        syy += dy * dy
      }
      if (sxx === 0) return {}
      const slope = sxy / sxx
      const intercept = my - slope * mx
      const r = syy === 0 ? undefined : sxy / Math.sqrt(sxx * syy)
      return { slope, intercept, r, r2: r === undefined ? undefined : r * r, n }
    },
    reference: ['m = Σ(x−x̄)(y−ȳ) / Σ(x−x̄)²', 'c = ȳ − m·x̄'],
    examples: [
      { label: 'Straight-line fit', inputs: { x: '1, 2, 3, 4, 5', y: '2.1, 4.2, 5.8, 8.1, 9.9' } },
    ],
  },
  {
    id: 'binomial',
    category: 'statistics',
    name: 'Binomial Probability',
    tagline: 'P(X = k) for n independent trials',
    keywords: ['binomial', 'probability', 'distribution', 'trials', 'combination', 'dice', 'coin'],
    inputs: [
      { key: 'n', label: 'Trials (n)', default: '10', min: 0, step: 1 },
      { key: 'p', label: 'Probability each (p)', default: '0.5', min: 0, max: 1, step: 0.05 },
      { key: 'k', label: 'Successes (k)', default: '5', min: 0, step: 1 },
    ],
    outputs: [
      { key: 'exact', label: 'P(X = k)', formula: 'nCk · p^k (1−p)^(n−k)', emphasis: 'primary' },
      { key: 'atLeast', label: 'P(X ≥ k)', formula: 'Σ from k', emphasis: 'primary' },
      { key: 'atMost', label: 'P(X ≤ k)', formula: 'Σ up to k', emphasis: 'primary' },
      { key: 'mean', label: 'Expected value', formula: 'np' },
      { key: 'variance', label: 'Variance', formula: 'np(1−p)' },
      { key: 'mode', label: 'Most likely', formula: 'floor((n+1)p)' },
    ],
    compute: (v) => {
      const n = num(v, 'n')
      const p = num(v, 'p')
      const k = num(v, 'k')
      if (n === undefined || p === undefined || k === undefined || n < 0 || k < 0 || k > n) return {}
      const fact = (m: number) => {
        let o = 1
        for (let i = 2; i <= m; i++) o *= i
        return o
      }
      const pmf = (i: number) => (fact(n) / (fact(i) * fact(n - i))) * Math.pow(p, i) * Math.pow(1 - p, n - i)
      let atLeast = 0
      for (let i = k; i <= n; i++) atLeast += pmf(i)
      let atMost = 0
      for (let i = 0; i <= k; i++) atMost += pmf(i)
      return {
        exact: pmf(k),
        atLeast,
        atMost,
        mean: n * p,
        variance: n * p * (1 - p),
        mode: Math.floor((n + 1) * p),
      }
    },
    reference: ['P(X = k) = nCk · p^k · (1−p)^(n−k)'],
    examples: [{ label: 'Fair coin, 10 flips', inputs: { n: '10', p: '0.5', k: '5' } }],
  },
]