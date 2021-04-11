#include <iostream>
#include <vector>
#include <regex>
#include <ctype.h>
#include <cstdlib>
#include <string>
#include <tuple>
#include "util.h"
#include "token.h"
#include "tokenizer.h"
using namespace std;

#define DEBUG 1

// NOTE: good source on how indentation is handled
// https://docs.python.org/2.0/ref/indentation.html

// Specifically:
/* Before the first line of the file is read, a single zero is pushed on the stack; 
   this will never be popped off again. The numbers pushed on the stack will always 
   be strictly increasing from bottom to top. At the beginning of each logical line, 
   the line's indentation level is compared to the top of the stack. If it is equal, 
   nothing happens. If it is larger, it is pushed on the stack, and one INDENT token 
   is generated. If it is smaller, it must be one of the numbers occurring on the stack; 
   all numbers on the stack that are larger are popped off, and for each number popped 
   off a DEDENT token is generated. At the end of the file, a DEDENT token is generated 
   for each number remaining on the stack that is larger than zero.
*/
// This is incredibly important when it comes to parsing productions like:
// block:
//     | NEWLINE INDENT statements DEDENT 
//     | simple_stmt

// implementation of tokenization logic written in python
// https://github.com/python/cpython/blob/85fd9f4e45ee95e2608dbc8cc6d4fe28e4d2abc4/Lib/tokenize.py#L45

// python3 -m tokenize [filename]

// NOTE: example tokenization at bottom of file


//===============================================================
// helper functions

template <class T>
bool vector_contains(vector<T> vec, T x) {
    return find(vec.begin(), vec.end(), x) != vec.end();
}

// NOTE: _sub and sub are neeed because c++'s builtin .substr() method
// does not like carriage returns '\r' at the end of the string
string _sub(string s, int prev, int i) {
    string sn = "";
    for (int _i=prev; _i < i; _i++) {
        sn += s[_i];
    }
    return sn;
}
string sub(string s, int prev, int i) {
    if (i-prev == 1 || i == prev) {
        return string(1, s[prev]);
    }
    return _sub(s, prev, i);
}

string get_type(string s) {
	string ops = "&|;:,.()+-=*/%[]<>";
    if (ops.find(s) != string::npos) {
        return "OP";
    }
    if (s.length() > 1 && ops.find(s[0]) != string::npos && ops.find(s[1]) != string::npos) {
        return "OP";
    }
    if (is_number(s)) {
        return "NUMBER";
    }
    return "NAME";
}

// TODO: handle 2+ char op's

// TODO: flush out error messages, ex:
// File "python-samples/random_tests.py", line 7
//     print("asd")
//                ^
// IndentationError: unindent does not match any outer indentation level

//===============================================================
// private members

// NOTE: this is a ton of boilerplate, might be a more simple way to handle this
bool Tokenizer::should_append_op(char c1, char c2, char c3) {

    if (c1 == '=') {
        if (c2 == '=') return true;  // ==
    }
    else if (c1 == '+') {
        if (c2 == '=') return true;  // +=
    }
    else if (c1 == '-') {
        if (c2 == '=') return true;  // -=
    }
    else if (c1 == '*') {
        if (c2 == '=') return true;  // *=
        if (c2 == '*') return true;  // **
    }
    else if (c1 == '/') {
        if (c2 == '=') return true;  // /=
        if (c2 == '/') return true;  // //
    }
    else if (c1 == '>') {
        if (c2 == '=') return true;  // >=
    }
    else if (c1 == '<') {
        if (c2 == '=') return true;  // <=
    }
    else if (c1 == '.') {
        if (c2 == '.' && c3 == '.') return true;  // ..
    }
    return false;
}

void Tokenizer::eof(vector<int> indents, int ln) {
    while(indents.size() > 1) {
        tokens.push_back(
            Token("DEDENT", "", {ln+1, 0}, {ln+1, 0}));
        indents.pop_back();
    } 
    tokens.push_back(
        Token("ENDMARKER", "", {ln+1, 0}, {ln+1, 0}));
}

