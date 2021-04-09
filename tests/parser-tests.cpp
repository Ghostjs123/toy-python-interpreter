#define CATCH_CONFIG_MAIN
#include "../lib/catch.hpp"

#include <string>
#include <vector>
#include "tokenizer.h"
#include "../src/parser.h"
#include "util.h"
#include "ast.h"

string ast_value(AST ast) {
    ostringstream stream;
    stream = move(stream);
	stream << ast;
	return stream.str();
}

TEST_CASE("Parser Test - 1 + 2", "[parser]") {
    vector<string> contents{"1 + 2"};
	Tokenizer tokenizer(contents);

    Parser parser(&tokenizer);
    AST parse_tree = parser.parse();
    string res = ast_value(parse_tree);
    REQUIRE( res == "1+2" );
    REQUIRE( tokenizer.next_token() == tokenizer.end_token );
}

TEST_CASE("Parser Test - 1 + 2 == 4", "[parser]") {
    vector<string> contents{"1 + 2 == 4"};
	Tokenizer tokenizer(contents);

    Parser parser(&tokenizer);
    AST parse_tree = parser.parse();
    string res = ast_value(parse_tree);
    REQUIRE( res == "1+2==4" );
    REQUIRE( tokenizer.next_token() == tokenizer.end_token );
}

TEST_CASE("Parser Test - 5 // 5 == 0", "[parser]") {
    vector<string> contents{"5 // 5 == 0"};
	Tokenizer tokenizer(contents);

    Parser parser(&tokenizer);
    AST parse_tree = parser.parse();
    string res = ast_value(parse_tree);
    REQUIRE( res == "5//5==0" );
    REQUIRE( tokenizer.next_token() == tokenizer.end_token );
}

TEST_CASE("Parser Test - print(1 + 2)", "[parser]") {
    vector<string> contents{"print(1 + 2)"};
	Tokenizer tokenizer(contents);

    Parser parser(&tokenizer);
    AST parse_tree = parser.parse();
    string res = ast_value(parse_tree);
    REQUIRE( res == "print(1+2)" );
    REQUIRE( tokenizer.next_token() == tokenizer.end_token );
}

TEST_CASE("Parser Test - print(\"1 + 2\")", "[parser]") {
    vector<string> contents{"print(\"1 + 2\")"};
	Tokenizer tokenizer(contents);

    Parser parser(&tokenizer);
    AST parse_tree = parser.parse();
    string res = ast_value(parse_tree);
    REQUIRE( res == "print(\"1 + 2\")" );
    REQUIRE( tokenizer.next_token() == tokenizer.end_token );
}
