#include <iostream>
#include <vector>
#include <regex>
#include <ctype.h>
#include <cstdlib>
#include <algorithm>
#include <string>
#include <tuple>
#include "logging.h"
#include "util.h"
#include "token.h"
#include "tokenizer.h"
using namespace std;


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

//===============================================================
// private members

bool Tokenizer::is_delim(char c) {
    // NOTE: adding '\r' and ' ' here as potential delims
    return (delimiters + "\r ").find(c) != string::npos;
}

bool Tokenizer::is_delim_not_space(char c) {
    return delimiters.find(c) != string::npos;
}

bool Tokenizer::is_delim_not_string(char c) {
    return delimiters_not_string.find(c) != string::npos;
}

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
    else if (c1 == '*') {
        if (c2 == '*') return true;  // **
    }
    return false;
}

bool Tokenizer::tokenize_default(int ln, int& prev, int i) {
    string temp_s;
    int temp_i;
    char c1, c2, c3;

    // check for 2+ character ops, push if found
    if (special_delimiters.find(input[ln][prev]) != string::npos) {
        c1 = input[ln][prev];
        c2 = prev < input[ln].length() ? input[ln][prev+1] : 'o';
        c3 = prev+1 < input[ln].length() ? input[ln][prev+2] : 'o';
        if (should_append_op(c1, c2, c3)) {
            if (input[ln][prev] == '.' && input[ln][prev+1] == '.') {
                temp_s = string(1, c1) + c2 + c3;
                if (DEBUG_TOK) cout << "pushed in default 1: '" << temp_s << "'" << endl;
                tokens.push_back(
                    Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, prev+3}));
                prev = i+2;
            }
            else if (input[ln][i] != '.') {
                temp_s = string(1, c1) + c2;
                if (DEBUG_TOK) cout << "pushed in default 2: '" << temp_s << "'" << endl;
                tokens.push_back(
                    Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, prev+2}));
                prev = i+1;
            }
            return false;
        }
    }

    temp_s = sub(input[ln], prev, i);
    if (ltrim(temp_s) == "") return true;  // sometimes whitespace slips through here
    
    // push the token
    if (DEBUG_TOK) cout << "pushed in default 3: '" << temp_s << "'" << endl;
    if (prev == i) {
        tokens.push_back(
            Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i+1}));
        prev = i + 1;
    } else {
        tokens.push_back(
            Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i}));
        prev = i;
    }
    
    // NOTE: could be 1 or more spaces, want to skip all of them
    if (input[ln][prev] == ' ') {
        temp_i = prev+1;
        while (temp_i < input[ln].size() && input[ln][temp_i] == ' ') {
            temp_i++;
        }
        prev = temp_i;
    }
    
    while (is_delim_not_string(input[ln][prev])) {
        if (tokenize_default(ln, prev, prev+1)) break;
    }
    return false;
}

void Tokenizer::tokenize_string(int ln, int& prev, int i, bool& in_str, 
                                int& string_line_start, int& string_column_start,
                                string& str_value, string& termination_char) {
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
            if (DEBUG_TOK) cout << "pushed in string: '" << str_value << "'" << endl;
            tokens.push_back(
                Token("STRING", str_value, {string_line_start+1, string_column_start}, {ln+1, i+3}));
            prev = i+3;
        }
        else {
            if (DEBUG_TOK) cout << "pushed in string: '" << str_value << "'" << endl;
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
        // check for " vs """ vs '
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

void Tokenizer::end_of_line(int ln, int& prev, int i, bool& in_str, 
                            int& string_line_start, string& str_value) {
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
            if (DEBUG_TOK) cout << "pushed in eol: '" << temp_s << "'" << endl;
            tokens.push_back(
                Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i}));
            prev = i;
            if (input[ln][i] != '\r' && is_delim(input[ln][i])) {
                // this should only be reached if the line does not contain a carriage return
                // as its last char AND there is a single character token remaining
                temp_s = sub(input[ln], prev, i);
                if (DEBUG_TOK) cout << "pushed in eol: '" << temp_s << "'" << endl;
                tokens.push_back(
                    Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i+1}));
                prev = i+1;
            }
        } 
        else if (prev == i && is_delim_not_space(input[ln][i])) {
            temp_s = string(1, input[ln][i]);
            if (DEBUG_TOK) cout << "pushed in eol: '" << temp_s << "'" << endl;
            tokens.push_back(
                Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i=1}));
            prev = i;
        }
        
        // push the NEWLINE
        temp_s = "NEWLINE";
        if (tokens.back().type == "COMMENT") temp_s = "NL";
        if (input[ln][i] == '\r') {
            if (DEBUG_TOK) cout << "pushed in eol: '" << temp_s << "'" << endl;
            tokens.push_back(
                Token(temp_s, "\\r\\n", {ln+1, i}, {ln+1, i+2}));
        }
        else {
            if (DEBUG_TOK) cout << "pushed in eol: '" << temp_s << "'" << endl;
            tokens.push_back(
                Token(temp_s, "", {ln+1, prev}, {ln+1, prev+1}));
        }
    }
    // NOTE: dont want to push a NEWLINE if the string will continue on
}

