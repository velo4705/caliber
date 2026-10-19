import type { Tool } from '../registry/types'
import { num } from '../lib/values'
import { formatNumber as fmt } from '../lib/format'

const RAD = 180 / Math.PI

export const geometryTools: Tool[] = [
  {
    id: 'vector-magnitude',
    category: 'geometry',
    name: 'Vector Magnitude',
    tagline: 'Length and direction of a 3D vector',
    keywords: ['vector', 'magnitude', 'length', 'norm', 'direction', 'angles'],
    inputs: [
      { key: 'x', label: 'x', default: '3' },
      { key: 'y', label: 'y', default: '4' },
      { key: 'z', label: 'z', default: '12' },
    ],
    outputs: [
      { key: 'mag', label: 'Magnitude', formula: '√(x² + y² + z²)', emphasis: 'primary' },
      { key: 'thetaX', label: 'Angle from +x', formula: 'acos(x / |v|)', unit: '°' },
      { key: 'thetaY', label: 'Angle from +y', formula: 'acos(y / |v|)', unit: '°' },
      { key: 'thetaZ', label: 'Angle from +z', formula: 'acos(z / |v|)', unit: '°' },
      { key: 'unitX', label: 'Unit vector i', formula: 'x / |v|' },
      { key: 'unitY', label: 'Unit vector j', formula: 'y / |v|' },
      { key: 'unitZ', label: 'Unit vector k', formula: 'z / |v|' },
    ],
    compute: (v) => {
      const x = num(v, 'x')
      const y = num(v, 'y')
      const z = num(v, 'z')
      if (x === undefined || y === undefined || z === undefined) return {}
      const mag = Math.sqrt(x * x + y * y + z * z)
      if (mag === 0) return { mag: 0 }
      return {
        mag,
        thetaX: Math.acos(x / mag) * RAD,
        thetaY: Math.acos(y / mag) * RAD,
        thetaZ: Math.acos(z / mag) * RAD,
        unitX: x / mag,
        unitY: y / mag,
        unitZ: z / mag,
      }
    },
    reference: ['|v| = √(x² + y² + z²)', 'cos θₓ = x / |v|'],
    examples: [{ label: '3-4-12', inputs: { x: '3', y: '4', z: '12' } }],
  },
  {
    id: 'dot-product',
    category: 'geometry',
    name: 'Dot Product & Angle',
    tagline: 'a · b and the angle between vectors',
    keywords: ['dot product', 'scalar product', 'angle between', 'parallel', 'perpendicular', 'orthogonal'],
    inputs: [
      { key: 'ax', label: 'aₓ', default: '1' },
      { key: 'ay', label: 'aᵧ', default: '2' },
      { key: 'az', label: 'a_z', default: '3' },
      { key: 'bx', label: 'bₓ', default: '4' },
      { key: 'by', label: 'bᵧ', default: '5' },
      { key: 'bz', label: 'b_z', default: '6' },
    ],
    outputs: [
      { key: 'dot', label: 'a · b', formula: 'aₓbₓ + aᵧbᵧ + a_zb_z', emphasis: 'primary' },
      { key: 'angle', label: 'Angle between', formula: 'acos(a·b / |a||b|)', unit: '°', emphasis: 'primary' },
      { key: 'magA', label: '|a|', formula: '—' },
      { key: 'magB', label: '|b|', formula: '—' },
    ],
    compute: (v) => {
      const ax = num(v, 'ax')
      const ay = num(v, 'ay')
      const az = num(v, 'az')
      const bx = num(v, 'bx')
      const by = num(v, 'by')
      const bz = num(v, 'bz')
      if (ax === undefined || ay === undefined || az === undefined || bx === undefined || by === undefined || bz === undefined) return {}
      const dot = ax * bx + ay * by + az * bz
      const magA = Math.sqrt(ax * ax + ay * ay + az * az)
      const magB = Math.sqrt(bx * bx + by * by + bz * bz)
      const den = magA * magB
      return { dot, magA, magB, angle: den === 0 ? undefined : Math.acos(dot / den) * RAD }
    },
    reference: ['a · b = |a||b| cos θ'],
    examples: [{ label: 'Perpendicular', inputs: { ax: '1', ay: '0', az: '0', bx: '0', by: '1', bz: '0' } }],
  },
  {
    id: 'cross-product',
    category: 'geometry',
    name: 'Cross Product',
    tagline: 'a × b and the parallelogram it spans',
    keywords: ['cross product', 'vector product', 'perpendicular', 'area', 'parallelogram'],
    inputs: [
      { key: 'ax', label: 'aₓ', default: '1' },
      { key: 'ay', label: 'aᵧ', default: '0' },
      { key: 'az', label: 'a_z', default: '0' },
      { key: 'bx', label: 'bₓ', default: '0' },
      { key: 'by', label: 'bᵧ', default: '1' },
      { key: 'bz', label: 'b_z', default: '0' },
    ],
    outputs: [
      { key: 'cx', label: 'x component', formula: 'aᵧb_z − a_zbᵧ', emphasis: 'primary' },
      { key: 'cy', label: 'y component', formula: 'a_zbₓ − aₓb_z', emphasis: 'primary' },
      { key: 'cz', label: 'z component', formula: 'aₓbᵧ − aᵧbₓ', emphasis: 'primary' },
      { key: 'mag', label: '|a × b|', formula: 'area of parallelogram', emphasis: 'primary' },
    ],
    compute: (v) => {
      const ax = num(v, 'ax')
      const ay = num(v, 'ay')
      const az = num(v, 'az')
      const bx = num(v, 'bx')
      const by = num(v, 'by')
      const bz = num(v, 'bz')
      if (ax === undefined || ay === undefined || az === undefined || bx === undefined || by === undefined || bz === undefined) return {}
      const cx = ay * bz - az * by
      const cy = az * bx - ax * bz
      const cz = ax * by - ay * bx
      return { cx, cy, cz, mag: Math.sqrt(cx * cx + cy * cy + cz * cz) }
    },
    steps: (_v, r) => [
      `Resultant vector = (${fmt(r.cx)}, ${fmt(r.cy)}, ${fmt(r.cz)})`,
      `Magnitude = |a||b| sin θ = ${fmt(r.mag)}`,
    ],
    reference: ['a × b is perpendicular to both a and b'],
    examples: [{ label: 'ẑ direction', inputs: { ax: '1', ay: '0', az: '0', bx: '0', by: '1', bz: '0' } }],
  },
  {
    id: 'triangle',
    category: 'geometry',
    name: 'Triangle Solver',
    tagline: 'From three sides, find every angle and length',
    keywords: ['triangle', 'heron', 'area', 'perimeter', 'angles', 'hypotenuse', 'pythagoras'],
    inputs: [
      { key: 'a', label: 'Side a', default: '3' },
      { key: 'b', label: 'Side b', default: '4' },
      { key: 'c', label: 'Side c', default: '5' },
    ],
    outputs: [
      { key: 'A', label: 'Angle A', formula: 'acos((b²+c²−a²)/2bc)', unit: '°', emphasis: 'primary' },
      { key: 'B', label: 'Angle B', formula: 'acos((a²+c²−b²)/2ac)', unit: '°', emphasis: 'primary' },
      { key: 'C', label: 'Angle C', formula: 'acos((a²+b²−c²)/2ab)', unit: '°', emphasis: 'primary' },
      { key: 'area', label: 'Area', formula: "Heron's", emphasis: 'primary' },
      { key: 'perimeter', label: 'Perimeter', formula: 'a + b + c' },
      { key: 'inradius', label: 'Inradius', formula: 'A / s' },
      { key: 'circumradius', label: 'Circumradius', formula: 'abc / 4A' },
      { key: 'kind', label: 'Type', formula: '—' },
    ],
    compute: (v) => {
      const a = num(v, 'a')
      const b = num(v, 'b')
      const c = num(v, 'c')
      if (a === undefined || b === undefined || c === undefined) return {}
      if (a <= 0 || b <= 0 || c <= 0) return {}
      if (a + b <= c || a + c <= b || b + c <= a) return {}
      const A = (Math.acos((b * b + c * c - a * a) / (2 * b * c)) * RAD)
      const B = (Math.acos((a * a + c * c - b * b) / (2 * a * c)) * RAD)
      const C = (Math.acos((a * a + b * b - c * c) / (2 * a * b)) * RAD)
      const s = (a + b + c) / 2
      const area = Math.sqrt(s * (s - a) * (s - b) * (s - c))
      const longest = Math.max(a, b, c)
      const others = [a, b, c].filter((x) => x !== longest)
      const kind =
        a === b && b === c ? 2 : a === b || b === c || a === c ? 1 : longest * longest === (others[0] as number) ** 2 + (others[1] as number) ** 2 ? 3 : 0
      return {
        A,
        B,
        C,
        area,
        perimeter: a + b + c,
        inradius: area / s,
        circumradius: (a * b * c) / (4 * area),
        kind: [0, 'Scalene', 'Isosceles', 'Equilateral', 'Right-angled'][kind],
      }
    },
    steps: (v, r) => {
      const s: string[] = []
      s.push(`Sides: a = ${fmt(num(v, 'a'))}, b = ${fmt(num(v, 'b'))}, c = ${fmt(num(v, 'c'))}`)
      if (r.area !== undefined) s.push(`s = (a+b+c)/2, then area = √[s(s−a)(s−b)(s−c)] = ${fmt(r.area)}`)
      if (r.A !== undefined) s.push(`Angles sum to ${fmt(Number(r.A) + Number(r.B ?? 0) + Number(r.C ?? 0), 6)}° (should be 180°)`)
      return s
    },
    reference: ["Heron: A = √[s(s−a)(s−b)(s−c)]", 'Law of cosines: cos C = (a²+b²−c²) / 2ab'],
    examples: [
      { label: '3-4-5 right triangle', inputs: { a: '3', b: '4', c: '5' } },
      { label: 'Equilateral', inputs: { a: '5', b: '5', c: '5' } },
    ],
  },
]