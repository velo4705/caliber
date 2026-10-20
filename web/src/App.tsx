import { useEffect, useMemo, useRef, useState } from 'react'
import { useRoute, useTheme } from './lib/hooks'
import { navigate } from './lib/router'
import { TOOLS, searchTools, toolsInCategory } from './registry'
import { CATEGORIES, CATEGORY_MAP } from './registry/categories'
import { CategoryRail } from './components/CategoryRail'
import { ToolView } from './components/ToolView'
import { GraphView } from './components/GraphView'
import { SurfaceView } from './components/SurfaceView'
import { ErrorBoundary } from './components/ErrorBoundary'

export default function App() {
  const route = useRoute()
  const [theme, toggleTheme] = useTheme()
  const [query, setQuery] = useState('')
  const searchRef = useRef<HTMLInputElement>(null)
  const topbarRef = useRef<HTMLElement>(null)

  useEffect(() => {
    const bar = topbarRef.current
    if (!bar) return
    const apply = () => {
      document.documentElement.style.setProperty('--topbar-h', `${Math.round(bar.getBoundingClientRect().height)}px`)
    }
    apply()
    const observer = new ResizeObserver(apply)
    observer.observe(bar)
    window.addEventListener('resize', apply)
    return () => {
      observer.disconnect()
      window.removeEventListener('resize', apply)
    }
  }, [])

  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      const target = e.target as HTMLElement | null
      const typing = target && /^(INPUT|SELECT|TEXTAREA)$/.test(target.tagName)
      if ((e.key === 'k' && (e.metaKey || e.ctrlKey)) || (e.key === '/' && !typing)) {
        e.preventDefault()
        searchRef.current?.focus()
      }
      if (e.key === 'Escape' && document.activeElement === searchRef.current) {
        searchRef.current?.blur()
      }
    }
    window.addEventListener('keydown', onKey)
    return () => window.removeEventListener('keydown', onKey)
  }, [])

  useEffect(() => {
    if (route.type !== 'home') setQuery('')
  }, [route.type, route.category])

  const tool = route.type === 'tool' && route.toolId ? TOOLS.find((t) => t.id === route.toolId) : undefined

  const results = useMemo(() => {
    if (!query.trim() && route.type === 'home') return null
    const category = route.type === 'category' ? route.category : undefined
    return searchTools(query, category)
  }, [query, route.type, route.category])

  return (
    <div className="app">
      <header className="topbar" ref={topbarRef}>
        <button className="brand" onClick={() => navigate('#/')}>
          <img className="brand-mark" src={`${import.meta.env.BASE_URL}caliber.svg`} alt="" width={30} height={30} />
          <span className="brand-name">
            Caliber
            <small>universal STEM solver</small>
          </span>
        </button>

        <div className="searchbar">
          <span className="search-icon" aria-hidden>
            ⌕
          </span>
          <input
            ref={searchRef}
            type="search"
            placeholder={`Search ${TOOLS.length}+ solvers — try “ohm”, “beam”, “pH”, “regression”`}
            value={query}
            onChange={(e) => setQuery(e.target.value)}
            onKeyDown={(e) => {
              if (e.key === 'Enter' && results && results.length) navigate(`#/t/${results[0]!.tool.id}`)
            }}
            aria-label="Search solvers"
          />
          <kbd>/</kbd>
        </div>

        <button className="theme-toggle" onClick={toggleTheme} aria-label="Toggle theme" title="Toggle theme">
          {theme === 'dark' ? '☀' : '☾'}
        </button>
      </header>

      <div className="layout">
        <CategoryRail active={route.type === 'category' ? route.category : undefined} />

        <main className="main">
          <ErrorBoundary label="This tool">
            {tool ? (
              tool.kind === 'chart' ? (
                tool.graph.mode === 'surface' ? (
                  <SurfaceView tool={tool} params={route.params} />
                ) : (
                  <GraphView tool={tool} params={route.params} />
                )
              ) : (
                <ToolView tool={tool} params={route.params} />
              )
            ) : results ? (
            <section className="listing">
              <h1 className="page-title">
                {query.trim()
                  ? `${results.length} result${results.length === 1 ? '' : 's'} for “${query.trim()}”`
                  : CATEGORY_MAP.get(route.category ?? '')?.name ?? 'All tools'}
              </h1>
              {results.length === 0 ? (
                <p className="empty">No solvers match that. Try a shorter word, like “force” or “log”.</p>
              ) : (
                <div className="cards">
                  {results.map(({ tool: t }) => (
                    <ToolCard key={t.id} id={t.id} name={t.name} tagline={t.tagline} category={t.category} />
                  ))}
                </div>
              )}
            </section>
          ) : (
            <Home />
          )}
          </ErrorBoundary>
        </main>
      </div>

      <footer className="footer">
        <span>Caliber · {TOOLS.length} solvers · everything runs in your browser</span>
      </footer>
    </div>
  )
}

function ToolCard({ id, name, tagline, category }: { id: string; name: string; tagline?: string; category: string }) {
  const cat = CATEGORY_MAP.get(category)
  return (
    <button className="tool-card" onClick={() => navigate(`#/t/${id}`)}>
      <div className="tool-card-top">
        <span className="tool-card-icon">{cat?.icon}</span>
        <span className="tool-card-name">{name}</span>
      </div>
      {tagline && <div className="tool-card-tagline">{tagline}</div>}
    </button>
  )
}

function Home() {
  return (
    <section className="home">
      <h1 className="page-title">What are you solving?</h1>
      <p className="page-sub">
        {TOOLS.length} solvers across {CATEGORIES.filter((c) => toolsInCategory(c.id).length).length} fields. No sign-in, no
        server — every calculation happens on your device.
      </p>
      <div className="home-grid">
        {CATEGORIES.map((c) => {
          const tools = toolsInCategory(c.id)
          if (!tools.length) return null
          return (
            <div key={c.id} className="home-card">
              <button className="home-card-head" onClick={() => navigate(`#/c/${c.id}`)}>
                <span className="home-card-icon">{c.icon}</span>
                <span>
                  <strong>{c.name}</strong>
                  <small>{c.blurb}</small>
                </span>
              </button>
              <div className="home-card-tools">
                {tools.slice(0, 5).map((t) => (
                  <button key={t.id} className="link" onClick={() => navigate(`#/t/${t.id}`)}>
                    {t.name}
                  </button>
                ))}
                {tools.length > 5 && <span className="more">+{tools.length - 5} more</span>}
              </div>
            </div>
          )
        })}
      </div>
    </section>
  )
}