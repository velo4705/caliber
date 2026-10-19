import { chromium, type ConsoleMessage, type Page } from 'playwright'
import { mkdirSync } from 'node:fs'
import { resolve } from 'node:path'

const BASE = process.env.BASE_URL ?? 'http://127.0.0.1:5199'
const OUT = resolve(process.cwd(), 'screenshots')
mkdirSync(OUT, { recursive: true })

const problems: string[] = []
const note = (msg: string) => console.log(`  ${msg}`)
const fail = (msg: string) => {
  problems.push(msg)
  console.log(`  FAIL ${msg}`)
}
const ok = (msg: string) => console.log(`  ok   ${msg}`)

function watch(page: Page, label: string) {
  page.on('console', (m: ConsoleMessage) => {
    if (m.type() === 'error') fail(`[${label}] console error: ${m.text()}`)
    if (m.type() === 'warning' && m.text().includes('registry')) fail(`[${label}] registry warning: ${m.text()}`)
  })
  page.on('pageerror', (e) => fail(`[${label}] page error: ${e.message}`))
}

async function shot(page: Page, name: string) {
  await page.screenshot({ path: resolve(OUT, `${name}.png`) })
}

const browser = await chromium.launch()
const context = await browser.newContext({ viewport: { width: 1400, height: 900 } })
const page = await context.newPage()
watch(page, 'desktop')

console.log('\nhome')
await page.goto(BASE, { waitUntil: 'networkidle' })
await page.waitForSelector('.home-grid')
const cards = await page.locator('.home-card').count()
cards >= 10 ? ok(`${cards} category cards render`) : fail(`only ${cards} category cards`)
const railItems = await page.locator('.rail-item').count()
railItems >= 11 ? ok(`${railItems} rail items render`) : fail(`only ${railItems} rail items`)
await shot(page, '01-home')

console.log('\nsearch')
await page.fill('.searchbar input', 'ohm')
await page.waitForSelector('.tool-card', { timeout: 3000 })
const results = await page.locator('.tool-card').count()
results >= 1 ? ok(`"ohm" returns ${results} result(s)`) : fail('search returned nothing')
const firstName = await page.locator('.tool-card-name').first().textContent()
firstName?.includes('Ohm') ? ok(`top hit is "${firstName}"`) : fail(`top hit was "${firstName}"`)
await shot(page, '02-search')

console.log('\nsolver tool')
await page.goto(`${BASE}#/t/ohm?V=12&R=470`, { waitUntil: 'networkidle' })
await page.waitForSelector('.output-value')
const values = await page.locator('.output-value').allTextContents()
const current = values.find((v) => v.includes('0.0255'))
current ? ok(`Ohm's law computes I = ${current}`) : fail(`expected I ≈ 0.0255, got ${values.join(' | ')}`)
const power = values.find((v) => v.includes('0.306'))
power ? ok(`power P = ${power}`) : fail(`expected P ≈ 0.306, got ${values.join(' | ')}`)
await shot(page, '03-solver-ohm')

console.log('\nurl state')
await page.fill('.field input', '24')
await page.waitForTimeout(250)
const hash = page.url().split('#')[1] ?? ''
hash.includes('V=24') ? ok(`url tracks the value: #${hash}`) : fail(`url did not update, got #${hash}`)

console.log('\nexpression evaluator')
await page.goto(`${BASE}#/t/expression-eval`, { waitUntil: 'networkidle' })
await page.waitForSelector('.output-value')
await page.fill('.field input', '2^3^2')
await page.waitForTimeout(300)
const exprValue = (await page.locator('.output-value').first().textContent()) ?? ''
exprValue.includes('512') ? ok(`2^3^2 evaluates to ${exprValue}`) : fail(`expected 512, got ${exprValue}`)

console.log('\ngrapher')
await page.goto(`${BASE}#/t/graph-2d`, { waitUntil: 'networkidle' })
await page.waitForSelector('.graph-canvas-wrap canvas')
await page.waitForTimeout(700)
const canvasBox = await page.locator('.graph-canvas-wrap canvas').boundingBox()
canvasBox && canvasBox.width > 300 ? ok(`canvas sized ${Math.round(canvasBox.width)}x${Math.round(canvasBox.height)}`) : fail('canvas has no size')
const seriesText = await page.locator('.graph-count').textContent()
seriesText?.includes('2 series') ? ok(`toolbar reports "${seriesText}"`) : fail(`expected 2 series, got "${seriesText}"`)
await shot(page, '04-grapher')

