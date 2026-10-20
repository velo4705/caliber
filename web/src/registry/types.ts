export type InputKind = 'number' | 'number-list' | 'expression' | 'select' | 'text'

export interface ToolInput {
  key: string
  label: string
  unit?: string
  kind?: InputKind
  default?: string
  options?: { value: string; label: string }[]
  min?: number
  max?: number
  step?: number
  help?: string
  placeholder?: string
}

export interface ToolOutput {
  key: string
  label: string
  unit?: string
  formula?: string
  precision?: number
  needs?: string
  emphasis?: 'primary' | 'secondary'
  /** Hide the row entirely when the value is undefined. */
  optional?: boolean
}

export type Values = Record<string, number | string | number[] | undefined>

export type Computed = Record<string, number | string | undefined>

export interface ToolExample {
  label: string
  inputs: Record<string, string>
}

export interface GraphFunction {
  key: string
  label: string
  default?: string
  defaultY?: string
  color?: string
}

export type GraphMode = 'cartesian' | 'polar' | 'parametric' | 'implicit' | 'surface'

export interface GraphSpec {
  mode: GraphMode
  functions: GraphFunction[]
  viewport?: { x0: number; x1: number; y0: number; y1: number }
  tRange?: [number, number]
  reference?: string[]
}

interface ToolBase {
  id: string
  category: string
  name: string
  tagline?: string
  keywords?: string[]
  examples?: ToolExample[]
  reference?: string[]
}

export interface SolverTool extends ToolBase {
  kind?: 'solver'
  inputs: ToolInput[]
  outputs: ToolOutput[]
  compute: (v: Values) => Computed
  steps?: (v: Values, r: Computed) => string[]
}

export interface ChartTool extends ToolBase {
  kind: 'chart'
  graph: GraphSpec
}

export type Tool = SolverTool | ChartTool

export interface Category {
  id: string
  name: string
  icon: string
  blurb: string
}