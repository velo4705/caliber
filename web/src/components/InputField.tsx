import type { ToolInput } from '../registry/types'

interface Props {
  input: ToolInput
  value: string
  onChange: (value: string) => void
}

export function InputField({ input, value, onChange }: Props) {
  const kind = input.kind ?? 'number'
  const label = input.unit ? `${input.label} (${input.unit})` : input.label

  return (
    <label className="field">
      <span className="field-label">{label}</span>
      {kind === 'select' ? (
        <select value={value} onChange={(e) => onChange(e.target.value)}>
          {(input.options ?? []).map((o) => (
            <option key={o.value} value={o.value}>
              {o.label}
            </option>
          ))}
        </select>
      ) : kind === 'number' ? (
        <input
          type="number"
          inputMode="decimal"
          step={input.step ?? 'any'}
          min={input.min}
          max={input.max}
          placeholder={input.placeholder}
          value={value}
          onChange={(e) => onChange(e.target.value)}
        />
      ) : (
        <input
          type="text"
          className={kind === 'expression' ? 'mono' : undefined}
          placeholder={input.placeholder}
          value={value}
          spellCheck={false}
          autoComplete="off"
          onChange={(e) => onChange(e.target.value)}
        />
      )}
      {input.help && <span className="field-help">{input.help}</span>}
    </label>
  )
}