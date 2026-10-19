import { createServer } from 'node:http'
import { readFile, stat } from 'node:fs/promises'
import { extname, join, resolve } from 'node:path'

const DIST = resolve(process.cwd(), 'dist')
const PREFIX = '/caliber'
const TYPES: Record<string, string> = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript',
  '.css': 'text/css',
  '.svg': 'image/svg+xml',
  '.json': 'application/json',
  '.map': 'application/json',
  '.png': 'image/png',
  '.ico': 'image/x-icon',
}

const server = createServer(async (req, res) => {
  const url = (req.url ?? '/').split('?')[0]
  if (!url.startsWith(PREFIX)) {
    res.writeHead(404).end('outside prefix')
    return
  }
  let path = url.slice(PREFIX.length) || '/'
  if (path.endsWith('/')) path += 'index.html'
  const file = join(DIST, path)
  try {
    const info = await stat(file)
    if (!info.isFile()) throw new Error('not a file')
    const body = await readFile(file)
    res.writeHead(200, { 'content-type': TYPES[extname(file)] ?? 'application/octet-stream' })
    res.end(body)
  } catch {
    res.writeHead(404, { 'content-type': 'text/plain' })
    res.end('not found')
  }
})

server.listen(4321, '127.0.0.1', () => console.log('serving dist/ at http://127.0.0.1:4321' + PREFIX + '/'))
