#pragma once
#include <QString>
#include "core/expression_parser.h"

class FunctionParser {
public:
    FunctionParser();

    // 2D: evaluate f(x)
    double evaluate(const QString& expression, double x);

    // Polar: evaluate f(theta) — substitutes theta and x
    double evaluatePolar(const QString& expression, double theta);

    // Parametric: evaluate g(t) — substitutes t
    double evaluateParametric(const QString& expression, double t);

    // 3D: evaluate f(x, y)
    double evaluate(const QString& expression, double x, double y);

    bool isValid(const QString& expression) const;

private:
    ExpressionParser m_parser;
};
