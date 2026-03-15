#pragma once
#include "expression_parser.h"
#include <QString>

// Thin wrapper around ExpressionParser that works with Qt strings
// and formats results cleanly.
class MathEngine {
public:
    MathEngine();

    // Evaluate expression, returns formatted result string.
    // On error, returns an error message string.
    QString evaluate(const QString& expression);

    // Returns the last numeric result (valid after a successful evaluate)
    double lastResult() const { return m_lastResult; }
    bool   hasError()   const { return m_hasError; }

private:
    ExpressionParser m_parser;
    double           m_lastResult = 0.0;
    bool             m_hasError   = false;

    QString formatResult(double value) const;
};
