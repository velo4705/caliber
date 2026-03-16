#pragma once
#include "expression_parser.h"
#include <QString>

class MathEngine {
public:
    MathEngine();

    QString evaluate(const QString& expression);

    void setAngleMode(ParserAngleMode mode);
    ParserAngleMode angleMode() const { return m_angleMode; }

    double lastResult() const { return m_lastResult; }
    bool   hasError()   const { return m_hasError; }

private:
    ExpressionParser m_parser;
    ParserAngleMode  m_angleMode  = ParserAngleMode::Degrees;
    double           m_lastResult = 0.0;
    bool             m_hasError   = false;

    QString formatResult(double value) const;
};
