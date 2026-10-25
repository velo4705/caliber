import { chromium } from 'playwright'
const b = await chromium.launch()
const p = await b.newPage({ viewport: { width: 1100, height: 700 } })
await p.goto('http://127.0.0.1:5173/#/t/linear-system', { waitUntil: 'networkidle' })
await p.waitForSelector('.matrix-input', { timeout: 10000 })
await p.locator('.matrix-input').screenshot({ path: 'screenshots/matrix-input.png' })
console.log('Screenshot saved')
await b.close()
