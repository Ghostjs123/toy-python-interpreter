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
		const string delimiters_not_string = "&|;:,.()+-=*/%[]#<>";
		const string special_delimiters = "=.+-*/";  // 2+ char op's

		// tokenize helpers
		bool is_delim(char c);
		bool is_delim_not_space(char c);
		bool is_delim_not_string(char c);
		bool should_append_op(char c1, char c2, char c3);
		bool tokenize_default(vector<string> input, int ln, int& prev, int i);
		void tokenize_string(vector<string> input, int ln, int& prev, int i, 
                            bool& in_str, int& string_line_start, int& string_column_start,
							string& str_value, string& termination_char);
		void tokenize_comment(vector<string> input, int ln, int& prev, int i, vector<int> indents);
		void end_of_line(vector<string> input, int ln, int& prev, int i, 
                            bool& in_str, int& string_line_start, string& str_value);
		void eof(vector<int> indents, int ln);

		// main function
		void tokenize();

	public:
		Tokenizer();
		Tokenizer(vector<string> input);

		void tokenize_input(string input);
		void begin();
		void reset();
		int size();
		void print();
		void log();

		void find_next();
		Token next_token();
		Token peek();
		Token lookahead(int amt);
		void compound(int amt, string sep="");
};

#endif

