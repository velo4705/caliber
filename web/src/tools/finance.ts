import type { Tool } from '../registry/types'
import { num } from '../lib/values'
import { formatNumber as fmt } from '../lib/format'

export const financeTools: Tool[] = [
  {
    id: 'compound-interest',
    category: 'finance',
    name: 'Compound Interest',
    tagline: 'Growth from principal, rate and time',
    keywords: ['compound', 'interest', 'investment', 'growth', 'apy', 'principal', 'savings'],
    inputs: [
      { key: 'P', label: 'Principal (P)', unit: 'currency', default: '10000' },
      { key: 'r', label: 'Annual rate (%)', unit: '%', default: '5', step: 0.1 },
      { key: 'n', label: 'Compounds per year', default: '12', min: 1, step: 1 },
      { key: 't', label: 'Years (t)', default: '10', step: 0.5 },
      { key: 'add', label: 'Regular addition', unit: 'currency', default: '0' },
    ],
    outputs: [
      { key: 'A', label: 'Future value', formula: 'A = P(1 + r/n)^(nt)', emphasis: 'primary' },
      { key: 'interest', label: 'Interest earned', formula: 'A − P', emphasis: 'primary' },
      { key: 'apy', label: 'Effective rate (APY)', formula: '(1 + r/n)^n − 1', unit: '%', emphasis: 'primary' },
      { key: 'doubling', label: 'Doubling time', formula: 'ln2 / ln(1+r)', unit: 'years' },
      { key: 'annuity', label: 'With regular additions', formula: 'future value of series', emphasis: 'primary' },
    ],
    compute: (v) => {
      const P = num(v, 'P')
      const r = num(v, 'r')
      const n = Math.round(num(v, 'n') ?? 12)
      const t = num(v, 't')
      const add = num(v, 'add') ?? 0
      if (P === undefined || r === undefined || t === undefined || n === 0) return {}
      const i = r / 100
      const per = i / n
      const A = P * Math.pow(1 + per, n * t)
      const growth = A - P
      const apy = (Math.pow(1 + per, n) - 1) * 100
      const annuity = per === 0 ? add * n * t : add * ((Math.pow(1 + per, n * t) - 1) / per)
      return {
        A,
        interest: growth,
        apy,
        doubling: Math.log(2) / Math.log(1 + per * n),
        annuity: A + annuity,
      }
    },
    steps: (_v, r) => {
      const out: string[] = []
      if (r.apy !== undefined) out.push(`Effective annual rate = ${fmt(r.apy, 4)} %`)
      if (r.interest !== undefined) out.push(`Interest earned = ${fmt(r.interest)}`)
      return out
    },
    reference: ['A = P(1 + r/n)^(nt)', 'APY = (1 + r/n)^n − 1'],
    examples: [
      { label: '£10k at 5% for 10 years', inputs: { P: '10000', r: '5', n: '12', t: '10', add: '0' } },
      { label: 'With £200/month', inputs: { P: '0', r: '5', n: '12', t: '10', add: '200' } },
    ],
  },
  {
    id: 'loan-payment',
    category: 'finance',
    name: 'Loan Repayment',
    tagline: 'Monthly payment and total interest',
    keywords: ['loan', 'mortgage', 'repayment', 'amortisation', 'amortization', 'monthly payment', 'interest'],
    inputs: [
      { key: 'P', label: 'Loan amount', unit: 'currency', default: '200000' },
      { key: 'r', label: 'Annual rate (%)', unit: '%', default: '4.5', step: 0.01 },
      { key: 't', label: 'Term (years)', default: '25', step: 1 },
    ],
    outputs: [
      { key: 'monthly', label: 'Monthly payment', formula: 'M = P·i(1+i)ⁿ / ((1+i)ⁿ−1)', emphasis: 'primary' },
      { key: 'totalInterest', label: 'Total interest', formula: 'M·n − P', emphasis: 'primary' },
      { key: 'totalPaid', label: 'Total repaid', formula: 'M·n', emphasis: 'primary' },
      { key: 'n', label: 'Number of payments', formula: 'years × 12' },
      { key: 'interestShare', label: 'Interest as share', formula: 'interest / total', unit: '%' },
    ],
    compute: (v) => {
      const P = num(v, 'P')
      const r = num(v, 'r')
      const t = num(v, 't')
      if (P === undefined || r === undefined || t === undefined) return {}
      const i = r / 100 / 12
      const n = Math.round(t * 12)
      const monthly = i === 0 ? P / n : (P * i * Math.pow(1 + i, n)) / (Math.pow(1 + i, n) - 1)
      const totalPaid = monthly * n
      const totalInterest = totalPaid - P
      return {
        monthly,
        totalInterest,
        totalPaid,
        n,
        interestShare: totalPaid === 0 ? undefined : (totalInterest / totalPaid) * 100,
      }
    },
    reference: ['M = P·i(1+i)ⁿ / ((1+i)ⁿ − 1)', 'i = annual rate / 12'],
    examples: [{ label: '£200k at 4.5% over 25y', inputs: { P: '200000', r: '4.5', t: '25' } }],
  },
  {
    id: 'break-even',
    category: 'finance',
    name: 'Break-Even Analysis',
    tagline: 'Units or revenue needed to cover fixed costs',
    keywords: ['break even', 'break-even', 'fixed cost', 'margin', 'contribution', 'pricing'],
    inputs: [
      { key: 'fixed', label: 'Fixed costs', unit: 'currency', default: '50000' },
      { key: 'variable', label: 'Variable cost / unit', unit: 'currency', default: '12' },
      { key: 'price', label: 'Selling price / unit', unit: 'currency', default: '30' },
    ],
    outputs: [
      { key: 'units', label: 'Break-even units', formula: 'F / (P − V)', emphasis: 'primary' },
      { key: 'revenue', label: 'Break-even revenue', formula: 'units × P', emphasis: 'primary' },
      { key: 'margin', label: 'Contribution margin', unit: 'currency', formula: 'P − V', emphasis: 'primary' },
      { key: 'marginPct', label: 'Margin ratio', formula: '(P − V) / P', unit: '%' },
    ],
    compute: (v) => {
      const F = num(v, 'fixed')
      const V = num(v, 'variable')
      const P = num(v, 'price')
      if (F === undefined || V === undefined || P === undefined) return {}
      const cm = P - V
      return {
        margin: cm,
        units: cm > 0 ? F / cm : undefined,
        revenue: cm > 0 && P > 0 ? (F / cm) * P : undefined,
        marginPct: P !== 0 ? (cm / P) * 100 : undefined,
      }
    },
    reference: ['Break-even units = Fixed costs / (Price − Variable cost)'],
    examples: [{ label: '£50k fixed, £30 price', inputs: { fixed: '50000', variable: '12', price: '30' } }],
  },
]