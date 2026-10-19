import { useEffect, useMemo, useState } from 'react'
import type { Tool, Values } from '../registry/types'
import { defaultValues } from '../registry'
import { CATEGORY_MAP } from '../registry/categories'
import { navigate } from '../lib/router'
import { formatNumber } from '../lib/format'
import { InputField } from './InputField'
import { OutputGrid } from './OutputGrid'

interface Props {
  tool: Tool
  params: URLSearchParams
}

export function ToolView({ tool, params }: Props) {
  const initial = useMemo(() => {
    const base = defaultValues(tool)
    for (const key of params.keys()) {
      if (key in base) base[key] = params.get(key) ?? ''
    }
    return base
  }, [tool, params])

  const [values, setValues] = useState<Record<string, string>>(initial)
  const [showSteps, setShowSteps] = useState(false)
  const [copied, setCopied] = useState(false)

  useEffect(() => {
    setValues(initial)
    setShowSteps(false)
  }, [tool.id, initial])

  const typedValues = values as Values
  const computed = useMemo(() => {
    try {
      return tool.compute(typedValues) ?? {}
    } catch (e) {
      console.error(`[${tool.id}] compute failed`, e)
      return {}
    }
  }, [tool, values])

  const category = CATEGORY_MAP.get(tool.category)
  const solved = tool.outputs.filter((o) => {
    const v = computed[o.key]
    if (v === undefined || v === '') return false
    return typeof v === 'string' || Number.isFinite(v)
  })

  const set = (key: string, value: string) => {
    setValues((prev) => {
      const next = { ...prev, [key]: value }
      const qs = new URLSearchParams()
      for (const [k, v] of Object.entries(next)) {
        if (v !== '' && v !== (defaultValues(tool)[k] ?? '')) qs.set(k, v)
      }
      history.replaceState(null, '', `#/t/${tool.id}${qs.toString() ? `?${qs}` : ''}`)
      return next
    })
  }

  const reset = () => {
    setValues(defaultValues(tool))
    history.replaceState(null, '', `#/t/${tool.id}`)
  }

  const copy = async () => {
    const text = solved
      .map((o) => `${o.label}: ${formatNumber(computed[o.key], o.precision ?? 6)}${o.unit && !o.unit.startsWith('currency') ? ` ${o.unit}` : ''}`)
      .join('\n')
    try {
      await navigator.clipboard.writeText(`${tool.name}\n${'—'.repeat(tool.name.length)}\n${text}`)
      setCopied(true)
      setTimeout(() => setCopied(false), 1600)
    } catch {
      setCopied(false)
    }
  }

  const steps = showSteps && tool.steps ? tool.steps(typedValues, computed) : []

  return (
    <article className="tool">
      <header className="tool-head">
        <button className="crumb" onClick={() => navigate(`#/c/${tool.category}`)}>
          {category?.icon} {category?.name}
        </button>
        <h1>{tool.name}</h1>
        {tool.tagline && <p className="tool-tagline">{tool.tagline}</p>}
      </header>

      {tool.examples && tool.examples.length > 0 && (
        <div className="examples">
          <span className="examples-label">Try</span>
          {tool.examples.map((ex) => (
            <button key={ex.label} className="chip" onClick={() => setValues({ ...defaultValues(tool), ...ex.inputs })}>
              {ex.label}
            </button>
          ))}
        </div>
      )}

      <div className="tool-body">
        <section className="panel panel-inputs">
          <h2>Inputs</h2>
          <div className="fields">
            {tool.inputs.map((input) => (
              <InputField key={input.key} input={input} value={values[input.key] ?? ''} onChange={(v) => set(input.key, v)} />
            ))}
          </div>
          <button className="btn-ghost" onClick={reset}>
            Reset
          </button>
        </section>

        <section className="panel panel-results">
          <div className="panel-head">
            <h2>Results</h2>
            {solved.length > 0 && (
              <button className="btn-ghost" onClick={copy}>
                {copied ? 'Copied' : 'Copy'}
              </button>
            )}
          </div>
          <OutputGrid outputs={tool.outputs} computed={computed} />
        </section>
      </div>

      {(tool.steps || tool.reference) && (
        <section className="panel panel-extra">
          <div className="extra-head">
            {tool.steps && (
              <button className="tab" onClick={() => setShowSteps((s) => !s)} aria-expanded={showSteps}>
                {showSteps ? 'Hide steps' : 'Show steps'}
              </button>
            )}
            {tool.reference && <span className="ref-list">{tool.reference.join('   ·   ')}</span>}
          </div>
          {showSteps && steps.length > 0 && (
            <ol className="steps">
              {steps.map((s, i) => (
                <li key={i}>{s}</li>
              ))}
            </ol>
          )}
        </section>
      )}
    </article>
  )
}