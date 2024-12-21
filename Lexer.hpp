// Slam Assembler (C) 2025 Lynton "Pionwave" Schneider

#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>

enum class TokenType {
    T_INSTRUCTION = 0,
    T_REGISTER = 1,
    T_LABEL = 2,
    T_INT = 3,
    T_LBRACKET = 4,
    T_RBRACKET = 5,
    T_COMMA = 6,
    T_COLON = 7,
    T_DIRECTIVE = 8,
    T_END_OF_FILE = 9
};

struct Token {
    TokenType type;
    std::string text;
    int64_t value;
    int line;
    int col;
};

class Lexer {
public:
    Lexer(const std::string& input, int lineNumber);

    const Token& currentToken() const;
    void nextToken();
    bool ended() const;

private:
    void skipSpaces();
    void advance();

    void error(const std::string& msg) const;

    bool isInstruction(const std::string& s) const;
    bool isRegister(const std::string& s) const;

    std::string input_;
    size_t pos_;
    int lineNumber_;
    int columnNumber_;
    Token current_;
};