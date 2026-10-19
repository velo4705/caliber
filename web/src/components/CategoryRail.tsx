import { CATEGORIES } from '../registry/categories'
import { TOOLS, toolsInCategory } from '../registry'
import { navigate } from '../lib/router'

interface Props {
  active?: string
}

export function CategoryRail({ active }: Props) {
  return (
    <nav className="rail" aria-label="Categories">
      <button className={`rail-item${!active ? ' active' : ''}`} onClick={() => navigate('#/')}>
        <span className="rail-icon">★</span>
        <span className="rail-label">All tools</span>
        <span className="rail-count">{TOOLS.length}</span>
      </button>
      {CATEGORIES.map((c) => {
        const count = toolsInCategory(c.id).length
        if (!count) return null
        return (
          <button
            key={c.id}
            className={`rail-item${active === c.id ? ' active' : ''}`}
            onClick={() => navigate(`#/c/${c.id}`)}
          >
            <span className="rail-icon">{c.icon}</span>
            <span className="rail-label">{c.name}</span>
            <span className="rail-count">{count}</span>
          </button>
        )
      })}
    </nav>
  )
}