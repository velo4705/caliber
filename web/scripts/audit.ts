import { chromium, type Page } from 'playwright'
import { TOOLS } from '../src/registry'
import { CATEGORIES } from '../src/registry/categories'
import { mkdirSync } from 'node:fs'
import { readFile, stat } from 'node:fs/promises'
import { createServer, type Server } from 'node:http'
import { extname, join, resolve } from 'node:path'

const SERVE_DIR = resolve(process.cwd(), process.env.SERVE_DIR ?? 'dist')
const PREFIX = '/caliber'
const TYPES: Record<string, string> = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript',
  '.css': 'text/css',
  '.svg': 'image/svg+xml',
  '.json': 'application/json',
  '.map': 'application/json',
}

let server: Server | undefined
let BASE = process.env.BASE_URL ?? ''

if (!BASE) {
  server = createServer(async (req, res) => {
    const url = (req.url ?? '/').split('?')[0]
    if (!url.startsWith(PREFIX)) {
      res.writeHead(404).end('outside prefix')
      return
    }
    let path = url.slice(PREFIX.length) || '/'
    if (path.endsWith('/')) path += 'index.html'
    const file = join(SERVE_DIR, path)
    try {
      const info = await stat(file)
      if (!info.isFile()) throw new Error('not a file')
      const body = await readFile(file)
      res.writeHead(200, { 'content-type': TYPES[extname(file)] ?? 'application/octet-stream' })
      res.end(body)
    } catch (e) {
      res.writeHead(404, { 'content-type': 'text/plain' })
      res.end(`not found: ${file} (${e instanceof Error ? e.message : e})`)
    }
  })
  await new Promise<void>((r, j) => {
    server!.once('error', (e) => j(new Error(`audit server could not start: ${e.message}`)))
    server!.listen(0, '127.0.0.1', () => r())
  })
  const address = server!.address()
  const port = typeof address === 'object' && address !== null ? address.port : 0
  BASE = `http://127.0.0.1:${port}${PREFIX}`.replace(/\/$/, '')
  console.log(`audit server: ${BASE}  serving ${SERVE_DIR}`)
}

const OUT = resolve(process.cwd(), 'screenshots/audit')
mkdirSync(OUT, { recursive: true })

interface Finding {
  tool: string
  problem: string
  detail?: string
}

const findings: Finding[] = []
const note = (tool: string, problem: string, detail?: string) => {
  findings.push({ tool, problem, detail })
  console.log(`   ✗ ${tool.padEnd(22)} ${problem}${detail ? ` — ${detail}` : ''}`)
}

const browser = await chromium.launch()

