import type { ToolOutput, Computed } from '../registry/types'
import { formatNumber } from '../lib/format'

interface Props {
  outputs: ToolOutput[]
  computed: Computed
  unitMode?: boolean
}

export function OutputGrid({ outputs, computed }: Props) {
  return (
    <div className="outputs">
      {outputs.map((o) => {
        const raw = computed[o.key]
        const missing = raw === undefined || raw === '' || (typeof raw === 'number' && !Number.isFinite(raw))
        if (o.optional && missing) return null
        return (
          <div key={o.key} className={`output${o.emphasis === 'primary' ? ' output-primary' : ''}`}>
            <div className="output-label">
              {o.label}
              {o.unit && !o.unit.startsWith('currency') && <span className="output-unit">{o.unit}</span>}
            </div>
            <div className={`output-value${missing ? ' missing' : ''}`}>{formatNumber(raw, o.precision ?? 6)}</div>
            {o.formula && <div className="output-formula">{o.formula}</div>}
            {missing && o.needs && <div className="output-needs">needs {o.needs}</div>}
          </div>
        )
      })}
    </div>
  )
}