#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>
#include "token.h"
using namespace std;

class Tokenizer {
	private:
		vector<string> input;
		vector<Token> tokens;
		int length, pos;

		const string delimiters = "\"'&|;:,.()+-=*/%[]#<>";
		const string special_delimiters = "=.+-*/";  // 2+ char op's

		void tokenize();
		void eof(vector<int> indents, int ln);
		bool should_append_op(char c1, char c2, char c3);
		bool normal_delim(vector<string> input, int ln, int& prev, int i);
		void end_of_line(vector<string> input, int ln, int& prev, int i, 
                            bool& in_str, int& string_line_start, string& str_value);
		void string_delim(vector<string> input, int ln, int& prev, int i, 
                            bool& in_str, int& string_line_start, int& string_column_start,
							string& str_value, string termination_char);
		void comment_delim(vector<string> input, int ln, int& prev, int i, vector<int> indents);

	public:
		Tokenizer();
		Tokenizer(vector<string> input);

		Token next_token();
		Token peek();
		Token lookahead(int amt);
		void begin();
		void compound(int amt, string sep="");
		int size();
		void prettyprint();
};

#endif

