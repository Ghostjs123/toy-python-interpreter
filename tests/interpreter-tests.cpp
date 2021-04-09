#define CATCH_CONFIG_MAIN
#include "../lib/catch.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <streambuf>
#include "tokenizer.h"
#include "../src/parser.h"
#include "util.h"
#include "ast.h"
#include "interpreter.h"
#include "stack.h"


// TODO: write many, many, test cases
// NOTE: this will break when I change interpret


// EXAMPLE CODE FOR LATER WHEN I WRITE MORE CASES
/*
tuple<PyObject, string> run_test_string(string s) {
	vector<string> tmp_vec;
	tmp_vec.push_back(s);

	cout << "\ninitial input:" << endl;
	cout << s << endl;

	Tokenizer tokenizer(tmp_vec);
	if (PRINT_TOKENS) {
		Token token;
		cout << "tokens:" << endl;
		if (tokenizer.size() == 0) {
			cout << "empty" << endl << endl;
			return tuple<PyObject, string>(PyObject(), "");
		}
		while ((token = tokenizer.next_token()).value != "EOF") {
			cout << token.line_num << ": " << token.value << endl;
		}
		cout << endl;
	}

	Parser parser(&tokenizer);
    Statements parse_tree = parser.parse();
	cout << "running parse_tree:\n" << parse_tree << endl << endl;

	stringstream buffer;
	streambuf *old = cout.rdbuf(buffer.rdbuf());

	Stack stack;
    PyObject res = interpret(parse_tree, stack);

	string stdout_res = buffer.str();
	cout.rdbuf(old);

	return tuple<PyObject, string>(res, stdout_res);
}
*/

/*
// some hard coded tests
    vector<string> contents;
    contents.push_back("1.0 - 1.0 - 1.0 * 1.0");
    contents.push_back("1 - 1.0");
    contents.push_back("print(\"1+2\")");
    contents.push_back("print(\"1+2:\", 1+2)");
    
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
*/

TEST_CASE("Interpreter Test - 1 + 2", "[interpreter]") {
    vector<string> contents{"1 + 2"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Statements parse_tree = parser.parse();
    Stack stack;
    PyObject res = interpret(parse_tree, stack);
    REQUIRE( res == PyObject(3, "int") );
}

TEST_CASE("Interpreter Test - 1 - 2", "[interpreter]") {
    vector<string> contents{"1 - 2"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Statements parse_tree = parser.parse();
    Stack stack;
    PyObject res = interpret(parse_tree, stack);
    REQUIRE( res == PyObject(-1, "int") );
}

TEST_CASE("Interpreter Test - 1 * 2", "[interpreter]") {
    vector<string> contents{"1 * 2"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Statements parse_tree = parser.parse();
    Stack stack;
    PyObject res = interpret(parse_tree, stack);
    REQUIRE( res == PyObject(2, "int") );
}

TEST_CASE("Interpreter Test - 1.0 * 2", "[interpreter]") {
    vector<string> contents{"1.0 * 2"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Statements parse_tree = parser.parse();
    Stack stack;
    PyObject res = interpret(parse_tree, stack);
    REQUIRE( res == PyObject(2.0, "float") );
}

TEST_CASE("Interpreter Test - print(1 + 2)", "[interpreter]") {
    vector<string> contents{"print(1 + 2)"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Statements parse_tree = parser.parse();

    stringstream buffer;
	streambuf *old = cout.rdbuf(buffer.rdbuf());

    Stack stack;
    PyObject res = interpret(parse_tree, stack);

    string stdout_res = buffer.str();
	cout.rdbuf(old);

    REQUIRE( res == PyObject() );
    REQUIRE( stdout_res == "3\n" );
}

TEST_CASE("Interpreter Test - print(1 - 2 + 1.0)", "[interpreter]") {
    vector<string> contents{"print(1 - 2 + 1.0)"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Statements parse_tree = parser.parse();

    stringstream buffer;
	streambuf *old = cout.rdbuf(buffer.rdbuf());

    Stack stack;
    PyObject res = interpret(parse_tree, stack);

    string stdout_res = buffer.str();
	cout.rdbuf(old);

    REQUIRE( res == PyObject() );
    REQUIRE( stdout_res == "0.0\n" );
}
