import type { Tool } from '../registry/types'
import { num, str } from '../lib/values'

interface UnitDef {
  label: string
  factor: number
  toBase?: (v: number) => number
  fromBase?: (v: number) => number
}

function converter(opts: {
  id: string
  name: string
  tagline: string
  keywords: string[]
  units: UnitDef[]
  from: string
  to: string
  defaultValue?: string
}): Tool {
  return {
    id: opts.id,
    category: 'units',
    name: opts.name,
    tagline: opts.tagline,
    keywords: opts.keywords,
    inputs: [
      { key: 'value', label: 'Value', default: opts.defaultValue ?? '1', step: 0.01 },
      {
        key: 'from',
        label: 'From',
        kind: 'select',
        default: opts.from,
        options: opts.units.map((u) => ({ value: u.label, label: u.label })),
      },
      {
        key: 'to',
        label: 'To',
        kind: 'select',
        default: opts.to,
        options: opts.units.map((u) => ({ value: u.label, label: u.label })),
      },
    ],
    outputs: [
      { key: 'result', label: 'Result', emphasis: 'primary' },
      { key: 'inverse', label: 'Inverse', emphasis: 'secondary' },
    ],
    compute: (v) => {
      const value = num(v, 'value')
      const fromLabel = str(v, 'from')
      const toLabel = str(v, 'to')
      if (value === undefined) return {}
      const from = opts.units.find((u) => u.label === fromLabel)
      const to = opts.units.find((u) => u.label === toLabel)
      if (!from || !to) return {}
      const base = from.toBase ? from.toBase(value) : value * from.factor
      const result = to.fromBase ? to.fromBase(base) : base / to.factor
      const invBase = to.toBase ? to.toBase(value) : value * to.factor
      const inverse = from.fromBase ? from.fromBase(invBase) : invBase / from.factor
      return { result, inverse }
    },
  }
}

const T = (degF: (v: number) => number, degC: (v: number) => number): UnitDef[] => [
  { label: 'Celsius (°C)', factor: 1, toBase: (v) => v + 273.15, fromBase: (v) => v - 273.15 },
  { label: 'Fahrenheit (°F)', factor: 1, toBase: (v) => degC(v) + 273.15, fromBase: (v) => degF(v - 273.15) },
  { label: 'Kelvin (K)', factor: 1, toBase: (v) => v, fromBase: (v) => v },
  { label: 'Rankine (°R)', factor: 1, toBase: (v) => v * (5 / 9), fromBase: (v) => v * (9 / 5) },
]

