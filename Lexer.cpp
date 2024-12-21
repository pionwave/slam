
#include "Lexer.hpp"

std::string tokenTypeName(TokenType t) {
    switch (t) {
    case TokenType::T_INSTRUCTION: return "INSTRUCTION";
    case TokenType::T_REGISTER: return "REGISTER";
    case TokenType::T_LABEL: return "LABEL";
    case TokenType::T_INT: return "INT";
    case TokenType::T_LBRACKET: return "LBRACKET";
    case TokenType::T_RBRACKET: return "RBRACKET";
    case TokenType::T_COMMA: return "COMMA";
    case TokenType::T_COLON: return "COLON";
    case TokenType::T_DIRECTIVE: return "DIRECTIVE";
    case TokenType::T_END_OF_FILE: return "END_OF_FILE";
    default: return "UNKNOWN";
    }
}


Lexer::Lexer(const std::string& input, int lineNumber)
    : input_(input), pos_(0), lineNumber_(lineNumber), columnNumber_(1) {
    nextToken();
}

const Token& Lexer::currentToken() const {
    return current_;
}

void Lexer::nextToken() {
    skipSpaces();
    if (pos_ >= input_.size()) {
        current_ = { TokenType::T_END_OF_FILE, "", 0, lineNumber_, (int)input_.size() + 1 };
        return;
    }

    char c = input_[pos_];
    auto makeToken = [&](TokenType t, const std::string& txt = "", int64_t val = 0) {
        current_ = { t, txt, val, lineNumber_, columnNumber_ };
        };

    if (c == '[') {
        makeToken(TokenType::T_LBRACKET, "[");
        advance();
    }
    else if (c == ']') {
        makeToken(TokenType::T_RBRACKET, "]");
        advance();
    }
    else if (c == ',') {
        makeToken(TokenType::T_COMMA, ",");
        advance();
    }
    else if (c == ':') {
        makeToken(TokenType::T_COLON, ":");
        advance();
    }
    else if (std::isdigit((unsigned char)c) || (c == '-' && (pos_ + 1 < input_.size()) && std::isdigit((unsigned char)input_[pos_ + 1]))) {
        int startCol = columnNumber_;
        size_t start = pos_;
        advance();
        while (pos_ < input_.size() && std::isdigit((unsigned char)input_[pos_])) {
            advance();
        }
        std::string numStr = input_.substr(start, pos_ - start);
        makeToken(TokenType::T_INT, numStr, std::stoll(numStr));
        current_.col = startCol;
    }
    else if (std::isalpha((unsigned char)c) || c == '_') {
        int startCol = columnNumber_;
        size_t start = pos_;
        advance();
        while (pos_ < input_.size() && (std::isalnum((unsigned char)input_[pos_]) || input_[pos_] == '_')) {
            advance();
        }
        std::string ident = input_.substr(start, pos_ - start);
        // Convert to uppercase
        for (auto& ch : ident) ch = (char)std::toupper((unsigned char)ch);

        if (isInstruction(ident)) {
            makeToken(TokenType::T_INSTRUCTION, ident);
        }
        else if (isRegister(ident)) {
            makeToken(TokenType::T_REGISTER, ident);
        }
        else {
            makeToken(TokenType::T_LABEL, ident);
        }
        current_.col = startCol;
    }
    else if (c == '.') {
        // read directive
        advance();
        int startCol = columnNumber_;
        size_t start = pos_;
        advance();
        while (pos_ < input_.size() && std::isalpha((unsigned char)input_[pos_])) {
            advance();
        }
        std::string dir = input_.substr(start, pos_ - start);
        for (auto& ch : dir) ch = (char)std::toupper((unsigned char)ch);

        if (dir == "DATA" || dir == "CODE" || dir == "WORD") {
            current_ = { TokenType::T_DIRECTIVE, dir, 0, lineNumber_, startCol };
        }
        else {
            error("Unknown directive: ." + dir);
        }
    }
    else {
        error(std::string("Unexpected character: ") + c);
    }
}

bool Lexer::ended() const {
    return current_.type == TokenType::T_END_OF_FILE;
}


void Lexer::skipSpaces() {
    while (pos_ < input_.size() && (input_[pos_] == ' ' || input_[pos_] == '\t')) {
        advance();
    }
}

void Lexer::advance() {
    if (pos_ < input_.size()) {
        pos_++;
        columnNumber_++;
    }
}

void Lexer::error(const std::string& msg) const {
    throw std::runtime_error("Lexer error at line " + std::to_string(lineNumber_) +
        ", col " + std::to_string(columnNumber_) + ": " + msg);
}

bool Lexer::isInstruction(const std::string& s) const {
    static std::unordered_map<std::string, bool> instrs = {
        {"MOV",true},{"ADD",true},{"SUB",true},{"MUL",true},{"DIV",true},{"AND",true},{"OR",true},
        {"XOR",true},{"SHL",true},{"SHR",true},{"CMP",true},{"JMP",true},{"JE",true},{"JNE",true},
        {"JG",true},{"JL",true},{"JLE", true},{"JGE", true},{ "LOAD",true },{"STORE",true},{"PUSH",true},{"POP",true},{"CALL",true},{"RET",true}
    };
    return instrs.find(s) != instrs.end();
}

bool Lexer::isRegister(const std::string& s) const {
    if (s.size() >= 2 && s[0] == 'R' && std::isdigit((unsigned char)s[1])) {
        int idx = std::stoi(s.substr(1));
        return (idx >= 0 && idx < 16);
    }
    return false;
}

