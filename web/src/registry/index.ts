import type { SolverTool, Tool } from './types'
import { CATEGORY_MAP } from './categories'
import { mathTools } from '../tools/math'
import { statisticsTools } from '../tools/statistics'
import { geometryTools } from '../tools/geometry'
import { physicsTools } from '../tools/physics'
import { chemistryTools } from '../tools/chemistry'
import { electricalTools } from '../tools/electrical'
import { engineeringTools } from '../tools/engineering'
import { computingTools } from '../tools/computing'
import { financeTools } from '../tools/finance'
import { unitTools } from '../tools/units'
import { graphTools } from '../tools/graphs'
import { dateTools } from '../tools/date'

export const TOOLS: Tool[] = [
  ...mathTools,
  ...statisticsTools,
  ...geometryTools,
  ...physicsTools,
  ...chemistryTools,
  ...electricalTools,
  ...engineeringTools,
  ...computingTools,
  ...financeTools,
  ...unitTools,
  ...graphTools,
  ...dateTools,
]

export const TOOL_MAP = new Map(TOOLS.map((t) => [t.id, t]))

export function validateTools(tools: Tool[]): string[] {
  const problems: string[] = []
  const warnings: string[] = []
  const seen = new Set<string>()
  const names = new Map<string, string>()
  const shapes = new Map<string, string>()

  for (const tool of tools) {
    if (seen.has(tool.id)) problems.push(`Duplicate tool id: ${tool.id}`)
    seen.add(tool.id)
    if (!CATEGORY_MAP.has(tool.category)) {
      problems.push(`${tool.id}: unknown category "${tool.category}"`)
    }

    const nameKey = tool.name.trim().toLowerCase()
    const nameOwner = names.get(nameKey)
    if (nameOwner) {
      problems.push(`${tool.id}: duplicate name "${tool.name}" — already used by "${nameOwner}"`)
    } else {
      names.set(nameKey, tool.id)
    }

    if (tool.kind === 'chart') {
      if (!tool.graph.functions.length) problems.push(`${tool.id}: chart has no functions`)
      for (const fn of tool.graph.functions) {
        if (!fn.key) problems.push(`${tool.id}: function with empty key`)
      }
      const modes = new Set(['cartesian', 'polar', 'parametric', 'implicit', 'surface'])
      if (!modes.has(tool.graph.mode)) problems.push(`${tool.id}: unknown graph mode`)
      continue
    }

    if (!tool.inputs.length) problems.push(`${tool.id}: no inputs`)
    if (!tool.outputs.length) problems.push(`${tool.id}: no outputs`)
    const inputKeys = new Set(tool.inputs.map((i) => i.key))
    const outputKeys = new Set(tool.outputs.map((o) => o.key))
    if (inputKeys.size !== tool.inputs.length) problems.push(`${tool.id}: duplicate input key`)
    if (outputKeys.size !== tool.outputs.length) problems.push(`${tool.id}: duplicate output key`)

    const shape =
      `${tool.category}|${[...inputKeys].sort().join(',')}|${[...outputKeys].sort().join(',')}` +
      `|${[...tool.inputs.flatMap((i) => (i.options ?? []).map((o) => o.value))].sort().join(',')}`
    const shapeOwner = shapes.get(shape)
    if (shapeOwner) {
      problems.push(`${tool.id}: same inputs, outputs and options as "${shapeOwner}" — that is the same solver twice`)
    } else {
      shapes.set(shape, tool.id)
    }

    const probe: Record<string, string> = {}
    for (const i of tool.inputs) probe[i.key] = i.default ?? '1'
    let result: Record<string, unknown>
    try {
      result = tool.compute(probe) ?? {}
    } catch (e) {
      problems.push(`${tool.id}: compute() threw — ${e instanceof Error ? e.message : String(e)}`)
      continue
    }
    for (const key of Object.keys(result)) {
      if (!outputKeys.has(key)) {
        problems.push(`${tool.id}: compute() returned "${key}" which is not a declared output`)
      }
    }
    for (const key of outputKeys) {
      if (!(key in result)) {
        problems.push(`${tool.id}: declared output "${key}" is never returned by compute()`)
      }
    }

    const filled = tool.outputs.filter((o) => {
      const v = result[o.key]
      return v !== undefined && v !== '' && !(typeof v === 'number' && !Number.isFinite(v))
    })
    if (filled.length === 0) {
      problems.push(
        `${tool.id}: produces no result from its default inputs — give the inputs defaults so the tool shows a worked example on arrival`,
      )
    }
  }

  for (const [name, id] of names) {
    for (const [other, otherId] of names) {
      if (id === otherId) continue
      if (other.length > name.length && other.includes(name)) {
        warnings.push(`"${id}" and "${otherId}" have overlapping names — check they are not the same solver`)
      }
    }
  }

  return problems
}

