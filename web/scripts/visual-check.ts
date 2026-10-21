import { chromium, type ConsoleMessage, type Page } from 'playwright'
import { mkdirSync } from 'node:fs'
import { resolve } from 'node:path'

const BASE = process.env.BASE_URL ?? 'http://127.0.0.1:5173'
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

console.log('\nbrand and favicon')
const iconHref = await page.getAttribute('link[rel="icon"]', 'href')
const iconStatus = await page.evaluate(async (href) => {
  const res = await fetch(new URL(href, document.baseURI))
  return res.status
}, iconHref ?? '')
iconHref?.includes('caliber.svg') ? ok(`favicon points at ${iconHref}`) : fail(`favicon href is "${iconHref}"`)
iconStatus === 200 ? ok('favicon loads') : fail(`favicon returned ${iconStatus}`)

const iconIsRelative = !iconHref?.startsWith('/')
iconIsRelative ? ok('favicon path is relative, so it survives subpath hosting') : fail('favicon path is absolute')

const markSrc = await page.getAttribute('.brand-mark', 'src')
markSrc?.includes('caliber.svg') ? ok(`brand mark uses ${markSrc}`) : fail(`brand mark src is "${markSrc}"`)
const markBox = await page.locator('.brand-mark').boundingBox()
markBox && markBox.width >= 24 ? ok(`brand mark renders at ${Math.round(markBox.width)}px`) : fail('brand mark has no size')
const markLoaded = await page.evaluate(() => {
  const img = document.querySelector('.brand-mark') as HTMLImageElement
  return img.complete && img.naturalWidth > 0
})
markLoaded ? ok('brand mark image actually loaded') : fail('brand mark image failed to load')

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

console.log('\nintercepts')
await page.goto(`${BASE}#/t/graph-2d`, { waitUntil: 'networkidle' })
await page.waitForSelector('.graph-fn input')
await page.locator('.graph-fn input').nth(0).fill('x^2 - 4')
await page.locator('.graph-fn input').nth(1).fill('')
await page.locator('.graph-fn input').nth(2).fill('')
await page.waitForTimeout(400)
await page.locator('.graph-check input').nth(1).check()
await page.waitForTimeout(600)
const interceptCanvas = await page.evaluate(() => {
  const c = document.querySelector('.graph-canvas-wrap canvas') as HTMLCanvasElement
  const ctx = c.getContext('2d')!
  const d = ctx.getImageData(0, 0, c.width, c.height).data
  const colours = new Set<string>()
  for (let i = 0; i < d.length; i += 4) colours.add(`${d[i]},${d[i + 1]},${d[i + 2]}`)
  return colours.size
})
interceptCanvas > 20 ? ok(`intercepts overlay painted (${interceptCanvas} colours)`) : fail('intercepts did not paint')
await shot(page, '06b-intercepts')

console.log('\nintegral shading')
await page.goto(`${BASE}#/t/graph-2d`, { waitUntil: 'networkidle' })
await page.waitForSelector('.graph-fn input')
await page.locator('.graph-fn input').nth(0).fill('x^2')
await page.locator('.graph-fn input').nth(1).fill('')
await page.locator('.graph-fn input').nth(2).fill('')
await page.waitForTimeout(300)
await page.locator('.graph-check input').nth(1).check()
await page.waitForTimeout(300)
await page.locator('.graph-check input').nth(2).check()
await page.waitForSelector('.graph-integral')
await page.fill('.graph-integral input >> nth=0', '0')
await page.fill('.graph-integral input >> nth=1', '2')
await page.waitForTimeout(600)
const integral = (await page.locator('.graph-integral-value').textContent()) ?? ''
const integralHasValue = /∫ = -?[\d.]/.test(integral)
integralHasValue ? ok(`integral reported "${integral.trim()}"`) : fail(`integral not reported, got "${integral}"`)
await shot(page, '07-integral')

console.log('\n3d surface')
await page.goto(`${BASE}#/t/graph-3d-surface`, { waitUntil: 'networkidle' })
await page.waitForSelector('.graph-canvas-wrap canvas')
await page.waitForTimeout(900)
const surfaceColours = await page.evaluate(() => {
  const c = document.querySelector('.graph-canvas-wrap canvas') as HTMLCanvasElement
  const ctx = c.getContext('2d')!
  const d = ctx.getImageData(0, 0, c.width, c.height).data
  const colours = new Set<string>()
  for (let i = 0; i < d.length; i += 4) colours.add(`${d[i]},${d[i + 1]},${d[i + 2]}`)
  return colours.size
})
surfaceColours > 100 ? ok(`3D surface painted with ${surfaceColours} colours`) : fail(`3D surface looks blank (${surfaceColours} colours)`)
const quadText = await page.locator('.graph-count').textContent()
quadText?.includes('quads') ? ok(`toolbar reports "${quadText}"`) : fail(`expected a quad count, got "${quadText}"`)
await shot(page, '11-surface')

