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
]

export const TOOL_MAP = new Map(TOOLS.map((t) => [t.id, t]))

export function validateTools(tools: Tool[]): string[] {
  const problems: string[] = []
  const seen = new Set<string>()
  for (const tool of tools) {
    if (seen.has(tool.id)) problems.push(`Duplicate tool id: ${tool.id}`)
    seen.add(tool.id)
    if (!CATEGORY_MAP.has(tool.category)) {
      problems.push(`${tool.id}: unknown category "${tool.category}"`)
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
  }
  return problems
}

if (import.meta.env.DEV) {
  const problems = validateTools(TOOLS)
  if (problems.length) {
    console.warn(`[registry] ${problems.length} problem(s):\n` + problems.map((p) => `  · ${p}`).join('\n'))
  }
}

export interface ScoredTool {
  tool: Tool
  score: number
}

export function searchTools(query: string, category?: string, limit = 50): ScoredTool[] {
  const pool = category ? TOOLS.filter((t) => t.category === category) : TOOLS
  const q = query.trim().toLowerCase()
  if (!q) return pool.map((tool) => ({ tool, score: 0 })).slice(0, limit)

  const results: ScoredTool[] = []
  for (const tool of pool) {
    const name = tool.name.toLowerCase()
    const tagline = (tool.tagline ?? '').toLowerCase()
    const keywords = (tool.keywords ?? []).map((k) => k.toLowerCase())
    let score = 0

    if (name === q) score = 1000
    else if (name.startsWith(q)) score = 800
    else if (name.includes(q)) score = 600
    else if (keywords.some((k) => k === q)) score = 500
    else if (keywords.some((k) => k.startsWith(q))) score = 400
    else if (keywords.some((k) => k.includes(q))) score = 250
    else if (tagline.includes(q)) score = 180
    else if (name.split(/\s+/).some((w) => w.startsWith(q))) score = 150

    if (score > 0) results.push({ tool, score })
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