const canvas = page.locator('.graph-canvas-wrap canvas')
const painted = await page.evaluate(() => {
  const c = document.querySelector('.graph-canvas-wrap canvas') as HTMLCanvasElement
  const ctx = c.getContext('2d')!
  const d = ctx.getImageData(0, 0, c.width, c.height).data
  const colours = new Set<string>()
  for (let i = 0; i < d.length; i += 4) colours.add(`${d[i]},${d[i + 1]},${d[i + 2]}`)
  return colours.size
})
painted > 20 ? ok(`canvas has ${painted} distinct colours — curves are drawn`) : fail(`canvas looks blank (${painted} colours)`)

console.log('\nzoom')
const before = await page.locator('.graph-axes').textContent()
await canvas.hover({ position: { x: 300, y: 200 } })
await page.mouse.wheel(0, -300)
await page.waitForTimeout(400)
const after = await page.locator('.graph-axes').textContent()
before !== after ? ok(`wheel zoom changed the viewport`) : fail('wheel zoom did nothing')

console.log('\nparameter sweep')
await page.goto(`${BASE}#/t/graph-2d`, { waitUntil: 'networkidle' })
await page.waitForSelector('.graph-fn input')
const inputs = page.locator('.graph-fn input')
await inputs.nth(0).fill('a = [1...5]')
await inputs.nth(1).fill('a*x^2 - 2*a')
await inputs.nth(2).fill('')
await page.waitForTimeout(700)
const sweepText = await page.locator('.graph-count').textContent()
sweepText?.includes('5 series') ? ok(`sweep of a=[1...5] yields "${sweepText}"`) : fail(`expected 5 series, got "${sweepText}"`)
const paramStyled = await page.locator('.graph-fn.is-param').count()
paramStyled === 1 ? ok('the parameter slot is styled distinctly') : fail(`expected 1 param slot, got ${paramStyled}`)
await shot(page, '05-sweep')

console.log('\nderivative overlay')
await page.locator('.graph-check input').first().check()
await page.waitForTimeout(500)
await shot(page, '06-derivative')

console.log('\nintegral shading')
await page.locator('.graph-check input').nth(1).check()
await page.waitForSelector('.graph-integral')
await page.fill('.graph-integral input >> nth=0', '0')
await page.fill('.graph-integral input >> nth=1', '2')
await page.waitForTimeout(600)
const integral = (await page.locator('.graph-integral-value').textContent()) ?? ''
const integralHasValue = /∫ = -?[\d.]/.test(integral)
integralHasValue ? ok(`integral reported "${integral.trim()}"`) : fail(`integral not reported, got "${integral}"`)
await shot(page, '07-integral')

console.log('\ntheme')
await page.click('.theme-toggle')
await page.waitForTimeout(400)
const theme = await page.evaluate(() => document.documentElement.dataset.theme)
theme === 'dark' ? ok('dark theme applied') : fail(`theme is "${theme}"`)
await shot(page, '08-dark')

console.log('\nmobile')
const mobile = await browser.newContext({ viewport: { width: 390, height: 844 }, isMobile: true, hasTouch: true })
const mp = await mobile.newPage()
watch(mp, 'mobile')
await mp.goto(`${BASE}#/t/graph-2d`, { waitUntil: 'networkidle' })
await mp.waitForSelector('.graph-canvas-wrap canvas')
await mp.waitForTimeout(700)
const overflow = await mp.evaluate(() => document.documentElement.scrollWidth - document.documentElement.clientWidth)
overflow <= 1 ? ok('no horizontal overflow at 390px') : fail(`horizontal overflow of ${overflow}px`)
const railHorizontal = await mp.evaluate(() => {
  const rail = document.querySelector('.rail')
  return rail ? getComputedStyle(rail).flexDirection : ''
})
railHorizontal === 'row' ? ok('category rail becomes a horizontal chip row') : fail(`rail flex-direction is "${railHorizontal}"`)
await mp.screenshot({ path: resolve(OUT, '09-mobile-graph.png'), fullPage: true })
await mp.goto(`${BASE}#/t/ohm?V=12&R=470`, { waitUntil: 'networkidle' })
await mp.waitForTimeout(400)
await mp.screenshot({ path: resolve(OUT, '10-mobile-solver.png'), fullPage: true })

await browser.close()

console.log(`\n${problems.length === 0 ? 'ALL VISUAL CHECKS PASSED' : `${problems.length} PROBLEM(S):`}`)
for (const p of problems) console.log(`  - ${p}`)
process.exit(problems.length ? 1 : 0)
