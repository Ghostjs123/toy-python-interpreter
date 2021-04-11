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


template <class T>
PyObject interpret(T tree, Stack &stack, string mode) {
	// if (mode == "file") {
    // 	return ((File)tree).evaluate(stack);
	// }
	// else if (mode == "interactive") {
    // 	return ((Interactive)tree).evaluate(stack);
	// }
	return tree.evaluate(stack);
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		// interpreting input file
		vector<string> contents = read_lines(argv[1]);

		Tokenizer tokenizer(contents);
		if (argc > 2 && argv[2] == (string)"-v") {
			tokenizer.print();
		}

		Parser parser(&tokenizer, "file");
    	File* parse_tree = dynamic_cast<File*>(parser.parse());
		
		cout << endl << "AST as a string:" << endl << *parse_tree << endl;

		cout << endl << "stdout:" << endl;;
		Stack stack;
    	interpret(*parse_tree, stack, parser.mode);

		delete parse_tree;

	} else {
		// interactive terminal
		cout << ">>> ";
		// TODO:
		// NOTE: parse should return an Interactive
	}

	return 0;
}
