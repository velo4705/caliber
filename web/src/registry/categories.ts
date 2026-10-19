import type { Category } from './types'

export const CATEGORIES: Category[] = [
  { id: 'math', name: 'Algebra', icon: '∑', blurb: 'Polynomials, ratios, combinatorics' },
  { id: 'statistics', name: 'Statistics', icon: 'σ', blurb: 'Descriptive stats, regression, probability' },
  { id: 'geometry', name: 'Geometry & Vectors', icon: '△', blurb: 'Triangles, vectors, cross and dot products' },
  { id: 'physics', name: 'Physics', icon: '⚛', blurb: 'Mechanics, waves, optics, energy' },
  { id: 'chemistry', name: 'Chemistry', icon: '⚗', blurb: 'Moles, pH, gases, solutions' },
  { id: 'electrical', name: 'Electrical', icon: '⚡', blurb: 'Ohm’s law, power, AC circuits' },
  { id: 'engineering', name: 'Engineering', icon: '⚙', blurb: 'Stress, strain, beams, tolerances' },
  { id: 'computing', name: 'Computing', icon: '⌨', blurb: 'Number bases, bitwise, encoding' },
  { id: 'finance', name: 'Finance', icon: '$', blurb: 'Interest, loans, growth' },
  { id: 'units', name: 'Units', icon: '⇄', blurb: 'Length, mass, energy, pressure and more' },
]

export const CATEGORY_MAP = new Map(CATEGORIES.map((c) => [c.id, c]))