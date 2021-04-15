#include <iostream>
#include <string>
#include <vector>
#include "tokenizer.h"
#include "../src/parser.h"
#include "token.h"
#include "util.h"
#include "ast.h"
#include "ast_helpers.h"
using namespace std;

int main(int argc, char* argv[]) {

	vector<string> contents{"print(1 + 2)"};
	Tokenizer tokenizer(contents);
    tokenizer.print();

    Parser parser(&tokenizer);
    Interactive* parse_tree = dynamic_cast<Interactive*>(parser.parse("interactive"));

	cout << endl << "AST as a string:" << endl << *parse_tree << endl;

	return 0;
}
