import { chromium } from 'playwright'
const b = await chromium.launch()
const p = await b.newPage({ viewport: { width: 1100, height: 700 } })
await p.goto('http://127.0.0.1:5173/#/t/ohm', { waitUntil: 'networkidle' })
await p.waitForSelector('.field input', { timeout: 10000 })
await p.locator('.field input').first().screenshot({ path: 'screenshots/ohm-fixed.png' })
console.log('Ohm screenshot saved')
await b.close()
