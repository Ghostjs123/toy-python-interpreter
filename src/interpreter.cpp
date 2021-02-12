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
using namespace std;

#define PRINT_TOKENS 1

PyObject interpret(Expression tree, Stack &stack) {
    return tree.evaluate(stack);
}

tuple<PyObject, string> run_test_string(string s) {
	vector<string> tmp_vec;
	tmp_vec.push_back(s);

	cout << "\ninitial input:" << endl;
	cout << s << endl;

	Tokenizer tokenizer(tmp_vec);
	if (PRINT_TOKENS) {
		Token token;
		cout << "tokens:" << endl;
		while ((token = tokenizer.next_token()).value != "EOF") {
			cout << token.line_num << ": " << token.value << endl;
		}
		cout << endl;
	}

	Parser parser(&tokenizer);

    Expression parse_tree = parser.parse();
	cout << "running parse_tree:\n" << parse_tree << endl << endl;

	stringstream buffer;
	streambuf *old = cout.rdbuf(buffer.rdbuf());

	Stack stack;
    PyObject res = interpret(parse_tree, stack);

	string stdout_res = buffer.str();
	cout.rdbuf(old);

	return tuple<PyObject, string>(res, stdout_res);
}

#ifdef INT_MAIN
int main(int argc, char* argv[]) {
	vector<string> contents{
		"1.0 - 1.0 - 1.0 * 1.0",
		"1 - 1.0",
		"print(\"1+2\")",
		"print(\"1+2:\", 1+2)"
	};

	for (int i=0; i < contents.size(); i++) {
		cout << "==============" << endl;
		tuple<PyObject, string> res = run_test_string(contents.at(i));
		cout << "result:\n" << get<0>(res).as_string() << endl;
		if (get<1>(res) != "") {
			cout << "\nstdout_res:\n" << get<1>(res) << endl;
		}
		if (i < contents.size()-1) {
			cout << endl;
		}
	}
	cout << "==============" << endl;
	cout << "Done." << endl;

	return 0;
}
#endif
