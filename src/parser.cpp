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

Parser::Parser(Tokenizer* tokenizer) {
    this->tokenizer = tokenizer;
}

AST* Parser::parse(string mode) {
    tokenizer->begin();
	if (mode == "file") {
		return new File(tokenizer, "");
	}
	else if (mode == "interactive") {
		return new Interactive(tokenizer, "");
	}
	throw runtime_error("unsupported mode '" + mode + "' for parser");
}