bool Tokenizer::normal_delim(vector<string> input, int ln, int& prev, int i) {
    string temp_s;
    int temp_i;
    char c1, c2, c3;

    if (special_delimiters.find(input[ln][prev]) != string::npos) {
        // potentially 2+ char op found
        c1 = input[ln][prev];
        c2 = prev < input[ln].length() ? input[ln][prev+1] : 'o';
        c3 = prev+1 < input[ln].length() ? input[ln][prev+2] : 'o';
        if (should_append_op(c1, c2, c3)) {
            if (input[ln][prev] == '.' && input[ln][prev+1] == '.') {
                temp_s = string(1, c1) + c2 + c3;
                tokens.push_back(
                    Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, prev+3}));
                prev = i+2;
            }
            else if (input[ln][i] != '.') {
                temp_s = string(1, c1) + c2;
                tokens.push_back(
                    Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, prev+2}));
                prev = i+1;
            }
            return false;
        }
    }
    temp_s = sub(input[ln], prev, i);
    if (ltrim(temp_s) == "") return true;  // sometimes whitespace slips through here
    
    tokens.push_back(
        Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i}));
    
    if (input[ln][i] == ' ') {
        // NOTE: could be 1 or more spaces, want to skip all of them
        temp_i = i+1;
        while (temp_i < input[ln].size() && input[ln][temp_i] == ' ') {
            temp_i++;
        }
        prev = temp_i;
    }
    else {
        if (prev == i) {
            prev = i + 1;
        }
        else {
            prev = i;
        }
    }
    return false;
}

void Tokenizer::end_of_line(vector<string> input, int ln, int& prev, int i, 
                            bool& in_str, int& string_line_start, string& str_value) {
    string temp_s;

    // NOTE: last line of file might not have a carriage return
    if (in_str) {
        // string will continue onto the next line, so append to str_value
        // NOTE: dont want the '\r' in the substring
        if (string_line_start == ln) {
            str_value += sub(input[ln], prev, i-1);
        }
        else {
            // NOTE: this intentionally includes leading whitespace
            str_value += sub(input[ln], 0, input[ln].size()-1);
        }
        str_value += "\\r\\n";
    }
    else {
        // push the remaining token if there is one
        if (prev != i) {
            temp_s = sub(input[ln], prev, i);
            tokens.push_back(
                Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i}));
            prev = i;
            if (input[ln][i] != '\r' && delimiters.find(input[ln][i]) != string::npos) {
                // this should only be reached if the line does not contain a carriage return
                // as its last char AND there is a single character token remaining
                temp_s = sub(input[ln], prev, i);
                tokens.push_back(
                    Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i+1}));
                prev = i+1;
            }
        }
        // push the NEWLINE
        if (input[ln][i] == '\r') {
            tokens.push_back(
                Token("NEWLINE", "\\r\\n", {ln+1, i}, {ln+1, i+2}));
        }
        else {
            tokens.push_back(
                Token("NEWLINE", "", {ln+1, prev}, {ln+1, prev+1}));
        }
    }
    // NOTE: dont want to push a NEWLINE if the string will continue on
}

void Tokenizer::string_delim(vector<string> input, int ln, int& prev, int i, 
                            bool& in_str, int& string_line_start, int& string_column_start,
                            string& str_value, string termination_char) {
    if (in_str) {
        // make sure the delimiter terminates the string
        if (input[ln][i] == '"') {
            if (input[ln][i+1] == '"' && input[ln][i+2] == '"') {
                if (termination_char != "\"\"\"") return;
            }
            else if (termination_char != "\"") return;
        }
        else if (termination_char != "'") return;
        
        // update str_value
        if (string_line_start < ln) {
            // dont want to ommit leading whitespace, prev = code_start at this point
            str_value += sub(input[ln], 0, i);
        }
        else {
            str_value += sub(input[ln], prev, i);
        }
        str_value += termination_char;

        // push STRING token
        if (termination_char == "\"\"\"") {
            tokens.push_back(
                Token("STRING", str_value, {string_line_start+1, string_column_start}, {ln+1, i+3}));
            prev = i+3;
        }
        else {
            tokens.push_back(
                Token("STRING", str_value, {string_line_start+1, string_column_start}, {ln+1, i+1}));
            prev = i+1;
        }
        in_str = false;
    }
    else {
        // starting a new string
        in_str = true;
        string_line_start = ln;
        string_column_start = i;
        // check for " vs """
        if (input[ln][i] == '"') {
            if (input[ln][i+1] == '"' && input[ln][i+2] == '"') {
                termination_char = "\"\"\"";
                prev = i+3;
            } 
            else {
                termination_char = "\"";
                prev = i+1;
            }
        }
        else {
            termination_char = "'";
            prev = i+1;
        }
        str_value = termination_char;
    }
}