void Tokenizer::tokenize_comment(int ln, int& prev, int i, vector<int> indents) {
    string temp_s;
    int temp_i;

    // push remaining token if there is one
    if (prev != i) {
        temp_s = sub(input[ln], prev, i);
        if (DEBUG_TOK) cout << "pushed in comment: '" << temp_s << "'" << endl;
        tokens.push_back(
            Token(get_type(temp_s), temp_s, {ln+1, prev}, {ln+1, i}));
        prev = i;
    }
    
    // push the COMMENT
    temp_i = input[ln].find_last_not_of("\r")+1;
    if (DEBUG_TOK) cout << "pushed in comment: '" << sub(input[ln], i, temp_i) << "'" << endl;
    tokens.push_back(
        Token("COMMENT", sub(input[ln], i, temp_i), {ln+1, i}, {ln+1, temp_i}));
    prev = temp_i;

    // weird stuff happens if the comment is the last line of the file with no '\r'
    // mimic'ing that behavior here
    if (input[ln].find('\r') == string::npos) {
        if (DEBUG_TOK) cout << "pushed in comment: '" << "NL" << "'" << endl;
        tokens.push_back(
            Token("NL", "", {ln+1, input[ln].size()}, {ln+1, input[ln].size()}));
        if (DEBUG_TOK) cout << "pushed in comment: '" << "NEWLINE" << "'" << endl;
        tokens.push_back(
            Token("NEWLINE", "", {ln+1, input[ln].size()}, {ln+1, input[ln].size()+1}));
        eof(ln+1, prev, indents);
    }
}

