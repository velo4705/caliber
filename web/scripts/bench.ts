import { compileExpression, evaluateNode, Parser } from '../src/lib/expr'

const p = new Parser('rad')
const src = 'sin(x) * 2 + x^2/3 - 4'
const N = 20000

console.log('expression evaluation benchmark')
console.log(`  ${N} evaluations of "${src}"\n`)

const t0 = performance.now()
for (let i = 0; i < N; i++) { p.setVars({ x: i / 100 }); p.evaluate(src) }
const reparse = performance.now() - t0

const node = compileExpression(src)
const t1 = performance.now()
for (let i = 0; i < N; i++) evaluateNode(node, { x: i / 100 }, 'rad')
const compiled = performance.now() - t1

console.log(`  re-parse every call : ${reparse.toFixed(1)} ms  (${((reparse / N) * 1000).toFixed(1)} µs/eval)`)
console.log(`  compiled AST        : ${compiled.toFixed(1)} ms  (${((compiled / N) * 1000).toFixed(1)} µs/eval)`)
console.log(`  speedup             : ${(reparse / compiled).toFixed(2)}x\n`)

const a = compileExpression(src)
const b = compileExpression(src)
console.log(`  compile cache returns the same node: ${a === b}`)

const heavy = 'sin(x)*cos(x) + atan(x)*sqrt(x) + x^3 - 2*x^2 + 1'
const t2 = performance.now()
for (let i = 0; i < N; i++) { p.setVars({ x: i / 100 }); p.evaluate(heavy) }
const heavyReparse = performance.now() - t2
const heavyNode = compileExpression(heavy)
const t3 = performance.now()
for (let i = 0; i < N; i++) evaluateNode(heavyNode, { x: i / 100 }, 'rad')
const heavyCompiled = performance.now() - t3
console.log(`  heavier expression  : ${(heavyReparse / heavyCompiled).toFixed(2)}x speedup`)

process.exit(reparse / compiled > 1.5 ? 0 : 1)
