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

AST* Parser::parse() {
    tokenizer->begin();
	advance_to_start();
	if (mode == "file") {
		return new File(tokenizer);
	}
	else if (mode == "interactive") {
		return new Interactive(tokenizer);
	}
	throw runtime_error("unsupported mode '" + mode + "' for parser");
}