void Tokenizer::comment_delim(vector<string> input, int ln, int& prev, int i, vector<int> indents) {
    string temp_s;
    int temp_i;

    if (prev != i) {
        // push remaining token if there is one
        temp_s = sub(input[ln], prev, i);
        tokens.push_back(
            Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i}));
        prev = i;
    }
    temp_i = input[ln].find_last_not_of("\r")+1;
    tokens.push_back(
        Token("COMMENT", sub(input[ln], i, temp_i), {ln+1, i}, {ln+1, temp_i}));
    prev = temp_i;
    if (input[ln].find('\r') == string::npos) {
        // weird stuff happens if the comment is the last line of the file with no '\r'
        tokens.push_back(
            Token("NL", "", {ln+1, input[ln].size()}, {ln+1, input[ln].size()}));
        tokens.push_back(
            Token("NEWLINE", "", {ln+1, input[ln].size()}, {ln+1, input[ln].size()+1}));
        this->eof(indents, ln+1);
        return;
    }
}

void Tokenizer::tokenize() {
    const string whitespace = "\n\r\t ";
    vector<int> indents{0};

	int prev, length, string_line_start, string_column_start, temp_i;
    string termination_char, temp_s, str_value = "";
    bool in_str = false;

    // iterate over each line in the file
    for (int ln=0; ln < input.size(); ln++) {
        int code_start = input[ln].find_first_not_of("\t ");  // intentionally ommiting '\r' and '\n'
        if (input[ln][code_start] == '\r') {
            // empty line
            tokens.push_back(
                Token("NL", "\\r\\n", {ln+1, code_start}, {ln+1, code_start+2}));
            continue;
        }
        if (+input[ln][code_start] == 0) {
            // EOF
            eof(indents, ln);
            return;
        }

        if (code_start > indents.back()) {
            // greater indentation level
            indents.push_back(code_start);
            tokens.push_back(
                Token("INDENT", sub(input[ln], 0, code_start), {ln+1, 0}, {ln+1, code_start}));
        }
        if (code_start < indents.back() && !vector_contains<int>(indents, code_start)) {
            // error case
            string msg = "line " + to_string(ln+1) + "\nunindent does not match any outer indentation level";
            throw runtime_error(msg);
        }
        while (code_start < indents.back()) {
            // lower indentation level
            indents.pop_back();
            tokens.push_back(
                Token("DEDENT", "", {ln+1, code_start}, {ln+1, code_start}));
        }
        
        prev = code_start;
        // iterate over each character in the line
        for (int i=code_start; i < input[ln].size(); i++) {
            if (i < prev) continue;  // happens with """ and whitespace

            // found a delimiter
            // NOTE: adding '\r' and ' ' here as potential delims
            if ((delimiters + "\r ").find(input[ln][i]) != string::npos) {
                if (input[ln][i] == '#') {
                    // comment found
                    comment_delim(input, ln, prev, i, indents);
                }
                else if (input[ln][i] == '"' || input[ln][i] == '\'') {
                    // string delimiter found (start or termination)
                    string_delim(input, ln, prev, i, 
                            in_str, string_line_start, string_column_start,
                            str_value, termination_char);
                }
                else if (input[ln][i] == '.' && isdigit(input[ln][i+1])) {
                    // 1.00 vs var.attr, continue on floating point cases
                    continue;
                }
                else if (input[ln][i] == '\r' || i == input[ln].size()-1) {
                    // reached end of line
                    end_of_line(input, ln, prev, i, in_str, string_line_start, str_value);
                }
                else if (in_str) {
                    // found a delimiter inside the string, skip it
                    continue;
                }
                else {
                    // normal delimiter
                    normal_delim(input, ln, prev, i);
                    while ((delimiters + "\r ").find(input[ln][prev]) != string::npos) {
                        if (normal_delim(input, ln, prev, prev+1)) break;
                    }
                }
            }
        }
    }
    // NOTE: this can be reached if the file does not have the EOF char checked for above
    eof(indents, input.size());
}

