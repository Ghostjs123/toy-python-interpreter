#include <iostream>
#include <string>
#include <iomanip>
#include <tuple>
#include "token.h"
#include "util.h"
using namespace std;

Token::Token() {
    this->type = "unknown";
    this->value = "undefined";
    this->line_start = -1;
    this->column_start = -1;
    this->line_end = -1;
    this->column_end = -1;
}
Token::Token(string type, char value, tuple<int, int> start, tuple<int, int> end) {
    this->type = type;
    this->value = string(1, value);
    this->line_start = get<0>(start);
    this->column_start = get<1>(start);
    this->line_end = get<0>(end);
    this->column_end = get<1>(end);
}
Token::Token(string type, string value, tuple<int, int> start, tuple<int, int> end){
    this->type = type;
    this->value = value;
    this->line_start = get<0>(start);
    this->column_start = get<1>(start);
    this->line_end = get<0>(end);
    this->column_end = get<1>(end);
}

void Token::append(string s, Token t) {
    if (this->line_start != t.line_start) {
        throw runtime_error("Attempted to concat:\n"
             + (string)(*this) + "\n" + (string)t + "\n"
             + "line numbers are not the same");
    }
    this->value += (s + t.value);
    this->column_end = t.column_end;
}

string Token::as_string() {
    string pos = to_string(line_start) + "," 
               + to_string(column_start) + "-" 
               + to_string(line_end) + "," 
               + to_string(column_end) + ":";
    return pos + "\t" + type + "\t\'" + value + "\'"; 
}

ostream& operator<<(ostream& os, const Token& token) {
    string pos = to_string(token.line_start) + "," 
               + to_string(token.column_start) + "-" 
               + to_string(token.line_end) + "," 
               + to_string(token.column_end) + ":";
    string val = "\'" + token.value + "\'";
    os << left << setw(20) << pos
       << left << setw(15) << token.type 
       << left << setw(15) << val;
    return os;
}

bool operator==(const Token& lhs, const Token& rhs) {
    return lhs.type == rhs.type && lhs.value == rhs.value;
}

bool operator!=(const Token& lhs, const Token& rhs) {
    return !(lhs == rhs);
}

Token::operator string() {
    return to_string(line_start) + "," + to_string(column_start)
         + "-" + to_string(line_end) + "," + to_string(column_end) + ": " +
         type + " '" + value + "'";
}
