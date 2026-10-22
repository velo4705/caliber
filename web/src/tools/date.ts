import type { Tool } from '../registry/types'
import { str } from '../lib/values'
import { formatNumber as fmt } from '../lib/format'

const DAY_MS = 86_400_000

function parseDate(text: string): Date | undefined {
  const m = /^(\d{4})-(\d{1,2})-(\d{1,2})$/.exec(text.trim())
  if (!m) return undefined
  const y = Number(m[1])
  const mo = Number(m[2])
  const d = Number(m[3])
  if (mo < 1 || mo > 12 || d < 1 || d > 31) return undefined
  const date = new Date(Date.UTC(y, mo - 1, d))
  if (date.getUTCFullYear() !== y || date.getUTCMonth() !== mo - 1 || date.getUTCDate() !== d) return undefined
  return date
}

function monthsBetween(a: Date, b: Date): number {
  let months = (b.getUTCFullYear() - a.getUTCFullYear()) * 12 + (b.getUTCMonth() - a.getUTCMonth())
  if (b.getUTCDate() < a.getUTCDate()) months -= 1
  return months
}

function addDays(date: Date, days: number): Date {
  return new Date(date.getTime() + days * DAY_MS)
}

function addMonths(date: Date, months: number): Date {
  const target = new Date(Date.UTC(date.getUTCFullYear(), date.getUTCMonth() + months, 1))
  const daysInMonth = new Date(Date.UTC(target.getUTCFullYear(), target.getUTCMonth() + 1, 0)).getUTCDate()
  target.setUTCDate(Math.min(date.getUTCDate(), daysInMonth))
  return target
}

function iso(date: Date): string {
  return date.toISOString().slice(0, 10)
}

const WEEKDAYS = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday']

export const dateTools: Tool[] = [
  {
    id: 'date-difference',
    category: 'date',
    name: 'Date Difference',
    tagline: 'Days, weeks, months and years between two dates',
    keywords: ['date', 'difference', 'between', 'days', 'age span', 'interval', 'calendar'],
    inputs: [
      { key: 'from', label: 'From', kind: 'text', default: '2026-01-01', placeholder: 'YYYY-MM-DD' },
      { key: 'to', label: 'To', kind: 'text', default: '2026-12-31', placeholder: 'YYYY-MM-DD' },
    ],
    outputs: [
      { key: 'days', label: 'Days', formula: '|to - from|', emphasis: 'primary' },
      { key: 'weeks', label: 'Weeks', formula: 'days / 7', emphasis: 'primary' },
      { key: 'months', label: 'Months', formula: 'calendar months', emphasis: 'primary' },
      { key: 'years', label: 'Years', formula: 'calendar years' },
      { key: 'plus', label: 'From + days', formula: '—' },
      { key: 'weekday', label: 'Weekday of "to"', formula: '—' },
    ],
    compute: (v) => {
      const from = parseDate(str(v, 'from'))
      const to = parseDate(str(v, 'to'))
      if (!from || !to) return {}
      const days = Math.round((to.getTime() - from.getTime()) / DAY_MS)
      const months = monthsBetween(from, to)
      return {
        days,
        weeks: days / 7,
        months,
        years: months / 12,
        plus: iso(addDays(from, days)),
        weekday: WEEKDAYS[to.getUTCDay()],
      }
    },
    steps: (v, r) => [
      `From ${str(v, 'from')} to ${str(v, 'to')}`,
      `Elapsed: ${fmt(r.days, 6)} days = ${fmt(r.weeks, 4)} weeks = ${fmt(r.months, 4)} months`,
      `${str(v, 'from')} plus ${fmt(r.days, 6)} days is ${r.plus}`,
      `The second date falls on a ${r.weekday}`,
    ],
    reference: ['Days = (to - from) / 86 400 000', 'Months are counted as whole calendar months'],
    examples: [
      { label: '2026 in full', inputs: { from: '2026-01-01', to: '2026-12-31' } },
      { label: '90 days', inputs: { from: '2026-01-01', to: '2026-04-01' } },
    ],
  },
  {
    id: 'age-calculator',
    category: 'date',
    name: 'Age Calculator',
    tagline: 'Exact age from a birth date',
    keywords: ['age', 'birthday', 'born', 'birthdate', 'years old', 'birth date'],
    inputs: [
      { key: 'birth', label: 'Date of birth', kind: 'text', default: '2000-06-15', placeholder: 'YYYY-MM-DD' },
      { key: 'today', label: 'On', kind: 'text', default: '', placeholder: 'YYYY-MM-DD — blank means today' },
    ],
    outputs: [
      { key: 'years', label: 'Years', formula: 'whole years', emphasis: 'primary' },
      { key: 'months', label: 'Months', formula: 'whole months', emphasis: 'primary' },
      { key: 'days', label: 'Days', formula: 'whole days', emphasis: 'primary' },
      { key: 'totalDays', label: 'Total days lived', formula: '—' },
      { key: 'nextBirthday', label: 'Days to next birthday', formula: '—' },
      { key: 'weekday', label: 'Born on a', formula: '—' },
    ],
    compute: (v) => {
      const birth = parseDate(str(v, 'birth'))
      if (!birth) return {}
      const raw = str(v, 'today').trim()
      const now = new Date()
      const today = raw
        ? parseDate(raw)
        : new Date(Date.UTC(now.getUTCFullYear(), now.getUTCMonth(), now.getUTCDate()))
      if (!today) return {}

      let years = today.getUTCFullYear() - birth.getUTCFullYear()
      let anniversary = addMonths(birth, years * 12)
      if (anniversary.getTime() > today.getTime()) {
        years -= 1
        anniversary = addMonths(birth, years * 12)
      }
      const months = monthsBetween(anniversary, today)
      const monthAnniversary = addMonths(anniversary, months)
      const days = Math.round((today.getTime() - monthAnniversary.getTime()) / DAY_MS)
      const totalDays = Math.round((today.getTime() - birth.getTime()) / DAY_MS)

      let nextBirthday = addMonths(birth, (years + 1) * 12)
      if (nextBirthday.getTime() <= today.getTime()) nextBirthday = addMonths(birth, (years + 2) * 12)

      return {
        years,
        months,
        days,
        totalDays,
        nextBirthday: Math.round((nextBirthday.getTime() - today.getTime()) / DAY_MS),
        weekday: WEEKDAYS[birth.getUTCDay()],
      }
    },
    steps: (_v, r) => [
      `Age: ${fmt(r.years, 6)} years, ${fmt(r.months, 6)} months and ${fmt(r.days, 6)} days`,
      `Total days lived: ${fmt(r.totalDays, 6)}`,
      `Next birthday in ${fmt(r.nextBirthday, 6)} days`,
      `Date of birth fell on a ${r.weekday}`,
    ],
    reference: ['Age = whole calendar years, then whole months, then days'],
    examples: [{ label: 'Born 2000-06-15', inputs: { birth: '2000-06-15', today: '' } }],
  },
]
