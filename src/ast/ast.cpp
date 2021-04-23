// https://docs.python.org/3/reference/grammar.html
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <numeric>
#include <typeinfo>
#include "logging.h"
#include "tokenizer.h"
#include "token.h"
#include "util.h"
#include "ast.h"
#include "ast_helpers.h"
#include "pyobject.h"
#include "stack.h"
using namespace std;

void log(string msg, int mode) {
    Logger::get_instance()->log(msg, mode);
}
void add_indent(int amt) {
    Logger::get_instance()->add_indent(amt);
}
void sub_indent(int amt) {
    Logger::get_instance()->sub_indent(amt);
}

//===============================================================
// AST: parent class of all nodes

AST::AST() { }
AST::AST(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
}
Token AST::peek(string func_name) {
    try {
        tokenizer->find_next();
        return tokenizer->peek();
    } catch (exception& e) {
        cout << func_name << ": " << e.what() << endl;
    }
    throw runtime_error("halting after error in peek()");
}
Token AST::lookahead(int amt) { 
    return tokenizer->lookahead(amt);
}
Token AST::next_token() {
    tokenizer->find_next();
    Token t = tokenizer->next_token();
    log(": " + t.as_string(), DEBUG);
    return t;
}
void AST::eat(string func_name) {
    Token next = next_token();
}
void AST::eat_value(string exp_value, string func_name) {
    Token next = next_token();
    if (next.value != exp_value) {
        throw runtime_error("ate '" + next.value + "' expected '" 
                            + exp_value + "' in '" + func_name + "'");
    }
}
void AST::eat_type(string exp_type, string func_name) {
    Token next = next_token();
    if (next.type != exp_type) {
        throw runtime_error("ate '" + next.type + "' expected '" 
                            + exp_type + "' in '" + func_name + "'");
    }
}
PyObject AST::evaluate(Stack stack) {
    throw runtime_error("Attempted to evaluate an AST - start evaluation at a subclass");
}
ostream& operator<<(ostream& os, const AST& ast) {
    return ast.print(os);
}
ostream& AST::print(ostream& os) const {
    if (children.size() == 0) {
        os << "Empty AST Node";
    } else {
        for (AST *child : children) {
            os << *child;
        }
    }
    return os;
}

//===============================================================
// File: starting non-terminal for file mode