void Tokenizer::eof(int ln, int prev, vector<int> indents) {
    // check if there is one more token that needs pushed
    // if (prev < input[ln-1].size()-1) {
    if (prev < input[ln-1].size()-1 && tokens.back().type != "NEWLINE" && tokens.back().type != "NL") {
        if (DEBUG_TOK) cout << "pushed in eof: '" << sub(input[ln-1], prev, input[ln-1].size()) << "'" << endl;
        string temp_s = sub(input[ln-1], prev, input[ln-1].size());
        tokens.push_back(
            Token(get_type(temp_s), temp_s, {ln, prev}, {ln, input[ln].size()}));
    }

    // if the file doesnt have a '\r' 13 on the last line then a NEWLINE 
    // wont get pushed, it is still needed
    if (tokens.back().type != "NEWLINE" && tokens.back().type != "NL") {
        if (DEBUG_TOK) cout << "pushed in eof: '" << "NEWLINE" << "'" << endl;
        tokens.push_back(
            Token("NEWLINE", "", {ln, input[ln-1].size()-1}, {ln, input[ln-1].size()}));
    }

    // push remaining DEDENTs
    while(indents.size() > 1) {
        if (DEBUG_TOK) cout << "pushed in eof: '" << "DEDENT" << "'" << endl;
        tokens.push_back(
            Token("DEDENT", "", {ln+1, 0}, {ln+1, 0}));
        indents.pop_back();
    }
    
    // push ENDMARKER
    if (DEBUG_TOK) cout << "pushed in eof: '" << "ENDMARKER" << "'" << endl;
    tokens.push_back(
        Token("ENDMARKER", "", {ln+1, 0}, {ln+1, 0}));
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
            eof(ln, prev, indents);
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
        while (code_start < indents.back() && input[ln][code_start] != '#') {
            // lower indentation level
            indents.pop_back();
            tokens.push_back(
                Token("DEDENT", "", {ln+1, code_start}, {ln+1, code_start}));
        }
        
        prev = code_start;
        // iterate over each character in the line
        for (int i=code_start; i < input[ln].size(); i++) {
            if (i < prev) {
                if (DEBUG_TOK) cout << "s: " << +input[ln][i] << endl;
                continue;  // happens with """ and whitespace
            }
            if (DEBUG_TOK) cout << +input[ln][i] << endl;

            // found a delimiter
            if (is_delim(input[ln][i])) {
                if (DEBUG_TOK) cout << "delim: '" << +input[ln][i] << "'" << endl;
                if (input[ln][i] == '#') {
                    // comment found
                    tokenize_comment(ln, prev, i, indents);
                }
                else if (input[ln][i] == '"' || input[ln][i] == '\'') {
                    // string delimiter found (start or termination)
                    tokenize_string(ln, prev, i, in_str, 
                            string_line_start, string_column_start,
                            str_value, termination_char);
                }
                else if (input[ln][i] == '.' && isdigit(input[ln][i+1])) {
                    // 1.00 vs var.attr, continue on floating point cases
                    continue;
                }
                else if (input[ln][i] == '\r' || i == input[ln].size()-1) {
                    // reached end of line
                    end_of_line(ln, prev, i, in_str, string_line_start, str_value);
                }
                else if (in_str) {
                    // found a delimiter inside the string, skip it
                    continue;
                }
                else {
                    // normal delimiter
                    tokenize_default(ln, prev, i);
                }
            }
        }
    }
    // NOTE: this can be reached if the file does not have the EOF char checked for above
    eof(input.size(), prev, indents);
}

// public members
Tokenizer::Tokenizer() {
    pos = 0;
    length = 0;
}

Tokenizer::Tokenizer(vector<string> input) {
	this->input = input;
	this->tokenize();
	pos = 0;
	length = tokens.size();
    this->log();
}

void Tokenizer::tokenize_input(string input) {
    this->reset();
    this->input.push_back(input);
    this->tokenize();
	pos = 0;
	length = tokens.size();
    this->log();
}

void Tokenizer::begin() {
    pos = 0;
}

void Tokenizer::reset() {
    this->input.clear();
    this->tokens.clear();
    pos = 0;
    length = 0;
}

int Tokenizer::size() {
    return this->length;
}

void Tokenizer::print() {
    for (Token t : this->tokens) {
        cout << t << endl;
    }
}

void Tokenizer::log() {
    Logger* logger = Logger::get_instance();
    for (Token t : this->tokens) {
        logger->log((string)t, DEBUG);
    }
}

void Tokenizer::strip() {
    // removing all NL's -- completely empty lines, and COMMENT's
    int before = tokens.size();

    tokens.erase(
        remove_if(tokens.begin(), tokens.end(), 
            [](Token t){return t.type == "NL" || t.type == "COMMENT";}
        ), tokens.end()
    );

    Logger::get_instance()->log("strip() before = " + to_string(before) 
                                + " after = " + to_string(tokens.size()), DEBUG);
}

Token Tokenizer::next_token() {
	if (pos < length) {
		return tokens[pos++];
	}
    throw runtime_error("attempted to get next_token() with no tokens remaining");
}

Token Tokenizer::peek() {
    if (this->pos < this->length) {
        return tokens[pos];
    }
    throw runtime_error("attempted to peek() with no tokens remaining");
}

Token Tokenizer::lookahead(int amt) {
	if (this->pos+amt < this->length) {
	    return tokens[pos+amt];
	}
    return Token();
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

void Tokenizer::rewind(int amt) {
    if (this->pos - amt < 0) {
        throw runtime_error("Cannot rewind " + to_string(amt) + " with pos " + to_string(pos));
    }
    this->pos -= amt;
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
