#pragma once
#include "ast.fwd.h"
#include "stack.fwd.h"
#include "pyobject.fwd.h"


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
class AST;
class File;
class Interactive;
class Statements;
class Statement;
class StatementNewline;
class SimpleStmt;
class SmallStmt;
class CompoundStmt;
class Assignment;
class IfStmt;
class ElifStmt;
class ElseBlock;
class WhileStmt;
class ForStmt;
class WithStmt;
class WithItem;
class TryStmt;
class ExceptBlock;
class FinallyBlock;
class ReturnStmt;
class FunctionDef;
class FunctionDefRaw;
class Block;
class StarExpressions;
class StarExpression;
class StarNamedExpressions;
class StarNamedExpression;
class NamedExpression;
class Expressions;
class Expression;
class Disjunction;
class Conjunction;
class Inversion;
class Comparison;
class BitwiseOr;
class BitwiseXor;
class BitwiseAnd;
class ShiftExpr;
class Sum;
class Term;
class Factor;
class Power;
class AwaitPrimary;
class Primary;
class Slices;
class Slice;
class Atom;
class List;
class Tuple;
class Arguments;
class Args;
class Op;
class _String;
class Number;
class Bool;

class AST {
    public:
        string indent;
        Tokenizer *tokenizer;
        vector<AST *> children;
        
        AST();
        AST(Tokenizer *tokenizer, string indent);

        Token peek(string func_name);
        Token lookahead(int amt);
        Token next_token();
        void eat(string func_name);
        void eat_value(string exp_value, string func_name);
        void eat_type(string exp_type, string func_name);
        virtual PyObject evaluate(Stack stack);
        friend ostream& operator<<(ostream& os, const AST& ast);
        virtual ostream& print(ostream& os) const;
};
class File: public AST {
    private:
        void parse();
    public:
        File(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Interactive: public AST {
    private:
        void parse();
    public:
        Interactive(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Statements: public AST {
    private:
        void parse();
    public:
        Statements(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Statement: public AST {
    private:
        void parse();
    public:
        Statement(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StatementNewline: public AST {
    private:
        void parse();
    public:
        StatementNewline(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class SimpleStmt: public AST {
    private:
        void parse();
    public:
        SimpleStmt(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class SmallStmt: public AST {
    private:
        void parse();
    public:
        SmallStmt(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class CompoundStmt: public AST {
    private:
        void parse();
    public:
        CompoundStmt(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Assignment: public AST {
    private:
        void parse();
    public:
        Assignment(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class IfStmt: public AST {
    private:
        void parse();
    public:
        IfStmt(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ElifStmt: public AST {
    private:
        map<NamedExpression*, Block*> _elifs;
        ElseBlock *_else;

        void parse();
    public:
        ElifStmt(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ElseBlock: public AST {
    private:
        void parse();
    public:
        ElseBlock(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class WhileStmt: public AST {
    private:
        void parse();
    public:
        WhileStmt(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ForStmt: public AST {
    private:
        void parse();
    public:
        ForStmt(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class WithStmt: public AST {
    private:
        void parse();
    public:
        WithStmt(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class WithItem: public AST {
    private:
        void parse();
    public:
        WithItem(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class TryStmt: public AST {
    private:
        void parse();
    public:
        TryStmt(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ExceptBlock: public AST {
    private:
        void parse();
    public:
        ExceptBlock(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class FinallyBlock: public AST {
    private:
        void parse();
    public:
        FinallyBlock(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ReturnStmt: public AST {
    private:
        PyObject return_value;

        void parse();
    public:
        ReturnStmt(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class FunctionDef: public AST {
    private:
        void parse();
    public:
        FunctionDefRaw* raw;

        FunctionDef(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class FunctionDefRaw: public AST {
    private:
        void parse();
    public:
        string name;
        vector<AST*> params;
        Block* body;

        FunctionDefRaw(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Block: public AST {
    private:
        bool has_newline;
        void parse();
    public:
        Block(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StarExpressions: public AST {
    private:
        void parse();
    public:
        StarExpressions(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StarExpression: public AST {
    private:
        void parse();
    public:
        StarExpression(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StarNamedExpressions: public AST {
    private:
        void parse();
    public:
        StarNamedExpressions(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class StarNamedExpression: public AST {
    private:
        void parse();
    public:
        StarNamedExpression(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class NamedExpression: public AST {
    private:
        void parse();
    public:
        NamedExpression(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Expressions: public AST {
    private:
        void parse();
    public:
        Expressions(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Expression: public AST {
    private:
        void parse();
    public:
        Expression(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Disjunction: public AST {
    private:
        void parse();
    public:
        Disjunction(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Conjunction: public AST {
    private:
        void parse();
    public:
        Conjunction(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Inversion: public AST {
    private:
        void parse();
    public:
        Inversion(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Comparison: public AST {
    private:
        void parse();
    public:
        Comparison(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class BitwiseOr: public AST {
    private:
        void parse();
    public:
        BitwiseOr(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class BitwiseXor: public AST {
    private:
        void parse();
    public:
        BitwiseXor(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class BitwiseAnd: public AST {
    private:
        void parse();
    public:
        BitwiseAnd(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class ShiftExpr: public AST {
    private:
        void parse();
    public:
        ShiftExpr(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Sum: public AST {
    private:
        void parse();
    public:
        Sum(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Term: public AST {
    private:
        void parse();
    public:
        Term(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Factor: public AST {
    private:
        void parse();
    public:
        Factor(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Power: public AST {
    private:
        void parse();
    public:
        Power(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class AwaitPrimary: public AST {
    private:
        void parse();
    public:
        AwaitPrimary(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Primary: public AST {
    private:
        void parse();
    public:
        Primary(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Slices: public AST {
    private:
        void parse();
    public:
        Slices(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Slice: public AST {
    private:
        void parse();
    public:
        Slice(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Atom: public AST {
    private:
        void parse();
    public:
        Atom(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class List: public AST {
    private:
        void parse();
    public:
        List(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Tuple: public AST {
    private:
        void parse();
    public:
        Tuple(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Arguments: public AST {
    private:
        void parse();
    public:
        Arguments(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Args: public AST {
    private:
        void parse();
    public:
        Args(Tokenizer *tokenizer, string indent);
        
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class Op: public AST {
    public:
        Token token;

        Op(Tokenizer *tokenizer, string indent);
          
        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};
class _String: public AST {
    private:
        string value;

        void parse();
    public:
        Token token;
        
        _String(Token token);
        _String(Tokenizer *tokenizer, string indent);

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
        Number(Tokenizer *tokenizer, string indent);

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
        Bool(Tokenizer *tokenizer, string indent);

        PyObject evaluate(Stack stack);
        virtual ostream& print(ostream& os) const override;
};

#endif