async function auditPage(
  page: Page,
  label: string,
  route: string,
  opts: { expectInputs?: boolean; expectOutputs?: boolean; expectCanvas?: boolean; hasDefaults?: boolean },
) {
  const errors: string[] = []
  const onErr = (e: Error) => errors.push(e.message)
  const onCon = (m: { type: () => string; text: () => string }) => {
    if (m.type() === 'error') errors.push(m.text().slice(0, 200))
  }
  page.on('pageerror', onErr)
  page.on('console', onCon)

  try {
    await page.goto(`${BASE}${route}`, { waitUntil: 'networkidle' })
    await page.waitForTimeout(450)
  } catch (e) {
    page.off('pageerror', onErr)
    page.off('console', onCon)
    note(label, 'failed to load', e instanceof Error ? e.message : String(e))
    return
  }

  if (errors.length) note(label, 'console/page error', errors[0])

  const crash = await page.locator('.crash-card').count()
  if (crash) note(label, 'rendered the crash screen')

  const m = await page.evaluate(() => {
    const doc = document.documentElement
    const rail = document.querySelector('.rail') as HTMLElement | null
    const canvas = document.querySelector('.graph-canvas-wrap canvas') as HTMLCanvasElement | null
    const inScroller = (el: Element) => {
      let p = el.parentElement
      while (p) {
        const ox = getComputedStyle(p).overflowX
        if (ox === 'auto' || ox === 'scroll') return true
        p = p.parentElement
      }
      return false
    }
    const overflowing: string[] = []
    document.querySelectorAll('body *').forEach((el) => {
      const r = el.getBoundingClientRect()
      if (r.right > doc.clientWidth + 2 && r.width > 60 && !inScroller(el)) {
        const cls = (el.className?.toString?.() ?? '').slice(0, 30)
        overflowing.push(`${el.tagName}.${cls}`)
      }
    })
    let colours = 0
    if (canvas) {
      const ctx = canvas.getContext('2d')
      if (ctx) {
        const d = ctx.getImageData(0, 0, canvas.width, canvas.height).data
        const set = new Set<string>()
        for (let i = 0; i < d.length; i += 4) set.add(`${d[i]},${d[i + 1]},${d[i + 2]}`)
        colours = set.size
      }
    }
    return {
      overflow: doc.scrollWidth - doc.clientWidth,
      railW: rail ? Math.round(rail.getBoundingClientRect().width) : -1,
      railVisible: rail ? rail.getBoundingClientRect().right > 0 : false,
      overflowing: overflowing.slice(0, 4),
      fields: document.querySelectorAll('.field').length,
      outputs: document.querySelectorAll('.output').length,
      filled: Array.from(document.querySelectorAll('.output-value')).filter(
        (e) => (e.textContent ?? '').trim() !== '—' && (e.textContent ?? '').trim() !== '',
      ).length,
      canvasColours: colours,
      hasCanvas: !!canvas,
      h1: document.querySelector('h1')?.textContent?.trim() ?? '',
    }
  })

  if (m.overflow > 1) note(label, `horizontal overflow of ${m.overflow}px`, m.overflowing.join(', '))
  if (m.overflowing.length) note(label, 'element past the viewport edge', m.overflowing.join(', '))
  if (!m.railVisible || (m.railW > 0 && m.railW < 40)) note(label, `category rail not visible (width ${m.railW})`)

  if (opts.expectInputs && m.fields === 0) note(label, 'no input fields rendered')
  if (opts.expectOutputs && m.outputs > 0 && m.filled === 0 && opts.hasDefaults) {
    note(label, 'every output is empty despite having defaults')
  }
  if (opts.expectCanvas && (!m.hasCanvas || m.canvasColours < 20)) {
    note(label, `canvas looks blank (${m.canvasColours} colours)`)
  }

  page.off('pageerror', onErr)
  page.off('console', onCon)
}

async function runViewport(label: string, width: number, height: number, mobile: boolean) {
  console.log(`\n=== ${label} (${width}x${height}) ===`)
  const ctx = await browser.newContext({
    viewport: { width, height },
    isMobile: mobile,
    hasTouch: mobile,
  })
  const page = await ctx.newPage()

  await auditPage(page, 'home', '/', { expectInputs: false, expectOutputs: false })

  for (const c of CATEGORIES) {
    await auditPage(page, `category:${c.id}`, `/#/c/${c.id}`, {})
  }

  for (const t of TOOLS) {
    const isChart = t.kind === 'chart'
    const hasDefaults = isChart ? false : t.inputs.some((i) => i.default !== undefined && i.default !== '')
    await auditPage(page, `tool:${t.id}`, `/#/t/${t.id}`, {
      expectInputs: !isChart,
      expectOutputs: !isChart,
      expectCanvas: isChart,
      hasDefaults,
    })
  }

  await auditPage(page, 'bad route', '/#/t/does-not-exist', {})
  await ctx.close()
}

await runViewport('desktop', 1440, 900, false)
await runViewport('laptop', 1280, 800, false)
await runViewport('tablet', 820, 1180, true)
await runViewport('phone', 390, 844, true)

await browser.close()
server?.close()

console.log(`\n${'='.repeat(60)}`)
console.log(`findings: ${findings.length}`)
for (const f of findings) console.log(`  ${f.tool}: ${f.problem}${f.detail ? ` (${f.detail})` : ''}`)
process.exit(findings.length ? 1 : 0)
