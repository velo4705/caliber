#include "expression_parser.h"
#include <cmath>
#include <cctype>
#include <stdexcept>
#include <string>
#include <algorithm>

static constexpr double PI = 3.14159265358979323846;
static constexpr double E  = 2.71828182845904523536;

ExpressionParser::ExpressionParser()
    : m_pos(0), m_angleMode(ParserAngleMode::Degrees) {}

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

// power := postfix ('^' unary)*  (right-associative)
double ExpressionParser::parsePower() {
    double base = parsePostfix(parseUnary());
    skipWhitespace();
    if (!isEnd() && current() == '^') {
        consume();
        double exp = parsePower();
        return std::pow(base, exp);
    }
    return base;
}

// postfix: handles n! factorial
double ExpressionParser::parsePostfix(double val) {
    skipWhitespace();
    if (!isEnd() && current() == '!') {
        consume();
        return factorial(val);
    }
    return val;
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

// primary := number | identifier | '(' expression ')'
double ExpressionParser::parsePrimary() {
    skipWhitespace();
    if (isEnd())
        throw std::runtime_error("Unexpected end of expression");

    // Parenthesised expression
    if (current() == '(') {
        consume();
        double val = parseExpression();
        skipWhitespace();
        if (isEnd() || current() != ')')
            throw std::runtime_error("Missing closing parenthesis");
        consume();
        return val;
    }

    // Identifier: function name or constant
    if (std::isalpha(static_cast<unsigned char>(current())) || current() == '_') {
        std::string name = parseIdentifier();
        // Convert to lowercase for case-insensitive matching
        std::string lower = name;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

        // Constants
        if (lower == "pi" || lower == "π") return PI;
        if (lower == "e")                  return E;

        // Functions — expect '(' arg ')'
        skipWhitespace();
        if (!isEnd() && current() == '(') {
            consume();
            double arg = parseExpression();
            skipWhitespace();
            if (isEnd() || current() != ')')
                throw std::runtime_error("Missing closing parenthesis after function");
            consume();
            return callFunction(lower, arg);
        }
        throw std::runtime_error("Unknown identifier: " + name);
    }

    // Number
    if (std::isdigit(static_cast<unsigned char>(current())) || current() == '.') {
        size_t start = m_pos;
        while (!isEnd() && (std::isdigit(static_cast<unsigned char>(current())) || current() == '.'))
            m_pos++;
        // Scientific notation
        if (!isEnd() && (current() == 'e' || current() == 'E')) {
            m_pos++;
            if (!isEnd() && (current() == '+' || current() == '-'))
                m_pos++;
            while (!isEnd() && std::isdigit(static_cast<unsigned char>(current())))
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

double ExpressionParser::callFunction(const std::string& name, double arg) {
    double rad = toRadians(arg);

    if (name == "sin")   return std::sin(rad);
    if (name == "cos")   return std::cos(rad);
    if (name == "tan") {
        // tan(90°) is undefined
        double r = std::fmod(rad, PI);
        if (std::abs(r - PI / 2.0) < 1e-12)
            throw std::runtime_error("tan is undefined at 90°");
        return std::tan(rad);
    }
    if (name == "asin") {
        if (arg < -1.0 || arg > 1.0)
            throw std::runtime_error("asin domain error: argument must be in [-1, 1]");
        double r = std::asin(arg);
        return (m_angleMode == ParserAngleMode::Degrees) ? r * 180.0 / PI
             : (m_angleMode == ParserAngleMode::Gradians) ? r * 200.0 / PI : r;
    }
    if (name == "acos") {
        if (arg < -1.0 || arg > 1.0)
            throw std::runtime_error("acos domain error: argument must be in [-1, 1]");
        double r = std::acos(arg);
        return (m_angleMode == ParserAngleMode::Degrees) ? r * 180.0 / PI
             : (m_angleMode == ParserAngleMode::Gradians) ? r * 200.0 / PI : r;
    }
    if (name == "atan") {
        double r = std::atan(arg);
        return (m_angleMode == ParserAngleMode::Degrees) ? r * 180.0 / PI
             : (m_angleMode == ParserAngleMode::Gradians) ? r * 200.0 / PI : r;
    }
    if (name == "log" || name == "log10") return std::log10(arg);
    if (name == "ln")                     return std::log(arg);
    if (name == "log2")                   return std::log2(arg);
    if (name == "sqrt")                   return std::sqrt(arg);
    if (name == "cbrt")                   return std::cbrt(arg);
    if (name == "abs")                    return std::abs(arg);
    if (name == "ceil")                   return std::ceil(arg);
    if (name == "floor")                  return std::floor(arg);
    if (name == "exp")                    return std::exp(arg);
    if (name == "sinh")                   return std::sinh(toRadians(arg));
    if (name == "cosh")                   return std::cosh(toRadians(arg));
    if (name == "tanh")                   return std::tanh(toRadians(arg));

    throw std::runtime_error("Unknown function: " + name);
}

double ExpressionParser::toRadians(double val) const {
    switch (m_angleMode) {
    case ParserAngleMode::Degrees:  return val * PI / 180.0;
    case ParserAngleMode::Gradians: return val * PI / 200.0;
    default:                        return val;
    }
}

double ExpressionParser::factorial(double n) const {
    if (n < 0 || n != std::floor(n))
        throw std::runtime_error("Factorial requires a non-negative integer");
    if (n > 170)
        throw std::runtime_error("Factorial overflow");
    double result = 1.0;
    for (int i = 2; i <= static_cast<int>(n); ++i)
        result *= i;
    return result;
}

std::string ExpressionParser::parseIdentifier() {
    std::string id;
    while (!isEnd() && (std::isalnum(static_cast<unsigned char>(current())) || current() == '_'))
        id += consume();
    return id;
}

void ExpressionParser::skipWhitespace() {
    while (!isEnd() && std::isspace(static_cast<unsigned char>(current())))
        m_pos++;
}

char ExpressionParser::current() const { return m_expr[m_pos]; }
char ExpressionParser::consume()       { return m_expr[m_pos++]; }
bool ExpressionParser::isEnd()   const { return m_pos >= m_expr.size(); }
