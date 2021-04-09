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
		if (argc > 2 && argv[2] == (string)"-v") {
			tokenizer.prettyprint();
		}

		Parser parser(&tokenizer, "file");
    	Statements parse_tree = parser.parse_statements();
		
		cout << endl << "AST as a string:" << endl << parse_tree << endl;

		cout << endl << "stdout:" << endl;;
		Stack stack;
    	interpret(parse_tree, stack);

	} else {
		// interactive terminal
		cout << ">>> ";
		// TODO:
		// NOTE: parse should return an Interactive
	}

	return 0;
}
