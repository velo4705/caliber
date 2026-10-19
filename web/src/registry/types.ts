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
}

export type Values = Record<string, number | string | number[] | undefined>

export type Computed = Record<string, number | string | undefined>

export interface ToolExample {
  label: string
  inputs: Record<string, string>
}

export interface Tool {
  id: string
  category: string
  name: string
  tagline?: string
  keywords?: string[]
  inputs: ToolInput[]
  outputs: ToolOutput[]
  compute: (v: Values) => Computed
  reference?: string[]
  steps?: (v: Values, r: Computed) => string[]
  examples?: ToolExample[]
}

export interface Category {
  id: string
  name: string
  icon: string
  blurb: string
}