#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <vector>
#include <string>
#include "tokenizer.h"
#include "token.h"
#include "util.h"

TEST_CASE("Tokenizer Tests - test_tokenizer_input", "[tokenizer]") {
    string fname = "tests/test_tokenizer_input";

    vector<string> contents = read_lines(fname);
	Tokenizer tokenizer(contents);

	vector<int> line_nums;
    vector<string> values;
    Token token;
	while ((token = tokenizer.next_token()) != tokenizer.end_token) {
   		line_nums.push_back(token.line_num);
        values.push_back(token.value);
    }
    vector<int> expected_line_nums{
        2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
        4, 4, 4,
        5,
        8, 8,
        9,
        10, 10,
        11,
        12,
        13,
        14, 14, 14, 14,
        16, 16, 16, 16, 16,
        17, 17, 17, 17, 17,
        18, 18, 18, 18, 18,
        19, 19, 19,
        21
    };
    vector<string> expected_values{
        "This", "This", "is", "||", "the",
        "input", "file", "(", ")", ";", "&&", ",", "asd", "(", ")", ";", "asd",
        "123\t\t\t456", "\"asd\"", ";",
        "\"this\tshould ;(): not be split\\n\"",
        "while", ":",
        "\tsingletab",
        "\twhile", ":",
        "\t\tdoubletab",
        "\tsingletab",
        "invalid\ttab",
        "print", "(", "\"hello world\"", ")",
        "1", "+", "1", "==", "2",
        "3", "*", "3", "==", "9",
        "5", "//", "5", "==", "1",
        "x", "=", "2",
        "\"this is the startthis is the middlethis is the end\""
    };
    REQUIRE( line_nums.size() == values.size() );
    REQUIRE( line_nums == expected_line_nums );
    REQUIRE( values == expected_values );
}

TEST_CASE("Tokenizer Tests - compound()", "[tokenizer]") {
    vector<string> contents{"1 is not 2"};
	Tokenizer tokenizer(contents);

    REQUIRE( tokenizer.next_token() == Token(1, "1") );
    tokenizer.compound(2, " "); // should combine "is" and "not" into one Token
    REQUIRE( tokenizer.next_token() == Token(1, "is not") );
    REQUIRE( tokenizer.next_token() == Token(1, "2") );
    REQUIRE( tokenizer.next_token() == tokenizer.end_token );
}

TEST_CASE("Tokenizer Tests - multiline string", "[tokenizer]") {
    vector<string> contents{"\"this is the start", "this is the middle", "this is the end\" asd"};
    Tokenizer tokenizer(contents);

    REQUIRE( tokenizer.next_token() == Token(1, "\"this is the startthis is the middlethis is the end\"") );
    REQUIRE( tokenizer.next_token() == Token(3, "asd") );
    REQUIRE( tokenizer.next_token() == tokenizer.end_token );
}
