export interface Route {
  type: 'home' | 'category' | 'tool'
  category?: string
  toolId?: string
  params: URLSearchParams
}

export function parseHash(hash: string): Route {
  const raw = hash.replace(/^#\/?/, '')
  const [path, query = ''] = raw.split('?')
  const params = new URLSearchParams(query)
  const parts = (path ?? '').split('/').filter(Boolean)

  if (parts[0] === 't' && parts[1]) {
    return { type: 'tool', toolId: parts[1], params }
  }
  if (parts[0] === 'c' && parts[1]) {
    return { type: 'category', category: parts[1], params }
  }
  return { type: 'home', params }
}

export function buildHash(route: Route): string {
  const qs = route.params.toString()
  const base =
    route.type === 'tool' && route.toolId
      ? `/t/${route.toolId}`
      : route.type === 'category' && route.category
        ? `/c/${route.category}`
        : '/'
  return `#${base}${qs ? `?${qs}` : ''}`
}

export function navigate(hash: string) {
  if (window.location.hash !== hash) window.location.hash = hash
}