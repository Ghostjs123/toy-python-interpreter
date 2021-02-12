#include <iostream>
#include <string>
#include "token.h"
using namespace std;

Token::Token() {
    this->line_num = -1;
    this->value = "undefined";
}
Token::Token(int line_num, string value) { 
    this->line_num = line_num;
    this->value = value; 
}
Token::Token(int line_num, char value) { 
    this->line_num = line_num;
    this->value = string(1, value); 
}

void Token::append(string s) {
    value += s;
}

ostream& operator<<(ostream& os, const Token& token) {
    os << "Token: " << token.line_num << ": \"" << token.value << "\"";
    return os;
}

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.line_num == rhs.line_num && lhs.value == rhs.value;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs.line_num == rhs.line_num && lhs.value == rhs.value);
}
