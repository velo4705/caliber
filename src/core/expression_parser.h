#pragma once
#include <string>
#include <stdexcept>

enum class ParserAngleMode { Degrees, Radians, Gradians };

// Parses and evaluates mathematical expressions using recursive descent.
// Supports: +, -, *, /, %, ^, parentheses, unary minus,
//           scientific functions: sin, cos, tan, asin, acos, atan,
//           log, ln, log2, sqrt, abs, ceil, floor, factorial (n!)
//           constants: pi, e
class ExpressionParser {
public:
    ExpressionParser();

    // Evaluate an infix expression string, returns the result
    // Throws std::runtime_error on invalid input
    double evaluate(const std::string& expression);

    void setAngleMode(ParserAngleMode mode) { m_angleMode = mode; }

private:
    std::string     m_expr;
    size_t          m_pos;
    ParserAngleMode m_angleMode;

    double parseExpression();   // handles + and -
    double parseTerm();         // handles * and /
    double parsePower();        // handles ^
    double parseUnary();        // handles unary minus
    double parsePostfix(double val); // handles n! factorial
    double parsePrimary();      // handles numbers, functions, constants, parentheses

    double callFunction(const std::string& name, double arg);
    double toRadians(double val) const;
    double factorial(double n) const;

    void   skipWhitespace();
    char   current() const;
    char   consume();
    bool   isEnd() const;
    std::string parseIdentifier();
};
