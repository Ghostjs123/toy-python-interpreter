#include <iostream>
#include <sstream>
#include <streambuf>
#include <vector>
#include <string>
#include <tuple>
#include "parser.h"
#include "tokenizer.h"
#include "ast.h"
#include "stack.h"
#include "util.h"
using namespace std;

#define PRINT_TOKENS 1

PyObject interpret(Statements tree, Stack &stack) {
    return tree.evaluate(stack);
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		// interpreting input file
		vector<string> contents = read_lines(argv[1]);

		Tokenizer tokenizer(contents);
		if (argc > 2 && ((string)argv[2]) == "-v") {
			Token token;
			cout << "tokens:" << endl;
			if (tokenizer.size() == 0) {
				return 0;
			}
			while ((token = tokenizer.next_token()).value != "EOF") {
				cout << token.line_num << ": " << token.value << endl;
			}
			cout << endl;
		}

		Parser parser(&tokenizer);
    	Statements parse_tree = parser.parse();

		Stack stack;
    	interpret(parse_tree, stack);

	} else {
		// interactive terminal
		// TODO:
	}

	return 0;
}
