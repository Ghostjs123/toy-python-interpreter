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

Parser::Parser(Tokenizer *tokenizer, string mode) {
    this->tokenizer = tokenizer;
	this->mode = mode;
}

void Parser::advance_to_start() {
	// skips empty lines
	while ((tokenizer->peek().type == "NL" || tokenizer->peek().type == "NEWLINE")
			&& tokenizer->peek().type != "ENDMARKER") {
		tokenizer->next_token();
	}
}

Interactive Parser::parse_interactive() {
    tokenizer->begin();
	advance_to_start();
	return Interactive(tokenizer);
}

Statements Parser::parse_statements() {
    tokenizer->begin();
	advance_to_start();
	return Statements(tokenizer);
}
