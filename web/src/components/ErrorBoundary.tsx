import { Component, type ErrorInfo, type ReactNode } from 'react'

interface Props {
  children: ReactNode
  label?: string
}

interface State {
  error: Error | null
  info: string
}

export class ErrorBoundary extends Component<Props, State> {
  state: State = { error: null, info: '' }

  static getDerivedStateFromError(error: Error): Partial<State> {
    return { error }
  }

  componentDidCatch(error: Error, info: ErrorInfo) {
    this.setState({ info: info.componentStack ?? '' })
    console.error('[caliber] render failed', error, info.componentStack)
  }

  render() {
    const { error, info } = this.state
    if (!error) return this.props.children

    const details = `${error.name}: ${error.message}\n\n${this.props.label ?? ''}\n${info}`.trim()

    return (
      <div className="crash">
        <div className="crash-card">
          <h1>Something went wrong</h1>
          <p className="crash-lead">
            {this.props.label ? `${this.props.label} could not be displayed.` : 'This view could not be displayed.'}{' '}
            Your inputs are still in the address bar — copy it before reloading if you want to keep them.
          </p>
          <pre className="crash-detail">{details}</pre>
          <div className="crash-actions">
            <button
              className="btn-ghost"
              onClick={() => navigator.clipboard?.writeText(details)}
            >
              Copy details
            </button>
            <button className="btn-ghost" onClick={() => window.location.reload()}>
              Reload
            </button>
            <button className="btn-ghost" onClick={() => (window.location.hash = '#/')}>
              Go to home
            </button>
          </div>
        </div>
      </div>
    )
  }
}
