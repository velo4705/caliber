#include "math_engine.h"
#include <cmath>
#include <QString>

MathEngine::MathEngine() = default;

QString MathEngine::evaluate(const QString& expression) {
    m_hasError = false;
    if (expression.trimmed().isEmpty()) {
        m_lastResult = 0.0;
        return "0";
    }
    try {
        m_lastResult = m_parser.evaluate(expression.toStdString());
        return formatResult(m_lastResult);
    } catch (const std::exception& e) {
        m_hasError = true;
        return QString("Error: %1").arg(e.what());
    }
}

void MathEngine::setAngleMode(ParserAngleMode mode) {
    m_angleMode = mode;
    m_parser.setAngleMode(mode);
}
QString MathEngine::formatResult(double value) const {
    // Show integer values without decimal point
    if (std::isnan(value))  return "Not a number";
    if (std::isinf(value))  return value > 0 ? "Infinity" : "-Infinity";

    // If the value is a whole number and fits in a long long, show as integer
    if (value == std::floor(value) && std::abs(value) < 1e15) {
        return QString::number(static_cast<long long>(value));
    }

    // Otherwise show up to 10 significant digits, trimming trailing zeros
    QString s = QString::number(value, 'g', 10);
    return s;
}
