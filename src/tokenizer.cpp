#include <iostream>
#include <vector>
#include <regex>
#include <ctype.h>
#include <cstdlib>
#include "util.h"
#include "token.h"
#include "tokenizer.h"
using namespace std;

#define DEBUG 0

// private members
void Tokenizer::lines_to_tokens() {
	const string delimiters = "\"&|;:,()+-=*/%[]";
    const string special_delimiters = "&|=/";  // 2 char op's
    const string whitespace = "\n\r\t ";

	int prev, length, s_line_num;
    string prev_str, s;
    bool in_str = false;
    for (int ln=0; ln < input.size(); ln++) {
        if (input[ln].size() == 0) continue;
        int start = input[ln].find_first_not_of(whitespace);
        if (input[ln][start] == '#') continue;
        
        prev = 0;
        for (int i=0; i < input[ln].size(); i++) {
            if ((delimiters + ' ').find(input[ln][i]) != string::npos) {
                // push the previous
                prev_str = input[ln].substr(prev, i - prev);
                if (in_str) {
                    if (DEBUG) cout << "s += " << prev_str << endl;
                    s += prev_str;
                } else {
                    if (ltrim(prev_str) != "") {
                        if (DEBUG) cout << "prev " << input[ln].substr(prev, i - prev) << endl;
                        tokens.push_back(Token(ln+1, input[ln].substr(prev, i - prev)));
                    }
                }

                // push the delimiter
                if (input[ln][i] == '"') {
                    if (in_str) {
                        // string end
                        s += "\"";
                        if (DEBUG) cout << "ending string " << s << endl;
                        in_str = false;
                        tokens.push_back(Token(s_line_num, s));
                    } else {
                        // string start
                        s = "\"";
                        if (DEBUG) cout << "starting new string " << s << endl;
                        in_str = true;
                        s_line_num = ln+1;
                    }
                }
                else if (in_str) {
                    // found a delim inside a string
                    s += input[ln][i];
                }
                else if (input[ln][i] != ' ') {
                    if (DEBUG) cout << "delim " << input[ln][i] << " " << i << endl;
                    if (i < input[ln].size() && \
                            special_delimiters.find(input[ln][i]) != string::npos && \
                            input[ln][i] == input[ln][i+1]) {
                        // combine &&, ||, etc
                        string cmbd = "";
                        cmbd += input[ln][i];
                        cmbd += input[ln][i+1];
                        tokens.push_back(Token(ln+1, cmbd));
                        i++;
                    } else {
                        // single char delim
                        tokens.push_back(Token(ln+1, input[ln][i]));
                    }
                }

                // update prev
                if (i < input[ln].size() && (delimiters + ' ').find(input[ln][i]) != string::npos) {
                    prev = i+1;
                } else {
                    prev = i;
                }
            }
        }

        // push the remainder
        if (prev < input[ln].size()) {
            if (in_str) {
                if (DEBUG) cout << "(rem) s += " << input[ln].substr(prev, string::npos) << endl;
                s += input[ln].substr(prev, string::npos);
            } else {
                if (DEBUG) cout << "rem " << input[ln].substr(prev, string::npos) << endl;
                tokens.push_back(Token(ln+1, input[ln].substr(prev, string::npos)));
            }
        }
    }
    if (DEBUG) cout << endl;
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
	lines_to_tokens();
	pos = 0;
	length = tokens.size();
}

Token Tokenizer::next_token() {
	if (pos < length) {
		return tokens[pos++];
	}
	return end_token;
}

Token Tokenizer::peek() {
    if (pos < length) {
        return tokens[pos];
    }
    return end_token;
}

Token Tokenizer::lookahead(int amt) {
	if (pos+amt < length) {
	    return tokens[pos+amt];
	}
    return end_token;
}

void Tokenizer::begin() {
    pos = 0;
}

// combines tokens
void Tokenizer::compound(int amt, string sep) {
    for (int i=pos+1; i < length && i < pos+amt; i++) {
        tokens[pos].append(sep + tokens[i].value);

        if (tokens[pos].line_num != tokens[i].line_num) {
            cout << "Error - compound(): line_num changed from " \
                 << tokens[pos].line_num << " to " \
                 << tokens[i].line_num << endl;
            exit(EXIT_FAILURE);
        }
    }
    for (int i=pos+amt-1; i >= pos+1; i--) {
        tokens.erase(tokens.begin()+i);
        length--;
    }
}

int Tokenizer::size() {
    return tokens.size();
}

#ifdef TOK_MAIN
int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Missing input filename\n";
		return 0;
	}
	if (!file_exists(argv[1])) {
		cout << "No file named \"" << argv[1] << "\"" << endl;
		return 0;
	}

	vector<string> contents = read_lines(argv[1]);
    cout << "contents:" << endl;
    for (int i=0; i < contents.size(); i++) {
        cout << i << ": " << contents[i] << endl;
    }
    cout << endl;

	Tokenizer tokenizer(contents);

	Token token;
    cout << "tokens:" << endl;
	while ((token = tokenizer.next_token()).value != "EOF")
   		cout << token.line_num << ": " << token.value << endl;
	return 0;
}
#endif
