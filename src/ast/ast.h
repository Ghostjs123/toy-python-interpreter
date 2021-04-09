#pragma once
#include "ast.fwd.h"
#include "stack.fwd.h"


#ifndef AST_H
#define AST_H

// https://docs.python.org/3/reference/grammar.html
#include <iostream>
#include <vector>
#include <string>
#include "ast_helpers.h"
#include "tokenizer.h"
#include "token.h"
#include "pyobject.h"
#include "stack.h"
using namespace std;

// forward declares bc these need to know about eachother
class NamedExpression;
class Block;
class IfStmt;
class ElifStmt;
class ElseBlock;
    
class AST {
    public:
        Tokenizer *tokenizer;
        vector<AST *> children;
        
        AST();
        AST(Tokenizer *tokenizer);

        Token peek();
        Token lookahead(int amt);
        Token next_token();
        void eat(string exp_value);
        void eat(string exp_type, string exp_value);
        virtual PyObject evaluate(Stack stack);
        friend ostream& operator<<(ostream& os, const AST& ast);
        virtual ostream& print(ostream& os) const;
};
class File: public AST {
    private:
        void parse();
    public:
        File(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Interactive: public AST {
    private:
        void parse();
    public:
        Interactive(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Statements: public AST {
    private:
        void parse();
    public:
        Statements(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Statement: public AST {
    private:
        void parse();
    public:
        Statement(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StatementNewline: public AST {
    private:
        void parse();
    public:
        StatementNewline(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class SimpleStmt: public AST {
    private:
        void parse();
    public:
        SimpleStmt(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class SmallStmt: public AST {
    private:
        void parse();
    public:
        SmallStmt(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class CompoundStmt: public AST {
    private:
        void parse();
    public:
        CompoundStmt(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Assignment: public AST {
    private:
        void parse();
    public:
        Assignment(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class IfStmt: public AST {
    private:
        void parse();
    public:
        IfStmt(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ElifStmt: public AST {
    private:
        map<NamedExpression*, Block*> _elifs;
        ElseBlock *_else;

        void parse();
    public:
        ElifStmt(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ElseBlock: public AST {
    private:
        void parse();
    public:
        ElseBlock(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class WhileStmt: public AST {
    private:
        void parse();
    public:
        WhileStmt(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ForStmt: public AST {
    private:
        void parse();
    public:
        ForStmt(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class WithStmt: public AST {
    private:
        void parse();
    public:
        WithStmt(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class WithItem: public AST {
    private:
        void parse();
    public:
        WithItem(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class TryStmt: public AST {
    private:
        void parse();
    public:
        TryStmt(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ExceptBlock: public AST {
    private:
        void parse();
    public:
        ExceptBlock(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class FinallyBlock: public AST {
    private:
        void parse();
    public:
        FinallyBlock(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ReturnStmt: public AST {
    private:
        void parse();
    public:
        ReturnStmt(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Block: public AST {
    private:
        void parse();
    public:
        Block(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StarExpressions: public AST {
    private:
        void parse();
    public:
        StarExpressions(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StarExpression: public AST {
    private:
        void parse();
    public:
        StarExpression(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StarNamedExpressions: public AST {
    private:
        void parse();
    public:
        StarNamedExpressions(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StarNamedExpression: public AST {
    private:
        void parse();
    public:
        StarNamedExpression(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class NamedExpression: public AST {
    private:
        void parse();
    public:
        NamedExpression(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Expressions: public AST {
    private:
        void parse();
    public:
        Expressions(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Expression: public AST {
    private:
        void parse();
    public:
        Expression(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Disjunction: public AST {
    private:
        void parse();
    public:
        Disjunction(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Conjunction: public AST {
    private:
        void parse();
    public:
        Conjunction(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Inversion: public AST {
    private:
        void parse();
    public:
        Inversion(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Comparison: public AST {
    private:
        void parse();
    public:
        Comparison(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class BitwiseOr: public AST {
    private:
        void parse();
    public:
        BitwiseOr(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class BitwiseXor: public AST {
    private:
        void parse();
    public:
        BitwiseXor(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class BitwiseAnd: public AST {
    private:
        void parse();
    public:
        BitwiseAnd(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ShiftExpr: public AST {
    private:
        void parse();
    public:
        ShiftExpr(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Sum: public AST {
    private:
        void parse();
    public:
        Sum(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Term: public AST {
    private:
        void parse();
    public:
        Term(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Factor: public AST {
    private:
        void parse();
    public:
        Factor(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Power: public AST {
    private:
        void parse();
    public:
        Power(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class AwaitPrimary: public AST {
    private:
        void parse();
    public:
        AwaitPrimary(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Primary: public AST {
    private:
        void parse();
    public:
        Primary(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};

class Slices: public AST {
    private:
        void parse();
    public:
        Slices(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Slice: public AST {
    private:
        void parse();
    public:
        Slice(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Atom: public AST {
    private:
        void parse();
    public:
        Atom(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class List: public AST {
    private:
        void parse();
    public:
        List(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Tuple: public AST {
    private:
        void parse();
    public:
        Tuple(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Arguments: public AST {
    private:
        void parse();
    public:
        Arguments(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Args: public AST {
    private:
        void parse();
    public:
        Args(Tokenizer *tokenizer);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Op: public AST {
    public:
        Token token;

        Op(Tokenizer *tokenizer);
          
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class String: public AST {
    private:
        string value;

        void parse();
    public:
        Token token;
        
        String(Token token);
        String(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Number: public AST {
    private:
        bool is_int;

        void parse();
    public:
        Token token;

        Number(Token token);
        Number(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Bool: public AST {
    private:
        bool bool_value;
        
        void parse();
    public:
        Token token;

        Bool(Token token);
        Bool(Tokenizer *tokenizer);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};

#endif
