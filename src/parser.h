#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "tokenizer.h"
#include "ast.h"
using namespace std;

class Parser {
    private:
        Tokenizer* tokenizer;
    public:
        Parser(Tokenizer *tokenizer);
        
        AST* parse(string mode);
};

#endif
