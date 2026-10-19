import { validateTools, TOOLS } from '../src/registry'
import { CATEGORIES, CATEGORY_MAP } from '../src/registry/categories'
import { searchTools } from '../src/registry'
import { Parser } from '../src/lib/expr'

const problems = validateTools(TOOLS)
console.log(`tools registered: ${TOOLS.length}`)
console.log(`categories used: ${new Set(TOOLS.map((t) => t.category)).size} / ${CATEGORIES.length}`)
for (const c of CATEGORIES) {
  const n = TOOLS.filter((t) => t.category === c.id).length
  console.log(`  ${c.id.padEnd(12)} ${n}`)
}
if (problems.length) {
  console.log(`\nVALIDATION PROBLEMS (${problems.length}):`)
  for (const p of problems) console.log('  ! ' + p)
} else {
  console.log('\nregistry validation: PASS')
}

const p = new Parser('rad')
const exprTests: [string, number][] = [
  ['2+3*4', 14],
  ['(2+3)*4', 20],
  ['2^3^2', 512],
  ['-2^2', -4],
  ['2^-1', 0.5],
  ['10%3', 1],
  ['3!', 6],
  ['sqrt(16)', 4],
  ['2(3+4)', 14],
  ['pi', Math.PI],
  ['gcd(12,18)', 6],
  ['lcm(4,6)', 12],
  ['1/3', 1 / 3],
]
let exprFail = 0
console.log('\nexpression engine:')
for (const [src, want] of exprTests) {
  const got = p.evaluate(src)
  const ok = Math.abs(got - want) < 1e-9
  if (!ok) exprFail++
  console.log(`  ${ok ? 'ok  ' : 'FAIL'} ${src} = ${got}${ok ? '' : ` (want ${want})`}`)
}
const deg = new Parser('deg')
const sinDeg = deg.evaluate('sin(30)')
console.log(`  ${Math.abs(sinDeg - 0.5) < 1e-9 ? 'ok  ' : 'FAIL'} sin(30) in degrees = ${sinDeg}`)

console.log('\nsearch:')
for (const q of ['ohm', 'beam', 'pH', 'regression', 'hex']) {
  const hits = searchTools(q)
  console.log(`  "${q}" -> ${hits.length} hits, top: ${hits[0]?.tool.name ?? 'none'}`)
}

void CATEGORY_MAP
process.exit(problems.length + exprFail ? 1 : 0)