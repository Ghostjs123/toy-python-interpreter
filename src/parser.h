#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>
#include "tokenizer.h"
#include "ast.h"
using namespace std;

class Parser {
    private:
        Tokenizer *tokenizer;

        void advance_to_start();
    public:
        string mode;

        Parser(Tokenizer *tokenizer, string mode);
        
        AST* parse();
        
        File parse_file();
        Interactive parse_interactive();
};

#endif
