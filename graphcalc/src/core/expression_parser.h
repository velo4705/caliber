#pragma once
#include <string>
#include <stdexcept>

// Parses and evaluates mathematical expressions using recursive descent.
// Supports: +, -, *, /, %, ^, parentheses, unary minus
class ExpressionParser {
public:
    // Evaluate an infix expression string, returns the result
    // Throws std::runtime_error on invalid input
    double evaluate(const std::string& expression);

private:
    std::string m_expr;
    size_t      m_pos;

    double parseExpression();   // handles + and -
    double parseTerm();         // handles * and /
    double parsePower();        // handles ^
    double parseUnary();        // handles unary minus
    double parsePrimary();      // handles numbers and parentheses

    void   skipWhitespace();
    char   current() const;
    char   consume();
    bool   isEnd() const;
};