export const unitTools: Tool[] = [
  converter({
    id: 'length',
    name: 'Length',
    tagline: 'Metric and imperial length',
    keywords: ['length', 'metre', 'meter', 'feet', 'inch', 'mile', 'distance', 'metres'],
    from: 'Metre (m)',
    to: 'Foot (ft)',
    units: [
      { label: 'Metre (m)', factor: 1 },
      { label: 'Kilometre (km)', factor: 1000 },
      { label: 'Centimetre (cm)', factor: 0.01 },
      { label: 'Millimetre (mm)', factor: 0.001 },
      { label: 'Foot (ft)', factor: 0.3048 },
      { label: 'Inch (in)', factor: 0.0254 },
      { label: 'Yard (yd)', factor: 0.9144 },
      { label: 'Mile (mi)', factor: 1609.344 },
      { label: 'Nautical mile (nmi)', factor: 1852 },
    ],
  }),
  converter({
    id: 'mass',
    name: 'Mass',
    tagline: 'Metric and imperial mass',
    keywords: ['mass', 'weight', 'kilogram', 'pound', 'ounce', 'stone', 'gram'],
    from: 'Kilogram (kg)',
    to: 'Pound (lb)',
    units: [
      { label: 'Kilogram (kg)', factor: 1 },
      { label: 'Gram (g)', factor: 0.001 },
      { label: 'Milligram (mg)', factor: 1e-6 },
      { label: 'Tonne (t)', factor: 1000 },
      { label: 'Pound (lb)', factor: 0.45359237 },
      { label: 'Ounce (oz)', factor: 0.028349523125 },
      { label: 'Stone (st)', factor: 6.35029318 },
    ],
  }),
  converter({
    id: 'volume',
    name: 'Volume',
    tagline: 'Litres, gallons and cubic units',
    keywords: ['volume', 'litre', 'liter', 'gallon', 'cubic', 'capacity', 'fluid'],
    from: 'Litre (L)',
    to: 'US gallon (gal)',
    units: [
      { label: 'Litre (L)', factor: 1 },
      { label: 'Millilitre (mL)', factor: 0.001 },
      { label: 'Cubic metre (m³)', factor: 1000 },
      { label: 'US gallon (gal)', factor: 3.785411784 },
      { label: 'Imperial gallon (imp gal)', factor: 4.54609 },
      { label: 'US quart (qt)', factor: 0.946352946 },
      { label: 'US pint (pt)', factor: 0.473176473 },
      { label: 'Fluid ounce (fl oz)', factor: 0.0295735295625 },
      { label: 'Cup', factor: 0.2365882365 },
    ],
  }),
  converter({
    id: 'time',
    name: 'Time',
    tagline: 'Seconds through to years',
    keywords: ['time', 'seconds', 'minutes', 'hours', 'days', 'weeks', 'years'],
    from: 'Second (s)',
    to: 'Hour (h)',
    units: [
      { label: 'Second (s)', factor: 1 },
      { label: 'Millisecond (ms)', factor: 0.001 },
      { label: 'Microsecond (µs)', factor: 1e-6 },
      { label: 'Minute (min)', factor: 60 },
      { label: 'Hour (h)', factor: 3600 },
      { label: 'Day (d)', factor: 86400 },
      { label: 'Week (wk)', factor: 604800 },
      { label: 'Julian year (yr)', factor: 31557600 },
    ],
  }),
  converter({
    id: 'speed',
    name: 'Speed',
    tagline: 'm/s, km/h, mph and more',
    keywords: ['speed', 'velocity', 'kmh', 'mph', 'knot', 'mach', 'fast'],
    from: 'Metres per second (m/s)',
    to: 'Kilometres per hour (km/h)',
    units: [
      { label: 'Metres per second (m/s)', factor: 1 },
      { label: 'Kilometres per hour (km/h)', factor: 1 / 3.6 },
      { label: 'Miles per hour (mph)', factor: 0.44704 },
      { label: 'Feet per second (ft/s)', factor: 0.3048 },
      { label: 'Knot (kn)', factor: 0.514444444 },
      { label: 'Mach (at sea level)', factor: 340.29 },
    ],
  }),
  converter({
    id: 'energy',
    name: 'Energy',
    tagline: 'Joules, calories, kWh and eV',
    keywords: ['energy', 'joule', 'calorie', 'watt hour', 'kwh', 'electronvolt', 'btu'],
    from: 'Joule (J)',
    to: 'Kilowatt-hour (kWh)',
    units: [
      { label: 'Joule (J)', factor: 1 },
      { label: 'Kilojoule (kJ)', factor: 1000 },
      { label: 'Megajoule (MJ)', factor: 1e6 },
      { label: 'Calorie (cal)', factor: 4.184 },
      { label: 'Kilocalorie (kcal)', factor: 4184 },
      { label: 'Watt-hour (Wh)', factor: 3600 },
      { label: 'Kilowatt-hour (kWh)', factor: 3.6e6 },
      { label: 'Electronvolt (eV)', factor: 1.602176634e-19 },
      { label: 'BTU', factor: 1055.05585262 },
      { label: 'Foot-pound (ft·lb)', factor: 1.3558179483 },
    ],
  }),
  converter({
    id: 'pressure',
    name: 'Pressure',
    tagline: 'Pa, bar, atm, psi and mmHg',
    keywords: ['pressure', 'pascal', 'bar', 'atmosphere', 'psi', 'mmhg', 'torr'],
    from: 'Pascal (Pa)',
    to: 'Bar',
    units: [
      { label: 'Pascal (Pa)', factor: 1 },
      { label: 'Kilopascal (kPa)', factor: 1000 },
      { label: 'Bar', factor: 1e5 },
      { label: 'Standard atmosphere (atm)', factor: 101325 },
      { label: 'PSI', factor: 6894.757293168 },
      { label: 'Millimetre of mercury (mmHg)', factor: 133.322387415 },
      { label: 'Torr', factor: 133.322368421 },
    ],
  }),
  converter({
    id: 'area',
    name: 'Area',
    tagline: 'Square metres, acres and hectares',
    keywords: ['area', 'square', 'acre', 'hectare', 'acreage', 'floor'],
    from: 'Square metre (m²)',
    to: 'Square foot (ft²)',
    units: [
      { label: 'Square metre (m²)', factor: 1 },
      { label: 'Square kilometre (km²)', factor: 1e6 },
      { label: 'Square centimetre (cm²)', factor: 1e-4 },
      { label: 'Square foot (ft²)', factor: 0.09290304 },
      { label: 'Square inch (in²)', factor: 0.00064516 },
      { label: 'Square yard (yd²)', factor: 0.83612736 },
      { label: 'Acre', factor: 4046.8564224 },
      { label: 'Hectare (ha)', factor: 10000 },
      { label: 'Square mile (mi²)', factor: 2589988.110336 },
    ],
  }),
  converter({
    id: 'force',
    name: 'Force',
    tagline: 'Newton, pound-force and dyne',
    keywords: ['force', 'newton', 'pound force', 'dyne', 'kilonewton', 'push'],
    from: 'Newton (N)',
    to: 'Pound-force (lbf)',
    units: [
      { label: 'Newton (N)', factor: 1 },
      { label: 'Kilonewton (kN)', factor: 1000 },
      { label: 'Millinewton (mN)', factor: 0.001 },
      { label: 'Pound-force (lbf)', factor: 4.4482216152605 },
      { label: 'Kilogram-force (kgf)', factor: 9.80665 },
      { label: 'Dyne', factor: 1e-5 },
    ],
  }),
  converter({
    id: 'angle',
    name: 'Angle',
    tagline: 'Degrees, radians, gradians and turns',
    keywords: ['angle', 'degree', 'radian', 'gradian', 'turn', 'revolution', 'bearing'],
    from: 'Degree (°)',
    to: 'Radian (rad)',
    units: [
      { label: 'Degree (°)', factor: 1 },
      { label: 'Radian (rad)', factor: 180 / Math.PI },
      { label: 'Gradian (gon)', factor: 0.9 },
      { label: 'Turn (rev)', factor: 360 },
      { label: 'Arcminute (′)', factor: 1 / 60 },
      { label: 'Arcsecond (″)', factor: 1 / 3600 },
    ],
  }),
  converter({
    id: 'data',
    name: 'Data Size',
    tagline: 'Decimal (KB) and binary (KiB) sizes',
    keywords: ['data', 'storage', 'byte', 'kilobyte', 'megabyte', 'gigabyte', 'binary', 'kb', 'mb'],
    from: 'Megabyte (MB)',
    to: 'Mebibyte (MiB)',
    units: [
      { label: 'Bit (b)', factor: 0.125 },
      { label: 'Byte (B)', factor: 1 },
      { label: 'Kilobyte (KB)', factor: 1e3 },
      { label: 'Megabyte (MB)', factor: 1e6 },
      { label: 'Gigabyte (GB)', factor: 1e9 },
      { label: 'Terabyte (TB)', factor: 1e12 },
      { label: 'Kibibyte (KiB)', factor: 1024 },
      { label: 'Mebibyte (MiB)', factor: 1024 ** 2 },
      { label: 'Gibibyte (GiB)', factor: 1024 ** 3 },
      { label: 'Tebibyte (TiB)', factor: 1024 ** 4 },
    ],
  }),
  converter({
    id: 'temperature',
    name: 'Temperature',
    tagline: 'Celsius, Fahrenheit, Kelvin and Rankine',
    keywords: ['temperature', 'celsius', 'fahrenheit', 'kelvin', 'rankine', 'degrees'],
    from: 'Celsius (°C)',
    to: 'Fahrenheit (°F)',
    units: T((v) => v * (9 / 5) + 32, (v) => (v - 32) * (5 / 9)),
    defaultValue: '20',
  }),
]