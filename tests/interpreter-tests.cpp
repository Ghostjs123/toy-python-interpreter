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
#include "../src/interpreter.h"
#include "stack.h"


// TODO: write many, many, test cases
// NOTE: this will break when I change interpret

TEST_CASE("Interpreter Test - 1 + 2", "[interpreter]") {
    vector<string> contents{"1 + 2"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Expression parse_tree = parser.parse();
    Stack stack;
    PyObject res = interpret(parse_tree, stack);
    REQUIRE( res == PyObject(3, "int") );
}

TEST_CASE("Interpreter Test - 1 - 2", "[interpreter]") {
    vector<string> contents{"1 - 2"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Expression parse_tree = parser.parse();
    Stack stack;
    PyObject res = interpret(parse_tree, stack);
    REQUIRE( res == PyObject(-1, "int") );
}

TEST_CASE("Interpreter Test - 1 * 2", "[interpreter]") {
    vector<string> contents{"1 * 2"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Expression parse_tree = parser.parse();
    Stack stack;
    PyObject res = interpret(parse_tree, stack);
    REQUIRE( res == PyObject(2, "int") );
}

TEST_CASE("Interpreter Test - 1.0 * 2", "[interpreter]") {
    vector<string> contents{"1.0 * 2"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Expression parse_tree = parser.parse();
    Stack stack;
    PyObject res = interpret(parse_tree, stack);
    REQUIRE( res == PyObject(2.0, "float") );
}

TEST_CASE("Interpreter Test - print(1 + 2)", "[interpreter]") {
    vector<string> contents{"print(1 + 2)"};
	Tokenizer tokenizer(contents);
    Parser parser(&tokenizer);
    Expression parse_tree = parser.parse();

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
    Expression parse_tree = parser.parse();

    stringstream buffer;
	streambuf *old = cout.rdbuf(buffer.rdbuf());

    Stack stack;
    PyObject res = interpret(parse_tree, stack);

    string stdout_res = buffer.str();
	cout.rdbuf(old);

    REQUIRE( res == PyObject() );
    REQUIRE( stdout_res == "0.0\n" );
}