export function validateWarnings(tools: Tool[]): string[] {
  const warnings: string[] = []
  const byCategory = new Map<string, { id: string; keywords: Set<string> }[]>()
  for (const tool of tools) {
    const list = byCategory.get(tool.category) ?? []
    list.push({ id: tool.id, keywords: new Set((tool.keywords ?? []).map((k) => k.toLowerCase())) })
    byCategory.set(tool.category, list)
  }
  for (const [category, list] of byCategory) {
    for (let i = 0; i < list.length; i++) {
      for (let j = i + 1; j < list.length; j++) {
        const a = list[i]!
        const b = list[j]!
        let shared = 0
        for (const k of a.keywords) if (b.keywords.has(k)) shared++
        if (shared >= 4) {
          warnings.push(`${category}: "${a.id}" and "${b.id}" share ${shared} keywords — check for overlap`)
        }
      }
    }
  }
  return warnings
}
const DEV = (import.meta as unknown as { env?: { DEV?: boolean } }).env?.DEV ?? false

if (DEV) {
  const problems = validateTools(TOOLS)
  if (problems.length) {
    console.warn(`[registry] ${problems.length} problem(s):\n` + problems.map((p) => `  · ${p}`).join('\n'))
  }
  const warnings = validateWarnings(TOOLS)
  if (warnings.length) {
    console.warn(`[registry] ${warnings.length} warning(s):\n` + warnings.map((w) => `  · ${w}`).join('\n'))
  }
}

export interface ScoredTool {
  tool: Tool
  score: number
}

function escapeRe(s: string): string {
  return s.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')
}

/** Matches q only where it stands as a whole word, so "ph" does not match "graph". */
function wordMatch(text: string, q: string): boolean {
  try {
    return new RegExp(`(^|[^a-z0-9])${escapeRe(q)}([^a-z0-9]|$)`, 'i').test(text)
  } catch {
    return text.toLowerCase().includes(q.toLowerCase())
  }
}

export function searchTools(query: string, category?: string, limit = 50): ScoredTool[] {
  const pool = category ? TOOLS.filter((t) => t.category === category) : TOOLS
  const q = query.trim().toLowerCase()
  if (!q) return pool.map((tool) => ({ tool, score: 0 })).slice(0, limit)

  const words = q.split(/\s+/).filter(Boolean)
  const results: ScoredTool[] = []

  for (const tool of pool) {
    const name = tool.name.toLowerCase()
    const tagline = (tool.tagline ?? '').toLowerCase()
    const keywords = (tool.keywords ?? []).map((k) => k.toLowerCase())
    const haystacks = [name, tagline, ...keywords]

    let score = 0
    let matchedAll = true

    for (const w of words) {
      let wordScore = -1
      if (name === w) wordScore = 1000
      else if (name.startsWith(w)) wordScore = 800
      else if (name.includes(w)) wordScore = 600
      else if (keywords.some((k) => k === w)) wordScore = 500
      else if (keywords.some((k) => k.startsWith(w))) wordScore = 400
      else if (keywords.some((k) => k.includes(w))) wordScore = 250
      else if (tagline.includes(w)) wordScore = 180
      else if (name.split(/\s+/).some((t) => t.startsWith(w))) wordScore = 150

      if (wordScore > 0 && w.length <= 3) {
        if (!haystacks.some((h) => wordMatch(h, w))) wordScore = -1
      }
      if (wordScore < 0) {
        matchedAll = false
        break
      }
      score += wordScore
    }

    if (matchedAll && score > 0) results.push({ tool, score })
  }

  results.sort((a, b) => b.score - a.score || a.tool.name.localeCompare(b.tool.name))
  return results.slice(0, limit)
}

export function toolsInCategory(category: string): Tool[] {
  return TOOLS.filter((t) => t.category === category)
}

export function defaultValues(tool: SolverTool): Record<string, string> {
  const out: Record<string, string> = {}
  for (const input of tool.inputs) out[input.key] = input.default ?? ''
  return out
}