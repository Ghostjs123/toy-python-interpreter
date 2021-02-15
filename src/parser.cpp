#include <iostream>
#include <string>
#include <vector>
#include "tokenizer.h"
#include "parser.h"
#include "objects/token.h"
#include "util.h"
#include "ast/ast.h"
#include "ast/ast_helpers.h"
using namespace std;

Parser::Parser(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
}
Statements Parser::parse() {
    tokenizer->begin();
    return Statements(tokenizer);
}

#ifdef PARS_MAIN
int main(int argc, char* argv[]) {

	vector<string> contents{"print(1 + 2)"};
	Tokenizer tokenizer(contents);

	Token tok;
	cout << "Contents:" << endl;
	while((tok = tokenizer.next_token()) != tokenizer.end_token) {
		cout << tok << endl;
	}
	cout << endl;

    Parser parser(&tokenizer);
    AST parse_tree = parser.parse();
	cout << parse_tree << endl;

	cout << "Rem:" << endl;
	while((tok = tokenizer.next_token()) != tokenizer.end_token) {
		cout << tok << endl;
	}
	cout << "Done." << endl;

	return 0;
}
#endif
