import type { Tool } from '../registry/types'
import { list, num } from '../lib/values'
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
  {
    id: 'npv-irr',
    category: 'finance',
    name: 'NPV & IRR',
    tagline: 'Net present value and internal rate of return',
    keywords: ['npv', 'irr', 'discount', 'present value', 'payback', 'cash flow', 'rate of return'],
    inputs: [
      {
        key: 'flows',
        label: 'Cash flows',
        kind: 'number-list',
        default: '-1000, 300, 400, 500, 600',
        help: 'First value is the cash flow at t = 0, then one per period. Negative means an outflow.',
      },
      { key: 'rate', label: 'Discount rate (%)', unit: '%', default: '10', step: 0.5 },
    ],
    outputs: [
      { key: 'npv', label: 'Net present value', formula: 'Σ Cₜ/(1+r)ᵗ', emphasis: 'primary' },
      { key: 'irr', label: 'Internal rate of return', formula: 'r where NPV = 0', unit: '%', emphasis: 'primary' },
      { key: 'payback', label: 'Payback period', formula: 'last negative to first positive', unit: 'periods' },
      { key: 'net', label: 'Net cash flow', formula: 'Σ Cₜ' },
      { key: 'profitability', label: 'Profitability index', formula: 'PV of inflows / |C₀|' },
    ],
    compute: (v) => {
      const flows = list(v, 'flows')
      const ratePct = num(v, 'rate') ?? 10
      if (flows.length < 2) return {}
      const r = ratePct / 100
      const npv = flows.reduce((acc, c, t) => acc + c / Math.pow(1 + r, t), 0)
      const net = flows.reduce((acc, c) => acc + c, 0)
      const inflowPv = flows.slice(1).reduce((acc, c, t) => acc + c / Math.pow(1 + r, t), 0)

      let irr: number | undefined
      let lo = -0.9999
      let hi = 10
      const f = (rate: number) => flows.reduce((acc, c, t) => acc + c / Math.pow(1 + rate, t), 0)
      if (f(lo) * f(hi) < 0) {
        for (let i = 0; i < 200; i++) {
          const mid = (lo + hi) / 2
          if (f(lo) * f(mid) <= 0) hi = mid
          else lo = mid
        }
        irr = ((lo + hi) / 2) * 100
      }

      let cumulative = 0
      let payback: number | undefined
      for (let t = 0; t < flows.length; t++) {
        const before = cumulative
        cumulative += (flows[t] as number)
        if (before < 0 && cumulative >= 0) {
          payback = t - 1 + (0 - before) / (cumulative - before)
          break
        }
      }

      const c0 = Math.abs(flows[0] as number)
      return {
        npv,
        irr,
        payback,
        net,
        profitability: c0 === 0 ? undefined : inflowPv / c0,
      }
    },
    reference: ['NPV = Σ Cₜ/(1+r)ᵗ', 'IRR is the rate where NPV = 0', 'Payback: cumulative cash flow crosses zero'],
    examples: [
      { label: '−1000 then 300–600', inputs: { flows: '-1000, 300, 400, 500, 600', rate: '10' } },
      { label: '−5000, 2000, 3000, 3000', inputs: { flows: '-5000, 2000, 3000, 3000', rate: '12' } },
    ],
  },
]
