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

		void lines_to_tokens();
		void tokenize();

	public:
		Token end_token = Token(-1, "EOF");

		Tokenizer();
		Tokenizer(vector<string> input);

		Token next_token();
		Token peek();
		Token lookahead(int amt);
		void begin();
		void compound(int amt, string sep="");
		int size();
};

#endif

