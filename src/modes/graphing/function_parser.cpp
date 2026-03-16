#include "function_parser.h"
#include <QRegularExpression>
#include <cmath>

FunctionParser::FunctionParser() {
    // Graphing always uses radians — mathematical convention
    m_parser.setAngleMode(ParserAngleMode::Radians);
}

static QString substituteVar(const QString& expr, const QString& var, double val) {
    QString result = expr;
    QString numStr = QString::number(val, 'g', 17);
    if (val < 0) numStr = "(" + numStr + ")";
    QRegularExpression re(QString(R"((?<![a-zA-Z0-9_])%1(?![a-zA-Z0-9_]))").arg(var));
    result.replace(re, numStr);
    return result;
}

double FunctionParser::evaluate(const QString& expression, double x) {
    QString expr = substituteVar(expression, "x", x);
    return m_parser.evaluate(expr.toStdString());
}

double FunctionParser::evaluate(const QString& expression, double x, double y) {
    QString expr = substituteVar(expression, "x", x);
    expr = substituteVar(expr, "y", y);
    return m_parser.evaluate(expr.toStdString());
}

bool FunctionParser::isValid(const QString& expression) const {
    if (expression.trimmed().isEmpty()) return false;
    FunctionParser tmp;
    try { tmp.evaluate(expression, 1.0); return true; }
    catch (...) { return false; }
}
