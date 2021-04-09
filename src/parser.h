#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "tokenizer.h"
#include "parser.h"
#include "ast.h"
using namespace std;

class Parser {
    private:
        Tokenizer *tokenizer;
        string mode;

        void advance_to_start();
    public:
        Parser(Tokenizer *tokenizer, string mode);

        Statements parse_statements();
        Interactive parse_interactive();
};

#endif