// public members
Tokenizer::Tokenizer() {
    vector<string> empty;
    this->input = empty;
    pos = 0;
    length = 0;
}

Tokenizer::Tokenizer(vector<string> input) {
	this->input = input;
	this->tokenize();
	pos = 0;
	length = tokens.size();
}

Token Tokenizer::next_token() {
	if (pos < length) {
		return tokens[pos++];
	}
    throw runtime_error("ERROR: attempted to get next_token() with no tokens remaining");
}

Token Tokenizer::peek() {
    if (this->pos < this->length) {
        return tokens[pos];
    }
    throw runtime_error("ERROR: attempted to peek() with no tokens remaining");
}

Token Tokenizer::lookahead(int amt) {
	if (this->pos+amt < this->length) {
	    return tokens[pos+amt];
	}
    throw runtime_error("ERROR: attempted to lookahead() with no tokens remaining");
}

void Tokenizer::begin() {
    pos = 0;
}

// combines tokens
void Tokenizer::compound(int amt, string sep) {
    for (int i=pos+1; i < length && i < pos+amt; i++) {
        tokens[pos].append(sep, tokens[i]);
    }
    for (int i=pos+amt-1; i >= pos+1; i--) {
        tokens.erase(tokens.begin()+i);
        length--;
    }
}

int Tokenizer::size() {
    return this->length;
}

void Tokenizer::print() {
    for (Token t : this->tokens) {
        cout << t << endl;
    }
}

//===============================================================

// example tokeniztion
// input file:
/*
if True:
    print("1) worked")
    x.y

else:
        print("1) something went wrong")
        x is not 1

"""
asd
"""

*/

// tokens
/*
0,0-0,0:            ENCODING       'utf-8'        
1,0-1,2:            NAME           'if'
1,3-1,7:            NAME           'True'
1,7-1,8:            OP             ':'
1,8-1,10:           NEWLINE        '\r\n'
2,0-2,4:            INDENT         '    '
2,4-2,9:            NAME           'print'        
2,9-2,10:           OP             '('
2,10-2,21:          STRING         '"1) worked"'
2,21-2,22:          OP             ')'
2,22-2,24:          NEWLINE        '\r\n'
3,4-3,5:            NAME           'x'
3,5-3,6:            OP             '.'
3,6-3,7:            NAME           'y'
3,7-3,9:            NEWLINE        '\r\n'
4,0-4,2:            NL             '\r\n'
5,0-5,0:            DEDENT         ''
5,0-5,4:            NAME           'else'
5,4-5,5:            OP             ':'
5,5-5,7:            NEWLINE        '\r\n'
6,0-6,8:            INDENT         '        '
6,8-6,13:           NAME           'print'
6,13-6,14:          OP             '('
6,14-6,39:          STRING         '"1) something went wrong"'
6,39-6,40:          OP             ')'
6,40-6,42:          NEWLINE        '\r\n'
7,8-7,9:            NAME           'x'
7,10-7,12:          NAME           'is'
7,13-7,16:          NAME           'not'
7,17-7,18:          NUMBER         '1'
7,18-7,20:          NEWLINE        '\r\n'
8,0-8,2:            NL             '\r\n'
9,0-9,0:            DEDENT         ''
9,0-11,3:           STRING         '"""\r\nasd\r\n"""'
11,3-11,5:          NEWLINE        '\r\n'
12,0-12,0:          ENDMARKER      ''
*/
