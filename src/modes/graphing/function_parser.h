#pragma once
#include <QString>
#include "core/expression_parser.h"

// Thin wrapper around ExpressionParser that substitutes 'x' with a value
// before evaluating, enabling f(x) style function plotting.
class FunctionParser {
public:
    FunctionParser();

    // Evaluate f(x) for a given x value
    // Throws std::runtime_error on parse error
    double evaluate(const QString& expression, double x);

    bool isValid(const QString& expression) const;

private:
    ExpressionParser m_parser;
};
