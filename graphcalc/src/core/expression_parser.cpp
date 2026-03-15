#include "expression_parser.h"
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <string>

double ExpressionParser::evaluate(const std::string& expression) {
    m_expr = expression;
    m_pos  = 0;
    double result = parseExpression();
    skipWhitespace();
    if (!isEnd())
        throw std::runtime_error("Unexpected character: " + std::string(1, current()));
    return result;
}

// expression := term (('+' | '-') term)*
double ExpressionParser::parseExpression() {
    double left = parseTerm();
    skipWhitespace();
    while (!isEnd() && (current() == '+' || current() == '-')) {
        char op = consume();
        double right = parseTerm();
        left = (op == '+') ? left + right : left - right;
        skipWhitespace();
    }
    return left;
}

// term := power (('*' | '/' | '%') power)*
double ExpressionParser::parseTerm() {
    double left = parsePower();
    skipWhitespace();
    while (!isEnd() && (current() == '*' || current() == '/' || current() == '%')) {
        char op = consume();
        double right = parsePower();
        if ((op == '/' || op == '%') && right == 0.0)
            throw std::runtime_error("Division by zero");
        if (op == '*') left = left * right;
        else if (op == '/') left = left / right;
        else left = std::fmod(left, right);
        skipWhitespace();
    }
    return left;
}

// power := unary ('^' unary)*  (right-associative)
double ExpressionParser::parsePower() {
    double base = parseUnary();
    skipWhitespace();
    if (!isEnd() && current() == '^') {
        consume();
        double exp = parsePower(); // right-associative recursion
        return std::pow(base, exp);
    }
    return base;
}

// unary := '-' unary | primary
double ExpressionParser::parseUnary() {
    skipWhitespace();
    if (!isEnd() && current() == '-') {
        consume();
        return -parseUnary();
    }
    if (!isEnd() && current() == '+') {
        consume();
        return parseUnary();
    }
    return parsePrimary();
}

// primary := number | '(' expression ')'
double ExpressionParser::parsePrimary() {
    skipWhitespace();
    if (isEnd())
        throw std::runtime_error("Unexpected end of expression");

    if (current() == '(') {
        consume(); // '('
        double val = parseExpression();
        skipWhitespace();
        if (isEnd() || current() != ')')
            throw std::runtime_error("Missing closing parenthesis");
        consume(); // ')'
        return val;
    }

    // Parse number (integer or decimal)
    if (std::isdigit(current()) || current() == '.') {
        size_t start = m_pos;
        while (!isEnd() && (std::isdigit(current()) || current() == '.'))
            m_pos++;
        // Handle scientific notation: e.g. 1.5e10
        if (!isEnd() && (current() == 'e' || current() == 'E')) {
            m_pos++;
            if (!isEnd() && (current() == '+' || current() == '-'))
                m_pos++;
            while (!isEnd() && std::isdigit(current()))
                m_pos++;
        }
        std::string numStr = m_expr.substr(start, m_pos - start);
        try {
            return std::stod(numStr);
        } catch (...) {
            throw std::runtime_error("Invalid number: " + numStr);
        }
    }

    throw std::runtime_error(std::string("Unexpected character: ") + current());
}

void ExpressionParser::skipWhitespace() {
    while (!isEnd() && std::isspace(static_cast<unsigned char>(current())))
        m_pos++;
}

char ExpressionParser::current() const {
    return m_expr[m_pos];
}

char ExpressionParser::consume() {
    return m_expr[m_pos++];
}

bool ExpressionParser::isEnd() const {
    return m_pos >= m_expr.size();
}
