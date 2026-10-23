import type { ToolInput } from '../registry/types'

interface Props {
  input: ToolInput
  value: string
  onChange: (value: string) => void
}

export function InputField({ input, value, onChange }: Props) {
  const kind = input.kind ?? 'number'
  const label = input.unit ? `${input.label} (${input.unit})` : input.label
  const matrix = (() => {
    try {
      return value ? JSON.parse(value) : undefined
    } catch {
      return undefined
    }
  })()

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
      ) : kind === 'matrix' ? (
        <div className="matrix-input">
          {matrix.map((row: number[], rIdx: number) => (
            <div key={rIdx} className="matrix-row">
              {row.map((cell: number, cIdx: number) => (
                <input
                  key={`${rIdx}-${cIdx}`}
                  type="number"
                  step="any"
                  value={cell}
                  onChange={(e) => {
                    const newMatrix = matrix.map((row2: number[], rIdx2: number) =>
                      rIdx === rIdx2
                        ? row2.map((cell2: number, cIdx2: number) =>
                            cIdx === cIdx2 ? Number(e.target.value) : cell2
                        )
                        : row2
                    )
                    onChange(JSON.stringify(newMatrix))
                  }}
                />
              ))}
            </div>
          ))}
        </div>
      ) : (
        <>
          {input.help && <span className="field-help">{input.help}</span>}
        </>
      )}
    </label>
  )
}