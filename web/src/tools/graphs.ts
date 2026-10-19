import type { ChartTool } from '../registry/types'

export const graphTools: ChartTool[] = [
  {
    id: 'graph-2d',
    kind: 'chart',
    category: 'graphing',
    name: 'Function Grapher',
    tagline: 'Plot y = f(x) and trace points on the curve',
    keywords: ['graph', 'plot', 'curve', 'function', 'graphing', 'plotter', 'cartesian', 'coordinate', 'trace', 'desmos'],
    graph: {
      mode: 'cartesian',
      viewport: { x0: -10, x1: 10, y0: -6, y1: 6 },
      functions: [
        { key: 'f1', label: 'f₁(x)', default: 'sin(x)' },
        { key: 'f2', label: 'f₂(x)', default: 'x^2/4 - 3' },
        { key: 'f3', label: 'f₃(x)', default: '' },
      ],
      reference: [
        'y = f(x)',
        'Implicit: enter LHS − RHS = 0 (e.g. x² + y² = 4)',
        'Sweep: put a range in a slot — a = [1...5], a = [1, 2, 4] or a = [0...1...0.1] — then reference it',
      ],
    },
    examples: [
      { label: 'sin(x)', inputs: { f1: 'sin(x)', f2: '', f3: '' } },
      { label: 'Cubic + line', inputs: { f1: 'x^3 - 3x', f2: 'x', f3: '' } },
      { label: 'Circle', inputs: { f1: 'sqrt(4 - x^2)', f2: '-sqrt(4 - x^2)', f3: '' } },
      { label: 'Sweep a = 1…5', inputs: { f1: 'a = [1...5]', f2: 'a*x^2 - 2*a', f3: '' } },
      { label: 'Two parameters', inputs: { f1: 'a = [1, 2, 3]', f2: 'b = [1, 4]', f3: 'a*x^3 + b*x' } },
    ],
  },
  {
    id: 'graph-3d-projection',
    kind: 'chart',
    category: 'graphing',
    name: 'Parametric Curve',
    tagline: 'Plot (x(t), y(t)) — spirals, cycloids, Lissajous',
    keywords: ['parametric', 'curve', 'lissajous', 'cycloid', 'spiral', 'graph', 'plot'],
    graph: {
      mode: 'parametric',
      tRange: [0, Math.PI * 2],
      functions: [
        { key: 'x1', label: 'x(t)', default: 'cos(t)' },
        { key: 'y1', label: 'y(t)', default: 'sin(t)' },
        { key: 'x2', label: 'x₂(t)', default: '' },
        { key: 'y2', label: 'y₂(t)', default: '' },
      ],
      reference: ['x = x(t),  y = y(t)'],
    },
    examples: [
      { label: 'Circle', inputs: { x1: 'cos(t)', y1: 'sin(t)', x2: '', y2: '' } },
      { label: 'Lissajous', inputs: { x1: 'sin(3t)', y1: 'sin(2t)', x2: '', y2: '' } },
      { label: 'Cycloid', inputs: { x1: 't - sin(t)', y1: '1 - cos(t)', x2: '', y2: '' } },
    ],
  },
  {
    id: 'graph-polar',
    kind: 'chart',
    category: 'graphing',
    name: 'Polar Plot',
    tagline: 'Plot r = f(θ) — rose curves, limacons, conics',
    keywords: ['polar', 'rose', 'limacon', 'conic', 'spiral', 'graph', 'plot', 'theta'],
    graph: {
      mode: 'polar',
      functions: [
        { key: 'r1', label: 'r₁(θ)', default: '2*cos(2*t)' },
        { key: 'r2', label: 'r₂(θ)', default: '' },
      ],
      reference: ['x = r cos θ', 'y = r sin θ'],
    },
    examples: [
      { label: 'Rose (4 petals)', inputs: { r1: '2*cos(2*t)', r2: '' } },
      { label: 'Cardioid', inputs: { r1: '1 + cos(t)', r2: '' } },
      { label: 'Spiral', inputs: { r1: 't/4', r2: '' } },
    ],
  },
  {
    id: 'graph-3d-surface',
    kind: 'chart',
    category: 'graphing',
    name: '3D Surface',
    tagline: 'Plot z = f(x, y) and rotate it',
    keywords: ['3d', 'surface', 'plot', 'z=f(x,y)', 'rotat', 'mesh', 'saddle', 'volcano', 'graph'],
    graph: {
      mode: 'surface',
      viewport: { x0: -5, x1: 5, y0: -5, y1: 5 },
      functions: [{ key: 'z', label: 'z(x, y)', default: 'sin(x)*cos(y) + 0.25*(x^2 - y^2)' }],
      reference: ['z = f(x, y)', 'Height is scaled to fit the box; colour follows the viridis ramp'],
    },
    examples: [
      { label: 'Saddle', inputs: { z: 'sin(x)*cos(y) + 0.25*(x^2 - y^2)' } },
      { label: 'Volcano', inputs: { z: 'exp(-0.1*(x^2 + y^2))' } },
      { label: 'Cone', inputs: { z: 'sqrt(x^2 + y^2)' } },
      { label: 'Ripples', inputs: { z: 'sin(sqrt(x^2 + y^2)) * 2' } },
    ],
  },
  {
    id: 'graph-implicit',
    kind: 'chart',
    category: 'graphing',
    name: 'Implicit Plot',
    tagline: 'Plot a level set — circles, ellipses, Cassini ovals',
    keywords: ['implicit', 'level set', 'contour', 'circle', 'ellipse', 'curve', 'graph', 'plot', 'equation'],
    graph: {
      mode: 'implicit',
      viewport: { x0: -6, x1: 6, y0: -4, y1: 4 },
      functions: [
        { key: 'g1', label: 'g₁(x, y)', default: 'x^2 + y^2 - 4' },
        { key: 'g2', label: 'g₂(x, y)', default: '' },
      ],
      reference: ['g(x, y) = 0', 'Marching squares extracts the zero crossings per cell'],
    },
    examples: [
      { label: 'Circle', inputs: { g1: 'x^2 + y^2 - 4', g2: '' } },
      { label: 'Ellipse', inputs: { g1: 'x^2/4 + y^2 - 1', g2: '' } },
      { label: 'Cassini oval', inputs: { g1: '(x^2 + y^2)^2 - 4*(x^2 - y^2)', g2: '' } },
    ],
  },
]