// file: [statements] ENDMARKER 
File::File(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void File::parse() {
    while (peek("File").type != "ENDMARKER") {
        children.push_back(new Statements(tokenizer));
    }
    eat_type("ENDMARKER", "File");
}
PyObject File::evaluate(Stack stack) {
    log("File::evaluate()", DEBUG); add_indent(2);
    for (AST *child : children) {
        child->evaluate(stack);
    }
    sub_indent(2);
    return PyObject();
}
ostream& File::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Interactive: starting non-terminal for interactive mode

// interactive: statement_newline
Interactive::Interactive(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Interactive::parse() {
    children.push_back(new StatementNewline(tokenizer));
}
PyObject Interactive::evaluate(Stack stack) {
    log("Interactive::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& Interactive::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// Statements

// statements: statement+ 
Statements::Statements(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Statements::parse() {
    Statement* temp = new Statement(tokenizer);
    if (temp->children.size() == 0) {
        delete temp;
    } else {
        children.push_back(temp);
    }
}
PyObject Statements::evaluate(Stack stack) {
    log("Statements::evaluate()", DEBUG); add_indent(2);
    if (children.size() > 0) {
        sub_indent(2);
        return children.at(0)->evaluate(stack);
    }
    sub_indent(2);
    return PyObject();
}
ostream& Statements::print(ostream& os) const {
    if (children.size() > 0) os << *children.at(0);
    return os;
}

//===============================================================
// Statement

// NOTE: this attempts to parse a CompoundStmt, falls back to SimpleStmt

// statement: compound_stmt  | simple_stmt
Statement::Statement(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Statement::parse() {
    CompoundStmt *temp = new CompoundStmt(tokenizer);
    if (temp->children.size() == 0) {
        delete temp;
        children.push_back(new SimpleStmt(tokenizer));
    } else {
        children.push_back(temp);
    }
}
PyObject Statement::evaluate(Stack stack) {
    log("Statement::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& Statement::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// StatementNewline

    // NOTE: this node is only used for interactive mode
    // I am opting to ignore a lot of the whitespace tokens in favor
    // of handling them in the terminal (ncurses) stuff instead

// statement_newline:
//     | compound_stmt NEWLINE 
//     | simple_stmt
//     | NEWLINE 
//     | ENDMARKER
StatementNewline::StatementNewline(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void StatementNewline::parse() {
    if (peek("StatementNewline").type == "NEWLINE") {
        eat_type("NEWLINE", "StatementNewline");
    } else if (peek("StatementNewline").type == "NL") {
        eat_type("NL", "StatementNewline");
    } else if (peek("StatementNewline").type == "ENDMARKER") {
        eat_type("ENDMARKER", "StatementNewline");
    } else {
        CompoundStmt *temp = new CompoundStmt(tokenizer);
        if (temp->children.size() == 0) {
            delete temp;
            children.push_back(new SimpleStmt(tokenizer));
        } else {
            children.push_back(temp);
            eat_type("NEWLINE", "StatementNewline");
        }
    }
}
PyObject StatementNewline::evaluate(Stack stack) {
    log("StatementNewline::evaluate()", DEBUG); add_indent(2);
    if (children.size() > 0) {
        sub_indent(2);
        return children.at(0)->evaluate(stack);
    }
    sub_indent(2);
    return PyObject();
}
ostream& StatementNewline::print(ostream& os) const {
    if (children.size() > 0) os << *children.at(0);
    return os;
}

//===============================================================
// SimpleStmt

// simple_stmt:
//     | small_stmt !';' NEWLINE  # Not needed, there for speedup
//     | ';'.small_stmt+ [';'] NEWLINE
SimpleStmt::SimpleStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void SimpleStmt::parse() {
    // TODO: grammar needs to support ;
    children.push_back(new SmallStmt(tokenizer));
    eat_type("NEWLINE", "SimpleStmt");
}
PyObject SimpleStmt::evaluate(Stack stack) {
    log("SimpleStmt::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& SimpleStmt::print(ostream& os) const {
    // NOTE: these should always have a NEWLINE
    os << *children.at(0) << endl;
    return os;
}

//===============================================================
// SmallStmt

// NOTE: star_expressions leads to bitwise stuff (only way to get to Atom)

// small_stmt:
//     | assignment
//     | star_expressions 
//     | return_stmt
//     | import_stmt
//     | raise_stmt
//     | 'pass' 
//     | del_stmt
//     | yield_stmt
//     | assert_stmt
//     | 'break' 
//     | 'continue' 
//     | global_stmt
//     | nonlocal_stmt
SmallStmt::SmallStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void SmallStmt::parse() {
    if (peek("SmallStmt").value == "return") {
        children.push_back(new ReturnStmt(tokenizer));
    }
    else if (peek("SmallStmt").value == "import") {
        throw runtime_error("SmallStmt: 'import' not implemented");
    }
    else if (peek("SmallStmt").value == "raise") {
        throw runtime_error("SmallStmt: 'raise' not implemented");
    }
    else if (peek("SmallStmt").value == "pass") {
        throw runtime_error("SmallStmt: 'pass' not implemented");
    }
    else if (peek("SmallStmt").value == "del") {
        throw runtime_error("SmallStmt: 'del' not implemented");
    }
    else if (peek("SmallStmt").value == "yield") {
        throw runtime_error("SmallStmt: 'yield' not implemented");
    }
    else if (peek("SmallStmt").value == "assert") {
        throw runtime_error("SmallStmt: 'assert' not implemented");
    }
    else if (peek("SmallStmt").value == "break") {
        throw runtime_error("SmallStmt: 'break' not implemented");
    }
    else if (peek("SmallStmt").value == "continue") {
        throw runtime_error("SmallStmt: 'continue' not implemented");
    }
    else if (peek("SmallStmt").value == "global") {
        throw runtime_error("SmallStmt: 'global' not implemented");
    }
    else if (peek("SmallStmt").value == "nonlocal") {
        throw runtime_error("SmallStmt: 'nonlocal' not implemented");
    }
    else {
        // assignment
        // star_expressions
        children.push_back(new StarExpressions(tokenizer));
    }
}
PyObject SmallStmt::evaluate(Stack stack) {
    log("SmallStmt::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& SmallStmt::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// CompoundStmt

// compound_stmt:
//     | function_def
//     | if_stmt
//     | class_def
//     | with_stmt
//     | for_stmt
//     | try_stmt
//     | while_stmt
CompoundStmt::CompoundStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void CompoundStmt::parse() {
    if (peek("CompoundStmt").value == "if") {
        children.push_back(new IfStmt(tokenizer));
    }
    else if (peek("CompoundStmt").value == "for") {
        children.push_back(new ForStmt(tokenizer));
    }
    else if (peek("CompoundStmt").value == "while") {
        children.push_back(new WhileStmt(tokenizer));
    }
    else if (peek("CompoundStmt").value == "def") {
        children.push_back(new FunctionDef(tokenizer));
    }
    // TODO: class_def, with_stmt, try_stmt
}
PyObject CompoundStmt::evaluate(Stack stack) {
    log("CompoundStmt::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& CompoundStmt::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// Assignment

// assignment:
//     | NAME ':' expression ['=' annotated_rhs ] 
//     | ('(' single_target ')' 
//          | single_subscript_attribute_target) ':' expression ['=' annotated_rhs ] 
//     | (star_targets '=' )+ (yield_expr | star_expressions) !'=' [TYPE_COMMENT] 
//     | single_target augassign ~ (yield_expr | star_expressions) 
// augassign:
//     | '+=' 
//     | '-=' 
//     | '*=' 
//     | '@=' 
//     | '/=' 
//     | '%=' 
//     | '&=' 
//     | '|=' 
//     | '^=' 
//     | '<<=' 
//     | '>>=' 
//     | '**=' 
//     | '//=' 
Assignment::Assignment(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Assignment::parse() {
    // TODO:
}
PyObject Assignment::evaluate(Stack stack) {
    // TODO:
    log("Assignment::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    return PyObject(); // returns None
}
ostream& Assignment::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}


//===============================================================
// IfStmt

// if_stmt:
//     | 'if' named_expression ':' block elif_stmt 
//     | 'if' named_expression ':' block [else_block] 
IfStmt::IfStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void IfStmt::parse() {
    eat_value("if", "IfStmt");
    children.push_back(new NamedExpression(tokenizer));
    eat_value(":", "IfStmt");
    children.push_back(new Block(tokenizer));
    // NOTE: if its an if-elif-else, 
    //  the else block should end up in the ElifStmt production
    if (peek("IfStmt").value == "elif") {
        children.push_back(new ElifStmt(tokenizer));
    }
    else if (peek("IfStmt").value == "else") {
        children.push_back(new ElseBlock(tokenizer));
    }
}
PyObject IfStmt::evaluate(Stack stack) {
    log("IfStmt::evaluate()", DEBUG); add_indent(2);
    PyObject ret;
    if (children.at(0)->evaluate(stack)) {
        // if statement is true
        ret = children.at(1)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    else if (children.size() == 3) {
        // this will either be the elif_stmt or [else_block]
        ret = children.at(2)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    // if statement was false and no elif_stmt or [else_block]
    sub_indent(2);
    return PyObject();  // returns None
}
ostream& IfStmt::print(ostream& os) const {
    os << "if " << *(children.at(0)) << ":";
    for (int i=1; i < children.size(); i++) {
        os << *(children.at(i));
    }
    return os;
}

//===============================================================
// ElifStmt

// elif_stmt:
//     | 'elif' named_expression ':' block elif_stmt 
//     | 'elif' named_expression ':' block [else_block] 
ElifStmt::ElifStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    _else = nullptr;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void ElifStmt::parse() {
    NamedExpression *t1;
    Block *t2;
    while (peek("ElifStmt").value == "elif") {
        eat_value("elif", "ElifStmt");
        t1 = new NamedExpression(tokenizer);
        eat_value(":", "ElifStmt");
        t2 = new Block(tokenizer);
        _elifs[t1] = t2;
    }
    if (peek("ElifStmt").value == "else") {
        _else = new ElseBlock(tokenizer);
    } 
}
PyObject ElifStmt::evaluate(Stack stack) {
    log("ElifStmt::evaluate()", DEBUG); add_indent(2);
    PyObject ret;
    map<NamedExpression*, Block*>::iterator it;
    for (it = _elifs.begin(); it != _elifs.end(); it++) {
        if (it->first->evaluate(stack)) {
            // an elif was true
            ret = it->second->evaluate(stack);
            sub_indent(2);
            return ret;
        }
    }
    if (_else != nullptr) {
        // else_block
        ret = _else->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    // all elifs were false and no else_block
    sub_indent(2);
    return PyObject(); // returns None
}
ostream& ElifStmt::print(ostream& os) const {
    map<NamedExpression*, Block*>::const_iterator it;
    for (it = _elifs.begin(); it != _elifs.end(); it++) {
        os << "elif " << *(it->first) << ":";
        os << *(it->second);
    }
    if (_else != nullptr) {
        os << *_else;
    }
    return os;
}

//===============================================================
// ElseBlock

// else_block: 'else' ':' block 
ElseBlock::ElseBlock(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void ElseBlock::parse() {
    eat_value("else", "ElseBlock");
    eat_value(":", "ElseBlock");
    children.push_back(new Block(tokenizer));
}
PyObject ElseBlock::evaluate(Stack stack) {
    log("ElseBlock::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& ElseBlock::print(ostream& os) const {
    os << "else:" << *children.at(0);
    return os;
}

//===============================================================
// WhileStmt

// while_stmt:
//     | 'while' named_expression ':' block [else_block] 
WhileStmt::WhileStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void WhileStmt::parse() {
    // TODO:
}
PyObject WhileStmt::evaluate(Stack stack) {
    // TODO:
    log("WhileStmt::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    return PyObject();
}
ostream& WhileStmt::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// ForStmt

// NOTE: need to unpack
// for [a,b,c] in ["123","def"]: print(a, b, c)
// 1 2 3
// d e f
// NOTE: not block scoped (z prev undefined)
// >>> for z in range(9): pass
// >>> z
// 8

// for_stmt:
//     | 'for' star_targets 'in' ~ star_expressions ':' [TYPE_COMMENT] block [else_block] 
//     | ASYNC 'for' star_targets 'in' ~ star_expressions ':' [TYPE_COMMENT] block [else_block]
ForStmt::ForStmt(Tokenizer *tokenizer) {
    // Loop better: a deeper look at iteration in Python
    // https://www.youtube.com/watch?v=V2PkkMS2Ack
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void ForStmt::parse() {
    // TODO:
}
PyObject ForStmt::evaluate(Stack stack) {
    // TODO:
    log("ForStmt::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    return PyObject(); // returns None
}
ostream& ForStmt::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// WithStmt

// with_stmt:
//     | 'with' '(' ','.with_item+ ','? ')' ':' block 
//     | 'with' ','.with_item+ ':' [TYPE_COMMENT] block 
//     | ASYNC 'with' '(' ','.with_item+ ','? ')' ':' block 
//     | ASYNC 'with' ','.with_item+ ':' [TYPE_COMMENT] block 
WithStmt::WithStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void WithStmt::parse() {
    // TODO:
}
PyObject WithStmt::evaluate(Stack stack) {
    // TODO:
    log("WithStmt::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    return PyObject();
}
ostream& WithStmt::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// WithItem

// with_item:
//     | expression 'as' star_target &(',' | ')' | ':') 
//     | expression
WithItem::WithItem(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void WithItem::parse() {
    // TODO:
}
PyObject WithItem::evaluate(Stack stack) {
    // TODO:
    log("WithItem::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    return PyObject();
}
ostream& WithItem::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// TryStmt

TryStmt::TryStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void TryStmt::parse() {
    // TODO:
}
PyObject TryStmt::evaluate(Stack stack) {
    // TODO:
    log("TryStmt::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    return PyObject();
}
ostream& TryStmt::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// ExceptBlock

// except_block:
//     | 'except' expression ['as' NAME ] ':' block 
//     | 'except' ':' block 
ExceptBlock::ExceptBlock(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void ExceptBlock::parse() {
    // TODO:
}
PyObject ExceptBlock::evaluate(Stack stack) {
    // TODO:
    log("ExceptBlock::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    return PyObject();
}
ostream& ExceptBlock::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// FinallyBlock

// finally_block: 'finally' ':' block 
FinallyBlock::FinallyBlock(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void FinallyBlock::parse() {
    // TODO:
}
PyObject FinallyBlock::evaluate(Stack stack) {
    // TODO:
    log("FinallyBlock::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    return PyObject();
}
ostream& FinallyBlock::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// ReturnStmt

// return_stmt:
//     | 'return' [star_expressions]
ReturnStmt::ReturnStmt(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void ReturnStmt::parse() {
    eat_value("return", "ReturnStmt");
    children.push_back(new StarExpressions(tokenizer));
}
PyObject ReturnStmt::evaluate(Stack stack) {
    log("ReturnStmt::evaluate()", DEBUG); add_indent(2);
    PyObject temp = children.at(0)->evaluate(stack);
    this->return_value = temp;
    stack.set_return_value(this->return_value);
    sub_indent(2);
    return PyObject();  // this value doesnt matter
}
ostream& ReturnStmt::print(ostream& os) const {
    os << "return " << *children.at(0);
    return os;
}

//===============================================================
// FunctionDef

// function_def:
//     | decorators function_def_raw 
//     | function_def_raw
FunctionDef::FunctionDef(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void FunctionDef::parse() {
    this->raw = new FunctionDefRaw(tokenizer);
}
PyObject FunctionDef::evaluate(Stack stack) {
    log("FunctionDef::evaluate()", DEBUG); add_indent(2);
    // add function definition to stack/frame
    stack.add_function(this);
    // function definition shouldnt return anything
    sub_indent(2);
    return PyObject();
}
ostream& FunctionDef::print(ostream& os) const {
    os << *this->raw;
    return os;
}

//===============================================================
// FunctionDefRaw

// function_def_raw:
//     | 'def' NAME '(' [params] ')' ['->' expression ] ':' [func_type_comment] block 
//     | ASYNC 'def' NAME '(' [params] ')' ['->' expression ] ':' [func_type_comment] block 
FunctionDefRaw::FunctionDefRaw(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void FunctionDefRaw::parse() {
    eat_value("def", "FunctionDefRaw");
    this->name = next_token().value;
    eat_value("(", "FunctionDefRaw");
    vector<AST*> params;
    while (peek("FunctionDefRaw").value != ")") {
        params.push_back(new _String(tokenizer));
    }
    this->params = params;
    eat_value(")", "FunctionDefRaw");
    eat_value(":", "FunctionDefRaw");
    this->body = new Block(tokenizer);
}
PyObject FunctionDefRaw::evaluate(Stack stack) {
    log("FunctionDefRaw::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    // function definition shouldnt return anything
    return PyObject();
}
ostream& FunctionDefRaw::print(ostream& os) const {
    os << "def " << name << "(";
    if (params.size() > 1) {
        for (int i=0; i < params.size()-1; i++) {
            os << *params.at(i) << ", ";
        }
        os << *params.back();
    }
    else if (params.size() == 1) os << *params.back();
    os << "):" << *body;
    return os;
}


//===============================================================
// Block

// NOTE: I am intentionally not calling Statements here, using Statement instead

// block:
//     | NEWLINE INDENT statements DEDENT 
//     | simple_stmt
Block::Block(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Block::parse() {
    // the NEWLINE can be optional, for instance one liner function like
    // def a(): return 1
    // if this happens it will also not have an INDENT or DEDENT
    // consequently it will only have 1 Statement
    if (peek("Block").type == "NEWLINE") {
        eat_type("NEWLINE", "Block");
        this->indent = peek("Block").value;
        eat_type("INDENT", "Block");
        while (peek("Block").type != "DEDENT") {
            children.push_back(new Statement(tokenizer));
        }
        eat_type("DEDENT", "Block");
    }
    else {
        this->indent = "";
        children.push_back(new Statement(tokenizer));
    }
}
PyObject Block::evaluate(Stack stack) {
    log("Block::evaluate()", DEBUG); add_indent(2);
    for (AST* child : children) {
        child->evaluate(stack);
        if (stack.is_returning()) {
            sub_indent(2);
            return PyObject();
        }
    }
    sub_indent(2);
    return PyObject();  // Blocks dont return anything
}
ostream& Block::print(ostream& os) const {
    for (AST *child : children) {
        os << endl << indent << *child;
    }
    return os;
}

//===============================================================
// StarExpressions

// NOTE: basically identical to Expressions

// star_expressions:
//     | star_expression (',' star_expression )+ [','] 
//     | star_expression ',' 
//     | star_expression
StarExpressions::StarExpressions(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void StarExpressions::parse() {
    children.push_back(new StarExpression(tokenizer));
    while(peek("StarExpressions").value == ",") {
        children.push_back(new StarExpression(tokenizer));
    }
}
PyObject StarExpressions::evaluate(Stack stack) {
    log("StarExpressions::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    sub_indent(2);
    return PyObject(results, "tuple");
}
ostream& StarExpressions::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// StarExpression

// star_expression:
//     | '*' bitwise_or 
//     | expression
StarExpression::StarExpression(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void StarExpression::parse() {
    if (peek("StarExpression").value == "*") {
        children.push_back(new _String(tokenizer));
        children.push_back(new BitwiseOr(tokenizer));
    }
    else {
        children.push_back(new Expression(tokenizer));
    }
}
PyObject StarExpression::evaluate(Stack stack) {
    log("StarExpression::evaluate()", DEBUG); add_indent(2);
    // TODO: figure out how the * grammar works in practice
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& StarExpression::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// StarNamedExpressions

// star_named_expressions: ','.star_named_expression+ [','] 
StarNamedExpressions::StarNamedExpressions(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void StarNamedExpressions::parse() {
    children.push_back(new StarNamedExpression(tokenizer));
    while (peek("StarNamedExpressions").value == ",") {
        eat_value(",", "StarNamedExpressions");
        children.push_back(new StarNamedExpression(tokenizer));
    }
}
PyObject StarNamedExpressions::evaluate(Stack stack) {
    log("StarNamedExpressions::evaluate()", DEBUG); add_indent(2);
    // NOTE: this always needs to return an iterable
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    sub_indent(2);
    return PyObject(results, "tuple");
}
ostream& StarNamedExpressions::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// StarNamedExpression

// star_named_expression:
//     | '*' bitwise_or 
//     | named_expression
StarNamedExpression::StarNamedExpression(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void StarNamedExpression::parse() {
    log("StarNamedExpression::evaluate()", DEBUG); add_indent(2);
    if (peek("StarNamedExpression").value == "*") {
        children.push_back(new _String(tokenizer));
        children.push_back(new BitwiseOr(tokenizer));
    }
    else {
        children.push_back(new NamedExpression(tokenizer));
    }
}
PyObject StarNamedExpression::evaluate(Stack stack) {
    // TODO: figure out how the star is gonna work
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& StarNamedExpression::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// NamedExpression

// named_expression:
//     | NAME ':=' ~ expression 
//     | expression !':='
NamedExpression::NamedExpression(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void NamedExpression::parse() {
    // TODO: handle ':=' case
    children.push_back(new Expression(tokenizer));
}
PyObject NamedExpression::evaluate(Stack stack) {
    log("NamedExpression::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& NamedExpression::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Expressions
// NOTE: falls back to tuples
// >>> 2+2,1-1
// (4, 0)

// NOTE: basically identical to StarExpressions

// expressions:
//     | expression (',' expression )+ [','] 
//     | expression ',' 
//     | expression
Expressions::Expressions(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Expressions::parse() {
    children.push_back(new Expression(tokenizer));
    while(peek("Expressions").value == ",") {
        children.push_back(new Expression(tokenizer));
    }
}
PyObject Expressions::evaluate(Stack stack) {
    log("Expressions::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    sub_indent(2);
    return PyObject(results, "tuple");
}
ostream& Expressions::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Expression

// expression:
//     | disjunction 'if' disjunction 'else' expression 
//     | disjunction
//     | lambdef
Expression::Expression(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Expression::parse() {
    // NOTE: skipping lambdef
    if (lookahead(1).value == "if") {

    }
    children.push_back(new Disjunction(tokenizer));
}
PyObject Expression::evaluate(Stack stack) {
    log("Expression::evaluate()", DEBUG); add_indent(2);
    // TODO: implement case (1)
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    throw runtime_error("reached end of Expression::evaluate() without returning");
}
ostream& Expression::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Disjunction

// disjunction:
//     | conjunction ('or' conjunction )+ 
//     | conjunction
Disjunction::Disjunction(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Disjunction::parse() {
    children.push_back(new Conjunction(tokenizer));
    while(peek("Disjunction").value == "or") {
        eat_value("or", "Disjunction");
        children.push_back(new Conjunction(tokenizer));
    }
}
PyObject Disjunction::evaluate(Stack stack) {
    log("Disjunction::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<bool> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    bool result = accumulate(
        results.begin(), results.end(), results.at(0), _boolean_or);
    sub_indent(2);
    return PyObject(result, "bool");
}
ostream& Disjunction::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Conjunction

// conjunction:
//     | inversion ('and' inversion )+ 
//     | inversion
Conjunction::Conjunction(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Conjunction::parse() {
    children.push_back(new Inversion(tokenizer));
    while(peek("Conjunction").value == "and") {
        eat_value("and", "Conjunction");
        children.push_back(new Inversion(tokenizer));
    }
}
PyObject Conjunction::evaluate(Stack stack) {
    log("Conjunction::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<bool> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    bool result = accumulate(
        results.begin(), results.end(), results.at(0), _boolean_and);
    sub_indent(2);
    return PyObject(result, "bool");
}
ostream& Conjunction::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Inversion

// inversion:
//     | 'not' inversion 
//     | comparison
Inversion::Inversion(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Inversion::parse() {
    if (peek("Inversion").value == "not") {
        children.push_back(new _String(tokenizer));
    }
    children.push_back(new Comparison(tokenizer));
}
PyObject Inversion::evaluate(Stack stack) {
    log("Inversion::evaluate()", DEBUG); add_indent(2);
    PyObject s = children.at(0)->evaluate(stack);
    if (s.type == "str" && s.as_string() == "not") {
        bool b = children.at(1)->evaluate(stack);
        sub_indent(2);
        return PyObject(!b, "bool");
    }
    sub_indent(2);
    return s;
}
ostream& Inversion::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Comparison

// comparison:
//     | bitwise_or compare_op_bitwise_or_pair+ 
//     | bitwise_or
// compare_op_bitwise_or_pair:
//     | eq_bitwise_or
//     | noteq_bitwise_or
//     | lte_bitwise_or
//     | lt_bitwise_or
//     | gte_bitwise_or
//     | gt_bitwise_or
//     | notin_bitwise_or
//     | in_bitwise_or
//     | isnot_bitwise_or
//     | is_bitwise_or
Comparison::Comparison(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Comparison::parse() {
    children.push_back(new BitwiseOr(tokenizer));
    while (is_comparison_op(tokenizer)) {
        children.push_back(new Op(tokenizer));
        children.push_back(new BitwiseOr(tokenizer));
    }
}
PyObject Comparison::evaluate(Stack stack) {
    log("Comparison::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    while (results.size() > 1) {
        string op = results.at(1);
        PyObject right = results.at(2);
        results.at(0) = apply_comparison_op(results.at(0), op, right);
        results.erase(results.begin()+1, results.begin()+3);
    }
    sub_indent(2);
    return results.front();
}
ostream& Comparison::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// BitwiseOr

// bitwise_or:
//     | bitwise_or '|' bitwise_xor 
//     | bitwise_xor
BitwiseOr::BitwiseOr(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void BitwiseOr::parse() {
    children.push_back(new BitwiseXor(tokenizer));
    while (peek("BitwiseOr").value == "|") {
        eat_value("|", "BitwiseOr");
        children.push_back(new BitwiseXor(tokenizer));
    }
}
PyObject BitwiseOr::evaluate(Stack stack) {
    log("BitwiseOr::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<int> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    int result = accumulate(
        results.begin(), results.end(), results.at(0), _bitwise_or);
    sub_indent(2);
    return PyObject(result, "int");
}
ostream& BitwiseOr::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// BitwiseXor

// bitwise_xor:
//     | bitwise_xor '^' bitwise_and 
//     | bitwise_and
BitwiseXor::BitwiseXor(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void BitwiseXor::parse() {
    children.push_back(new BitwiseAnd(tokenizer));
    while (peek("BitwiseXor").value == "^") {
        eat_value("^", "BitwiseXor");
        children.push_back(new BitwiseAnd(tokenizer));
    }
}
PyObject BitwiseXor::evaluate(Stack stack) {
    log("BitwiseXor::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<int> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    int result = accumulate(
        results.begin(), results.end(), results.at(0), _bitwise_xor);
    sub_indent(2);
    return PyObject(result, "int");
}
ostream& BitwiseXor::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// BitwiseAnd

// bitwise_and:
//     | bitwise_and '&' shift_expr 
//     | shift_expr
BitwiseAnd::BitwiseAnd(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void BitwiseAnd::parse() {
    children.push_back(new ShiftExpr(tokenizer));
    while (peek("BitwiseAnd").value == "&") {
        eat_value("&", "BitwiseAnd");
        children.push_back(new ShiftExpr(tokenizer));
    }
}
PyObject BitwiseAnd::evaluate(Stack stack) {
    log("BitwiseAnd::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<int> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    int result = accumulate(
        results.begin(), results.end(), results.at(0), _bitwise_and);
    sub_indent(2);
    return PyObject(result, "int");
}
ostream& BitwiseAnd::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// ShiftExpr

// shift_expr:
//     | shift_expr '<<' sum 
//     | shift_expr '>>' sum 
//     | sum
ShiftExpr::ShiftExpr(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void ShiftExpr::parse() {
    children.push_back(new Sum(tokenizer));
    while (peek("ShiftExpr").value == "<<" || peek("ShiftExpr").value == ">>") {
        children.push_back(new Op(tokenizer));
        children.push_back(new Sum(tokenizer));
    }
}
PyObject ShiftExpr::evaluate(Stack stack) {
    log("ShiftExpr::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    while (results.size() > 1) {
        string op = results.at(1);
        PyObject right = results.at(2);
        results.at(0) = apply_shift_op(results.at(0), op, right);
        results.erase(results.begin()+1, results.begin()+3);
    }
    sub_indent(2);
    return results.front();
}
ostream& ShiftExpr::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Sum

// sum:
//     | sum '+' term 
//     | sum '-' term 
//     | term
Sum::Sum(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Sum::parse() {
    children.push_back(new Term(tokenizer));
    while(is_sum_op(peek("Sum").value)) {
        children.push_back(new Op(tokenizer));
        children.push_back(new Term(tokenizer));
    }
}
PyObject Sum::evaluate(Stack stack) {
    log("Sum::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    while (results.size() > 1) {
        string op = results.at(1);
        PyObject right = results.at(2);
        results.at(0) = apply_sum_op(results.at(0), op, right);
        results.erase(results.begin()+1, results.begin()+3);
    }
    sub_indent(2);
    return results.front();
}
ostream& Sum::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Term

// term:
//     | term '*' factor 
//     | term '/' factor 
//     | term '//' factor 
//     | term '%' factor 
//     | term '@' factor 
//     | factor
Term::Term(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Term::parse() {
    children.push_back(new Factor(tokenizer));
    while(is_term_op(peek("Term").value)) {
        children.push_back(new Op(tokenizer));
        children.push_back(new Factor(tokenizer));
    }
}
PyObject Term::evaluate(Stack stack) {
    log("Term::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    while (results.size() > 1) {
        string op = results.at(1);
        PyObject right = results.at(2);
        results.at(0) = apply_term_op(results.at(0), op, right);
        results.erase(results.begin()+1, results.begin()+3);
    }
    sub_indent(2);
    return results.front();
}
ostream& Term::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Factor

// factor:
//     | '+' factor 
//     | '-' factor 
//     | '~' factor 
//     | power
Factor::Factor(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Factor::parse() {
    if (is_factor_op(peek("Factor").value)) {
        if (peek("Factor").value == "+") {
            eat_value("+", "Factor");  // the '+' doesn't really do anything
        }
        children.push_back(new Op(tokenizer));
    }
    children.push_back(new Power(tokenizer));
}
PyObject Factor::evaluate(Stack stack) {
    log("Factor::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 1) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    string op = children.at(0)->evaluate(stack);
    PyObject val = children.at(1)->evaluate(stack);
    sub_indent(2);
    if (op == "-") {
        if (val.type == "int") {
            return PyObject(-(int)val, "int");
        }
        else if (val.type == "float") {
            return PyObject(-(float)val, "float");
        }
        else if (val.type == "bool") {
            return PyObject(-(bool)val, "bool");
        }
        else {
            throw runtime_error("Unsupported PyObject of type \'" 
                            + val.type + "\' in Factor::evaluate()\n");
        }
    }
    if (op == "~") {
        return PyObject((int)(~val), "int");
    }
    throw runtime_error("reached end of Factor::evaluate() without returning");
}
ostream& Factor::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Power

// power:
//     | await_primary '**' factor 
//     | await_primary
Power::Power(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Power::parse() {
    children.push_back(new AwaitPrimary(tokenizer));
    if (peek("Power").value == "**") {
        eat_value("**", "Power");
        children.push_back(new Factor(tokenizer));
    }
}
PyObject Power::evaluate(Stack stack) {
    log("Power::evaluate()", DEBUG); add_indent(2);
    PyObject ret;
    if (children.size() == 1){
        ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    ret = children.at(0)->evaluate(stack)._pow(children.at(1)->evaluate(stack));
    sub_indent(2);
    return ret;
}
ostream& Power::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// AwaitPrimary

// await_primary:
//     | AWAIT primary 
//     | primary
AwaitPrimary::AwaitPrimary(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void AwaitPrimary::parse() {
    // TODO: implement this completely
    children.push_back(new Primary(tokenizer));
}
PyObject AwaitPrimary::evaluate(Stack stack) {
    log("AwaitPrimary::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& AwaitPrimary::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Primary

// primary:
//     | invalid_primary  # must be before 'primay genexp' because of invalid_genexp
//     | primary '.' NAME 
//     | primary genexp 
//     | primary '(' [arguments] ')' 
//     | primary '[' slices ']' 
//     | atom
Primary::Primary(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Primary::parse() {
    // NOTE: this is supposed to be a left recusive production
    // im keeping it simple atm but its likely that a while loop will
    // be needed like Term, Sum, etc
    children.push_back(new Atom(tokenizer));
    if (peek("Primary").value == ".") {
        // TODO: implement
        throw runtime_error("Primary: '.' not implemented");
    } else if(peek("Primary").value == "(") {
        children.push_back(new _String(tokenizer));  // (
        children.push_back(new Arguments(tokenizer));
        children.push_back(new _String(tokenizer));  // )
    }
}
PyObject Primary::evaluate(Stack stack) {
    log("Primary::evaluate()", DEBUG); add_indent(2);
    PyObject ret;
    if (children.size() == 1) {
        ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    else {
        PyObject c1 = children.at(1)->evaluate(stack);
        if ((string)c1 == "(") {
            PyObject func = children.at(0)->evaluate(stack);
            PyObject args = children.at(2)->evaluate(stack);
            PyObject ret = stack.call_function(func, args);
            sub_indent(2);
            return ret;
        }
    }
    throw runtime_error("reached end of Primary::evaluate() without returning");
}
ostream& Primary::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Slices

// slices:
//     | slice !',' 
//     | ','.slice+ [','] 
Slices::Slices(Tokenizer *tokenizer) {

}
void Slices::parse() {

}
PyObject Slices::evaluate(Stack stack) {
    throw runtime_error("Slices::evaluate() not implemented");
}
ostream& Slices::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Slice

// slice:
//     | [expression] ':' [expression] [':' [expression] ] 
//     | expression 
Slice::Slice(Tokenizer *tokenizer) {

}
void Slice::parse() {

}
PyObject Slice::evaluate(Stack stack) {
    throw runtime_error("Slice::evaluate() not implemented");
}
ostream& Slice::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Atom

// atom:
//     | NAME
//     | 'True' 
//     | 'False' 
//     | 'None' 
//     | '__peg_parser__' 
//     | strings
//     | NUMBER
//     | (tuple | group | genexp)
//     | (list | listcomp)
//     | (dict | set | dictcomp | setcomp)
//     | '...' 
Atom::Atom(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Atom::parse() {
    if (is_number(peek("Atom").value)) {
        children.push_back(new Number(next_token()));
    } 
    else if (peek("Atom").value == "True" || peek("Atom").value == "False") {
        children.push_back(new Bool(next_token()));
    }
    else if (peek("Atom").value == "(") {
        // TODO: figure out a cleaner way to determine if this should be
        // a tuple, group, or genexp
        children.push_back(new Tuple(tokenizer));
    }
    else if (peek("Atom").value == "[") {
        // TODO: figure out a cleaner way to determine if this should be
        // a list or listcomp
        children.push_back(new List(tokenizer));
    }
    else {
        children.push_back(new _String(next_token()));
    }
}
PyObject Atom::evaluate(Stack stack) {
    log("Atom::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& Atom::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// List

// list:
//     | '[' [star_named_expressions] ']'
List::List(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void List::parse() {
    eat_value("[", "List");
    children.push_back(new StarNamedExpressions(tokenizer));
    eat_value("]", "List");
}
PyObject List::evaluate(Stack stack) {
    log("List::evaluate()", DEBUG); add_indent(2);
    PyObject ret = PyObject(children.at(0)->evaluate(stack).as_list(), "list");
    sub_indent(2);
    return ret;
}
ostream& List::print(ostream& os) const {
    os << "[";
    for (int i=0; i < children.size()-1; i++) {
        os << *(children[i]) << ",";
    }
    os << *children.back();
    os << "]";
    return os;
}

//===============================================================
// Tuple

// NOTE: a tuple most contain 1 or more commas
// >>> (1)
// 1
// >>> (1,)
// (1,)

// tuple:
//     | '(' [star_named_expression ',' [star_named_expressions]  ] ')'
Tuple::Tuple(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Tuple::parse() {
    eat_value("(", "Tuple");
    if (peek("Tuple").value != ")") {
        children.push_back(new StarNamedExpression(tokenizer));
        eat_value(",", "Tuple");
    }
    if (peek("Tuple").value != ")") {
        children.push_back(new StarNamedExpressions(tokenizer));
    }
    eat_value(")", "Tuple");
}
PyObject Tuple::evaluate(Stack stack) {
    log("Tuple::evaluate()", DEBUG); add_indent(2);
    if (children.size() == 0) {
        PyObject ret = children.at(0)->evaluate(stack);
        sub_indent(2);
        return ret;
    }
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    sub_indent(2);
    return PyObject(results, "tuple");
}
ostream& Tuple::print(ostream& os) const {
    os << "(";
    for (int i=0; i < children.size()-1; i++) {
        os << *(children[i]) << ",";
    }
    os << *children.back();
    os << ")";
    return os;
}

//===============================================================
// Arguments

// arguments:
//     | args [','] &')' 
Arguments::Arguments(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Arguments::parse() {
    if (peek("Arguments").value != ")") {
        children.push_back(new Args(tokenizer));
        while (peek("Arguments").value == ",") {
            eat_value(",", "Arguments");
            children.push_back(new Args(tokenizer));
        }
    }
}
PyObject Arguments::evaluate(Stack stack) {
    log("Arguments::evaluate()", DEBUG); add_indent(2);
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    sub_indent(2);
    return PyObject(results, "list");
}
ostream& Arguments::print(ostream& os) const {
    if (children.size() > 1) {
        for (int i=0; i < children.size()-1; i++) {
            os << *(children[i]) << ",";
        }
        os << *children.back();
    }
    else if (children.size() == 1) os << *children.back();
    return os;
}

//===============================================================
// Args

// args:
//     | ','.(starred_expression | named_expression !'=')+ [',' kwargs ] 
//     | kwargs 
Args::Args(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
void Args::parse() {
    //TODO: args are really complicated, im simplifying them for now
    children.push_back(new Expression(tokenizer));
}
PyObject Args::evaluate(Stack stack) {
    log("Args::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& Args::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}

//===============================================================
// Op

Op::Op(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG);
    this->tokenizer = tokenizer;
    token = next_token();
    log(__FUNCTION__ + (string)" - token.value == '" + token.value + "'", DEBUG);
}
PyObject Op::evaluate(Stack stack) {
    log("Op::evaluate()", DEBUG);
    return PyObject(token.value, "str");
}
ostream& Op::print(ostream& os) const {
    os << token.value;
    return os;
}

//===============================================================
// String

_String::_String(Token token) {
    log(__FUNCTION__, DEBUG);
    this->token = token;
    parse();
    log(__FUNCTION__ + (string)" - this->value == '" + this->value + "'", DEBUG);
}
_String::_String(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG);
    this->token = tokenizer->next_token();
    parse();
    log(__FUNCTION__ + (string)" - this->value == '" + this->value + "'", DEBUG);
}
void _String::parse() {
    if (token.value.size() > 0 && token.value.at(0) == '"') {
        this->value = token.value.substr(1, token.value.size()-2);
    } else {
        this->value = token.value;
    }
}
PyObject _String::evaluate(Stack stack) {
    log("_String::evaluate()", DEBUG);
    return PyObject(this->value, "str");
}
ostream& _String::print(ostream& os) const {
    os << token.value;
    return os;
}

//===============================================================
// Number

Number::Number(Token token) {
    log(__FUNCTION__, DEBUG);
    this->token = token;
    parse();
    log(__FUNCTION__ + (string)" - token.value == '" + token.value + "'", DEBUG);
}
Number::Number(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG);
    this->token = tokenizer->next_token();
    parse();
    log(__FUNCTION__ + (string)" - token.value == '" + token.value + "'", DEBUG);
}
void Number::parse() {
    this->is_int = this->token.value.find(".") == string::npos;
}
PyObject Number::evaluate(Stack stack) {
    log("Number::evaluate()", DEBUG);
    if (this->is_int) {
        return PyObject(stoi(token.value), "int");
    }
    return PyObject(stof(token.value), "float");
}
ostream& Number::print(ostream& os) const {
    os << token.value;
    return os;
}

//===============================================================
// Bool

Bool::Bool(Token token) {
    log(__FUNCTION__, DEBUG);
    this->token = token;
    parse();
    log(__FUNCTION__ + (string)" - token.value == '" + token.value + "'", DEBUG);
}
Bool::Bool(Tokenizer *tokenizer) {
    log(__FUNCTION__, DEBUG);
    this->token = token;
    parse();
    log(__FUNCTION__ + (string)" - token.value == '" + token.value + "'", DEBUG);
}
void Bool::parse() {
    this->bool_value = token.value == "True";
}
PyObject Bool::evaluate(Stack stack) {
    log("Bool::evaluate()", DEBUG);
    PyObject res = PyObject(this->bool_value, "bool");
    return res;
}
ostream& Bool::print(ostream& os) const {
    os << token.value;
    return os;
}

//===============================================================