console.log('\n3d rotation')
const surface = page.locator('.graph-canvas-wrap canvas')
const box = await surface.boundingBox()
if (box) {
  await page.mouse.move(box.x + box.width / 2, box.y + box.height / 2)
  await page.mouse.down()
  await page.mouse.move(box.x + box.width / 2 + 120, box.y + box.height / 2 + 40, { steps: 8 })
  await page.mouse.up()
  await page.waitForTimeout(500)
}
const rotatedColours = await page.evaluate(() => {
  const c = document.querySelector('.graph-canvas-wrap canvas') as HTMLCanvasElement
  const ctx = c.getContext('2d')!
  const d = ctx.getImageData(0, 0, c.width, c.height).data
  const colours = new Set<string>()
  for (let i = 0; i < d.length; i += 4) colours.add(`${d[i]},${d[i + 1]},${d[i + 2]}`)
  return colours.size
})
rotatedColours > 100 ? ok('surface still renders after rotation') : fail('rotation broke the render')
await shot(page, '12-surface-rotated')

console.log('\ngraph editing robustness')
await page.goto(`${BASE}#/t/graph-2d`, { waitUntil: 'networkidle' })
await page.waitForSelector('.graph-fn input')
const graphInput = page.locator('.graph-fn input').nth(0)
for (const partial of ['sin(', 'x^', '((', '1+', 'sin(x', 'a = [']) {
  await graphInput.fill(partial)
  await page.waitForTimeout(200)
}
const survived = await page.locator('.graph-canvas-wrap canvas').count()
survived === 1 ? ok('partial expressions do not crash the grapher') : fail('the grapher crashed on a partial expression')
const errorShown = await page.locator('.graph-error').count()
errorShown > 0 ? ok('the syntax error is reported inline') : fail('no inline error for an invalid expression')
const fieldWidth = await page.evaluate(() => {
  const i = document.querySelector('.graph-fn input') as HTMLInputElement
  return Math.round(i.getBoundingClientRect().width)
})
fieldWidth >= 60
  ? ok(`the field stays readable when the error shows (${fieldWidth}px)`)
  : fail(`the field collapsed to ${fieldWidth}px when the error appeared — the text is invisible`)
const fieldValue = await page.locator('.graph-fn input').first().inputValue()
fieldValue.length > 0 ? ok(`the expression is still in the field ("${fieldValue}")`) : fail('the expression was lost')
await graphInput.fill('sin(x)')
await page.waitForTimeout(300)
const errorCleared = await page.locator('.graph-error').count()
errorCleared === 0 ? ok('the error clears once the expression is valid') : fail('the error did not clear')

await page.goto(`${BASE}#/t/graph-3d-surface`, { waitUntil: 'networkidle' })
await page.waitForSelector('.graph-fn input')
const surfaceInput = page.locator('.graph-fn input').nth(0)
for (const partial of ['sin(', 'x^', '((']) {
  await surfaceInput.fill(partial)
  await page.waitForTimeout(200)
}
const surfaceAlive = await page.locator('.graph-canvas-wrap canvas').count()
surfaceAlive === 1 ? ok('partial expressions do not crash the 3D surface') : fail('the 3D surface crashed on a partial expression')

console.log('\ncrash handling')
await page.goto(`${BASE}#/t/graph-2d`, { waitUntil: 'networkidle' })
await page.waitForSelector('.graph-fn input')
const crashResult = await page.evaluate(() => {
  const el = document.querySelector('.graph-fn input')
  if (!el) return 'no input'
  const setter = Object.getOwnPropertyDescriptor(window.HTMLInputElement.prototype, 'value')?.set
  const event = new Event('input', { bubbles: true })
  try {
    setter?.call(el, 'sin(x)xcos(x)')
    el.dispatchEvent(event)
  } catch {
    return 'threw'
  }
  return 'ok'
})
await page.waitForTimeout(400)
const errorText = await page.locator('.graph-error').first().textContent().catch(() => null)
errorText?.includes('Unknown function') ? ok(`a bad expression reports "${errorText}"`) : fail(`no inline error, got "${errorText}" (${crashResult})`)
const stillAlive = await page.locator('.graph-canvas-wrap canvas').count()
stillAlive === 1 ? ok('the view survives an invalid expression') : fail('the view died on an invalid expression')

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
