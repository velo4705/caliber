#include "function_parser.h"
#include <QRegularExpression>
#include <cmath>

FunctionParser::FunctionParser() = default;

double FunctionParser::evaluate(const QString& expression, double x) {
    // Replace 'x' tokens with the numeric value
    // Use word-boundary-aware replacement to avoid hitting 'exp', 'max', etc.
    QString expr = expression;

    // Replace standalone 'x' (not preceded/followed by alphanumeric or underscore)
    QString xVal = QString::number(x, 'g', 17);

    // Wrap negative values in parens to avoid sign issues: x -> (-3.14)
    if (x < 0) xVal = "(" + xVal + ")";

    QRegularExpression re(R"((?<![a-zA-Z0-9_])x(?![a-zA-Z0-9_]))");
    expr.replace(re, xVal);

    return m_parser.evaluate(expr.toStdString());
}

bool FunctionParser::isValid(const QString& expression) const {
    if (expression.trimmed().isEmpty()) return false;
    // Quick check: try evaluating at x=1
    FunctionParser tmp;
    try {
        tmp.evaluate(expression, 1.0);
        return true;
    } catch (...) {
        return false;
    }
}
