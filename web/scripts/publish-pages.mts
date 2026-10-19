import { cp, mkdir, rm, writeFile } from 'node:fs/promises'
import { join, resolve } from 'node:path'

const dist = resolve(process.cwd(), 'dist')
const out = resolve(process.cwd(), '..', 'docs')

await rm(out, { recursive: true, force: true })
await mkdir(out, { recursive: true })
await cp(dist, out, { recursive: true })

// Tell GitHub Pages to skip Jekyll and serve these files verbatim. Without it
// the Pages builder still runs Jekyll, which needs a source dir and a theme
// and fails on a plain static build.
await writeFile(join(out, '.nojekyll'), '')

console.log(`published dist/ -> ${out}`)
