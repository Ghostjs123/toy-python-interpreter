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
AST::AST(Tokenizer *tokenizer, string indent) {
    this->tokenizer = tokenizer;
    this->indent = indent;
}
AST::~AST() {
    // log(__FUNCTION__ + (string)" - " + to_string(rewind_amt), DEBUG);  // this is spammy
    if (rewind_amt > 0) tokenizer->rewind(rewind_amt);
}
Token AST::peek(string func_name) {
    try {
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
    Token t = tokenizer->next_token();
    log(": " + t.as_string(), DEBUG);
    return t;
}
void AST::eat_value(string exp_value, string func_name) {
    Token next = next_token();
    if (next.value != exp_value) {
        throw runtime_error("ate '" + next.value + "' expected '" 
                            + exp_value + "' in '" + func_name + "'");
    }
    rewind_amt++;
}
void AST::eat_type(string exp_type, string func_name) {
    Token next = next_token();
    if (next.type != exp_type) {
        throw runtime_error("ate '" + next.type + "' expected '" 
                            + exp_type + "' in '" + func_name + "'");
    }
    rewind_amt++;
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
File::File(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
File::~File() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void File::parse() {
    children.push_back(new Statements(tokenizer, indent));
    eat_type("ENDMARKER", "File");
}
PyObject File::evaluate(Stack stack) {
    log("File::evaluate()", DEBUG); add_indent(2);
    children.at(0)->evaluate(stack);
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
Interactive::Interactive(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Interactive::~Interactive() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Interactive::parse() {
    children.push_back(new StatementNewline(tokenizer, indent));
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
Statements::Statements(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Statements::~Statements() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Statements::parse() {
    while (peek("Statements").type != "ENDMARKER") {
        children.push_back(new Statement(tokenizer, indent));
    }
}
PyObject Statements::evaluate(Stack stack) {
    log("Statements::evaluate()", DEBUG); add_indent(2);
    for (AST* child : children) {
        child->evaluate(stack);
    }
    sub_indent(2);
    return PyObject();
}
ostream& Statements::print(ostream& os) const {
    for (AST* child : children) os << *child;
    return os;
}

//===============================================================
// Statement

// NOTE: this attempts to parse a CompoundStmt, falls back to SimpleStmt

// statement: compound_stmt  | simple_stmt
Statement::Statement(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Statement::~Statement() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Statement::parse() {
    CompoundStmt *temp = new CompoundStmt(tokenizer, indent);
    if (temp->children.size() == 0) {
        delete temp;
        children.push_back(new SimpleStmt(tokenizer, indent));
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
StatementNewline::StatementNewline(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
StatementNewline::~StatementNewline() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void StatementNewline::parse() {
    if (peek("StatementNewline").type == "NEWLINE") {
        eat_type("NEWLINE", "StatementNewline");
    } else if (peek("StatementNewline").type == "NL") {
        eat_type("NL", "StatementNewline");
    } else if (peek("StatementNewline").type == "ENDMARKER") {
        eat_type("ENDMARKER", "StatementNewline");
    } else {
        CompoundStmt *temp = new CompoundStmt(tokenizer, indent);
        if (temp->children.size() == 0) {
            delete temp;
            children.push_back(new SimpleStmt(tokenizer, indent));
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
SimpleStmt::SimpleStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
SimpleStmt::~SimpleStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void SimpleStmt::parse() {
    // TODO: grammar needs to support ;
    children.push_back(new SmallStmt(tokenizer, indent));
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
SmallStmt::SmallStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
SmallStmt::~SmallStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void SmallStmt::parse() {
    if (peek("SmallStmt").value == "return") {
        children.push_back(new ReturnStmt(tokenizer, indent));
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
        children.push_back(new StarExpressions(tokenizer, indent));
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
CompoundStmt::CompoundStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
CompoundStmt::~CompoundStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void CompoundStmt::parse() {
    if (peek("CompoundStmt").value == "if") {
        children.push_back(new IfStmt(tokenizer, indent));
    }
    else if (peek("CompoundStmt").value == "for") {
        children.push_back(new ForStmt(tokenizer, indent));
    }
    else if (peek("CompoundStmt").value == "while") {
        children.push_back(new WhileStmt(tokenizer, indent));
    }
    else if (peek("CompoundStmt").value == "def") {
        children.push_back(new FunctionDef(tokenizer, indent));
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
Assignment::Assignment(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Assignment::~Assignment() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
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
IfStmt::IfStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
IfStmt::~IfStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void IfStmt::parse() {
    eat_value("if", "IfStmt");
    children.push_back(new NamedExpression(tokenizer, indent));
    eat_value(":", "IfStmt");
    children.push_back(new Block(tokenizer, indent));
    // NOTE: if its an if-elif-else, 
    //  the else block should end up in the ElifStmt production
    if (peek("IfStmt").value == "elif") {
        children.push_back(new ElifStmt(tokenizer, indent));
    }
    else if (peek("IfStmt").value == "else") {
        children.push_back(new ElseBlock(tokenizer, indent));
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
ElifStmt::ElifStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    _else = nullptr;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ElifStmt::~ElifStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void ElifStmt::parse() {
    NamedExpression *t1;
    Block *t2;
    while (peek("ElifStmt").value == "elif") {
        eat_value("elif", "ElifStmt");
        t1 = new NamedExpression(tokenizer, indent);
        eat_value(":", "ElifStmt");
        t2 = new Block(tokenizer, indent);
        _elifs[t1] = t2;
    }
    if (peek("ElifStmt").value == "else") {
        _else = new ElseBlock(tokenizer, indent);
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
        os << indent << "elif " << *(it->first) << ":";
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
ElseBlock::ElseBlock(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ElseBlock::~ElseBlock() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void ElseBlock::parse() {
    eat_value("else", "ElseBlock");
    eat_value(":", "ElseBlock");
    children.push_back(new Block(tokenizer, indent));
}
PyObject ElseBlock::evaluate(Stack stack) {
    log("ElseBlock::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& ElseBlock::print(ostream& os) const {
    os << indent << "else:" << *children.at(0);
    return os;
}

//===============================================================
// WhileStmt

// while_stmt:
//     | 'while' named_expression ':' block [else_block] 
WhileStmt::WhileStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
WhileStmt::~WhileStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
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
ForStmt::ForStmt(Tokenizer *tokenizer, string indent) {
    // Loop better: a deeper look at iteration in Python
    // https://www.youtube.com/watch?v=V2PkkMS2Ack
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ForStmt::~ForStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
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
WithStmt::WithStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
WithStmt::~WithStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
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
WithItem::WithItem(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
WithItem::~WithItem() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
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

TryStmt::TryStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
TryStmt::~TryStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
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
ExceptBlock::ExceptBlock(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ExceptBlock::~ExceptBlock() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
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
FinallyBlock::FinallyBlock(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
FinallyBlock::~FinallyBlock() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
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
ReturnStmt::ReturnStmt(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ReturnStmt::~ReturnStmt() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void ReturnStmt::parse() {
    eat_value("return", "ReturnStmt");
    children.push_back(new StarExpressions(tokenizer, indent));
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
FunctionDef::FunctionDef(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
FunctionDef::~FunctionDef() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
    delete this->raw;
}
void FunctionDef::parse() {
    this->raw = new FunctionDefRaw(tokenizer, indent);
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
FunctionDefRaw::FunctionDefRaw(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
FunctionDefRaw::~FunctionDefRaw() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    delete this->body;
    delete this->params;
    tokenizer->rewind(1);  // name
    sub_indent(2);
}
void FunctionDefRaw::parse() {
    eat_value("def", "FunctionDefRaw");
    this->name = next_token().value;
    eat_value("(", "FunctionDefRaw");
    this->params = new Params(tokenizer, indent);
    eat_value(")", "FunctionDefRaw");
    eat_value(":", "FunctionDefRaw");
    this->body = new Block(tokenizer, indent);
}
PyObject FunctionDefRaw::evaluate(Stack stack) {
    log("FunctionDefRaw::evaluate()", DEBUG); add_indent(2); sub_indent(2);
    // function definition shouldnt return anything
    return PyObject();
}
ostream& FunctionDefRaw::print(ostream& os) const {
    os << "def " << name << "(" << *params << "):" << *body;
    return os;
}

//===============================================================
// Params

// params:
//     | parameters
Params::Params(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Params::~Params() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Params::parse() {
    children.push_back(new Parameters(tokenizer, indent));
}
PyObject Params::evaluate(Stack stack) {
    log("Params::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return PyObject();
}
ostream& Params::print(ostream& os) const {
    os << *children.at(0);
    return os;
}


//===============================================================
// Parameters

// NOTE: slash_no_default vs slash_with_default are important
// no_default uses + while with_default uses *

// parameters:
//     | slash_no_default param_no_default* param_with_default* [star_etc]
//     | slash_with_default param_with_default* [star_etc]
//     | param_no_default+ param_with_default* [star_etc]
//     | param_with_default+ [star_etc]
//     | star_etc
Parameters::Parameters(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Parameters::~Parameters() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Parameters::parse() {
    // NOTE: parsing this thing is gonna be weird, my main goal is to determine two things:
    // 1. does it need to swap from no_default -> with_default
    // 2. did it find a /, to be lazy im gonna lookahead for the / first
    // I'm checking these by checking the amount of children in the AST after parsing
    // , if 0 then mismatched
    
    int i = 0;
    while(lookahead(i).value != "/" && lookahead(i).value != ")") i++;

    if (lookahead(i).value == "/" ) {
        SlashNoDefault* snd = new SlashNoDefault(tokenizer, indent);
        if (snd->children.size() > 0) {
            //     | slash_no_default param_no_default* param_with_default* [star_etc]
            children.push_back(snd);
            ParamNoDefault* pnd;
            while (1) {
                pnd = new ParamNoDefault(tokenizer, indent);
                if (pnd->children.size() == 0) {
                    delete pnd;
                    break;
                }
                else { children.push_back(pnd); }
            }
            while (peek("Parameters").value != "*" 
                    && peek("Parameters").value != "**" && peek("Parameters").value != ")") {
                children.push_back(new ParamWithDefault(tokenizer, indent));
            }
        }
        else {
            //     | slash_with_default param_with_default* [star_etc]
            delete snd;

            children.push_back(new SlashWithDefault(tokenizer, indent));
            while (peek("Parameters").value != "*" 
                    && peek("Parameters").value != "**" && peek("Parameters").value != ")") {
                children.push_back(new ParamWithDefault(tokenizer, indent));
            }
        }
    }
    else {
        //     | param_no_default+ param_with_default* [star_etc]
        //     | param_with_default+ [star_etc]
        ParamNoDefault* pnd;
        while (1) {
            pnd = new ParamNoDefault(tokenizer, indent);
            if (pnd->children.size() == 0) {
                delete pnd;
                break;
            }
            else { children.push_back(pnd); }
        }
        while (peek("Parameters").value != "*" 
                && peek("Parameters").value != "**" && peek("Parameters").value != ")") {
            children.push_back(new ParamWithDefault(tokenizer, indent));
        }
    }

    // all productions end with optional star_etc
    if (peek("Parameters").value == "*" || peek("Parameters").value == "**") {
        children.push_back(new StarEtc(tokenizer, indent));
    }
}
PyObject Parameters::evaluate(Stack stack) {
    log("Parameters::evaluate()", DEBUG); add_indent(2);
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    sub_indent(2);
    return PyObject(results, "tuple");
}
ostream& Parameters::print(ostream& os) const {
    for (AST* child : children) os << *child;
    return os;
}

//===============================================================
// SlashNoDefault

// slash_no_default:
//     | param_no_default+ '/' ',' 
//     | param_no_default+ '/' &')' 
SlashNoDefault::SlashNoDefault(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
SlashNoDefault::~SlashNoDefault() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void SlashNoDefault::parse() {
    // push as many param_no_default's as possible, if any are a with_default, return 0 children
    while(peek("SlashNoDefault").value != "=" 
            && peek("SlashNoDefault").value != "/" && peek("SlashNoDefault").value != ")") {
        children.push_back(new ParamNoDefault(tokenizer, indent));
    }
    if (peek("SlashNoDefault").value == "=") {
        // should have been a SlashWithDefault
        children.clear();
    sub_indent(2);
        return;
    }
    eat_value("/", "SlashNoDefault");
    if (peek("SlashNoDefault").value == ",") {
        eat_value(",", "SlashNoDefault");
    }
    else if (peek("SlashNoDefault").value != ")") {
        throw runtime_error("SyntaxError: invalid syntax");
    }
}
PyObject SlashNoDefault::evaluate(Stack stack) {
    log("SlashNoDefault::evaluate()", DEBUG); add_indent(2);
    
    sub_indent(2);
    return PyObject();
}
ostream& SlashNoDefault::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// SlashWithDefault

// slash_with_default:
//     | param_no_default* param_with_default+ '/' ',' 
//     | param_no_default* param_with_default+ '/' &')'
SlashWithDefault::SlashWithDefault(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
SlashWithDefault::~SlashWithDefault() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void SlashWithDefault::parse() {
    ParamNoDefault* pnd;
    while (1) {
        pnd = new ParamNoDefault(tokenizer, indent);
        if (pnd->children.size() == 0) {
            delete pnd;
            break;
        }
        else { children.push_back(pnd); }
    }
    while (peek("SlashWithDefault").value != "/" && peek("SlashWithDefault").value != ")") {
        children.push_back(new ParamWithDefault(tokenizer, indent));
    }
    eat_value("/", "SlashWithDefault");
    if (peek("SlashWithDefault").value == ",") {
        eat_value(",", "SlashWithDefault");
    }
    else if (peek("SlashWithDefault").value != ")") {
        throw runtime_error("SyntaxError: invalid syntax");
    }
}
PyObject SlashWithDefault::evaluate(Stack stack) {
    log("SlashWithDefault::evaluate()", DEBUG); add_indent(2);
    
    sub_indent(2);
    return PyObject();
}
ostream& SlashWithDefault::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// StarEtc

// star_etc:
//     | '*' param_no_default param_maybe_default* [kwds] 
//     | '*' ',' param_maybe_default+ [kwds] 
//     | kwds
StarEtc::StarEtc(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
StarEtc::~StarEtc() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void StarEtc::parse() {
    if (peek("StarEtc").value == "*") {
        eat_value("*", "StarEtc");
        if (peek("StarEtc").value == ",") {
            // '*' ',' param_maybe_default+ [kwds] 
            eat_value(",", "StarEtc");
            children.push_back(new ParamMaybeDefault(tokenizer, indent));
        }
        else {
            // '*' param_no_default param_maybe_default* [kwds] 
            children.push_back(new ParamNoDefault(tokenizer, indent));
            while(peek("StarEtc").value != "**" && peek("StarEtc").value != ")") {
                children.push_back(new ParamMaybeDefault(tokenizer, indent));
            }
        }
    }
    // all productions have a potential kwds at the end
    if (peek("StarEtc").value == "**") {
        children.push_back(new Kwds(tokenizer, indent));
    }
    if (children.size() == 0) {
        throw runtime_error("Reached end of StarEtc with no children");
    }
}
PyObject StarEtc::evaluate(Stack stack) {
    log("StarEtc::evaluate()", DEBUG); add_indent(2);
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    sub_indent(2);
    return PyObject(results, "tuple");
}
ostream& StarEtc::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// Kwds

// kwds: '**' param_no_default
Kwds::Kwds(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Kwds::~Kwds() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Kwds::parse() {
    eat_value("**", "Kwds");
    children.push_back(new ParamNoDefault(tokenizer, indent));
}
PyObject Kwds::evaluate(Stack stack) {
    log("Kwds::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return ret;
}
ostream& Kwds::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// ParamNoDefault

// param_no_default:
//     | param ',' TYPE_COMMENT? 
//     | param TYPE_COMMENT? &')'
ParamNoDefault::ParamNoDefault(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ParamNoDefault::~ParamNoDefault() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void ParamNoDefault::parse() {
    if (peek("ParamNoDefault").value != ")") {
        children.push_back(new Param(tokenizer, indent));
        if (peek("ParamNoDefault").value == "=") {
            // this actually should have been a with_default,
            // signal by returning w/ 0 children
            children.pop_back();
            return;
        }
        if (peek("ParamNoDefault").value == ",") {
            eat_value(",", "ParamNoDefault");
        }
    }
}
PyObject ParamNoDefault::evaluate(Stack stack) {
    log("ParamNoDefault::evaluate()", DEBUG); add_indent(2);
    PyObject param_name = children.at(0)->evaluate(stack);
    PyObject arg = stack.next_param();

    // if (get<0>(arg) != PyObject() && get<0>(arg) != param_name) {
    //     string func_name = stack.get_function_name();
    //     throw runtime_error(
    //         "TypeError: " + func_name + "() got an unexpected keyword argument '" 
    //         + get<0>(arg).as_string() + "'");
    // }
    sub_indent(2);
    return PyObject();
}
ostream& ParamNoDefault::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// ParamWithDefault

// param_with_default:
//     | param default ',' TYPE_COMMENT? 
//     | param default TYPE_COMMENT? &')' 
ParamWithDefault::ParamWithDefault(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ParamWithDefault::~ParamWithDefault() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void ParamWithDefault::parse() {
    if (peek("ParamNoDefault").value != ")") {
        children.push_back(new Param(tokenizer, indent));
        children.push_back(new Default(tokenizer, indent));
        if (peek("ParamNoDefault").value == ",") {
            eat_value(",", "ParamNoDefault");
        }
    }
}
PyObject ParamWithDefault::evaluate(Stack stack) {
    log("ParamWithDefault::evaluate()", DEBUG); add_indent(2);
    
    sub_indent(2);
    return PyObject();
}
ostream& ParamWithDefault::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// ParamMaybeDefault

// param_maybe_default:
//     | param default? ',' TYPE_COMMENT? 
//     | param default? TYPE_COMMENT? &')'
ParamMaybeDefault::ParamMaybeDefault(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ParamMaybeDefault::~ParamMaybeDefault() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void ParamMaybeDefault::parse() {
    if (peek("ParamMaybeDefault").value != ")") {
        children.push_back(new Param(tokenizer, indent));
        Default* d = new Default(tokenizer, indent);
        if (d->children.size() == 0) {
            delete d;
        }
        else {
            children.push_back(d);
        }
        if (peek("ParamMaybeDefault").value == ",") {
            eat_value(",", "ParamMaybeDefault");
        }
    }
}
PyObject ParamMaybeDefault::evaluate(Stack stack) {
    log("ParamMaybeDefault::evaluate()", DEBUG); add_indent(2);
    
    sub_indent(2);
    return PyObject();
}
ostream& ParamMaybeDefault::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// Param

// param: NAME annotation?
Param::Param(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Param::~Param() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Param::parse() {
    this->name = new Name(tokenizer, indent);
}
PyObject Param::evaluate(Stack stack) {
    log("Param::evaluate()", DEBUG);
    // NOTE: calling evaluate on the Name* will call get_value()
    // on the stack, I just want the actual name of the Param
    return PyObject(this->name->token.value, "str");
}
ostream& Param::print(ostream& os) const {
    os << this->name->token.value;
    return os;
}

//===============================================================
// Default

// default: '=' expression
Default::Default(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Default::~Default() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Default::parse() {
    if (peek("Default").value == "=") {
        eat_value("=", "Default");
        children.push_back(new Expression(tokenizer, indent));
    }
    // NOTE: if 0 children case is for maybe_default productions
}
PyObject Default::evaluate(Stack stack) {
    log("Default::evaluate()", DEBUG); add_indent(2);
    return children.at(0)->evaluate(stack);
    sub_indent(2);
    return PyObject();
}
ostream& Default::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// Block

// NOTE: I am intentionally not calling Statements here, using Statement instead

// block:
//     | NEWLINE INDENT statements DEDENT 
//     | simple_stmt
Block::Block(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Block::~Block() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Block::parse() {
    // the NEWLINE can be optional, for instance one liner function like
    // def a(): return 1
    // if this happens it will also not have an INDENT or DEDENT
    // consequently it will only have 1 Statement
    if (peek("Block").type == "NEWLINE") {
        this->has_newline = true;
        eat_type("NEWLINE", "Block");
        this->indent = peek("Block").value;
        eat_type("INDENT", "Block");
        while (peek("Block").type != "DEDENT") {
            children.push_back(new Statement(tokenizer, indent));
        }
        eat_type("DEDENT", "Block");
    }
    else {
        this->has_newline = false;
        this->indent = "";
        children.push_back(new Statement(tokenizer, indent));
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
    if (has_newline) {
        for (AST *child : children) {
            os << endl << indent  << *child;
        }
    }
    else {
        os << *children.at(0);
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
StarExpressions::StarExpressions(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
StarExpressions::~StarExpressions() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void StarExpressions::parse() {
    children.push_back(new StarExpression(tokenizer, indent));
    while(peek("StarExpressions").value == ",") {
        children.push_back(new StarExpression(tokenizer, indent));
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
StarExpression::StarExpression(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
StarExpression::~StarExpression() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void StarExpression::parse() {
    if (peek("StarExpression").value == "*") {
        children.push_back(new Op(tokenizer, indent));
        children.push_back(new BitwiseOr(tokenizer, indent));
    }
    else {
        children.push_back(new Expression(tokenizer, indent));
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
StarNamedExpressions::StarNamedExpressions(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
StarNamedExpressions::~StarNamedExpressions() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void StarNamedExpressions::parse() {
    children.push_back(new StarNamedExpression(tokenizer, indent));
    while (peek("StarNamedExpressions").value == ",") {
        eat_value(",", "StarNamedExpressions");
        children.push_back(new StarNamedExpression(tokenizer, indent));
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
StarNamedExpression::StarNamedExpression(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
StarNamedExpression::~StarNamedExpression() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void StarNamedExpression::parse() {
    log("StarNamedExpression::evaluate()", DEBUG); add_indent(2);
    if (peek("StarNamedExpression").value == "*") {
        children.push_back(new Op(tokenizer, indent));
        children.push_back(new BitwiseOr(tokenizer, indent));
    }
    else {
        children.push_back(new NamedExpression(tokenizer, indent));
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
NamedExpression::NamedExpression(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
NamedExpression::~NamedExpression() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void NamedExpression::parse() {
    // TODO: handle ':=' case
    children.push_back(new Expression(tokenizer, indent));
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
Expressions::Expressions(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Expressions::~Expressions() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Expressions::parse() {
    children.push_back(new Expression(tokenizer, indent));
    while(peek("Expressions").value == ",") {
        children.push_back(new Expression(tokenizer, indent));
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
Expression::Expression(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Expression::~Expression() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Expression::parse() {
    // NOTE: skipping lambdef
    if (lookahead(1).value == "if") {

    }
    children.push_back(new Disjunction(tokenizer, indent));
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
Disjunction::Disjunction(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Disjunction::~Disjunction() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Disjunction::parse() {
    children.push_back(new Conjunction(tokenizer, indent));
    while(peek("Disjunction").value == "or") {
        eat_value("or", "Disjunction");
        children.push_back(new Conjunction(tokenizer, indent));
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
Conjunction::Conjunction(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Conjunction::~Conjunction() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Conjunction::parse() {
    children.push_back(new Inversion(tokenizer, indent));
    while(peek("Conjunction").value == "and") {
        eat_value("and", "Conjunction");
        children.push_back(new Inversion(tokenizer, indent));
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
Inversion::Inversion(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Inversion::~Inversion() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Inversion::parse() {
    if (peek("Inversion").value == "not") {
        children.push_back(new Op(tokenizer, indent));
    }
    children.push_back(new Comparison(tokenizer, indent));
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
Comparison::Comparison(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Comparison::~Comparison() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Comparison::parse() {
    children.push_back(new BitwiseOr(tokenizer, indent));
    while (is_comparison_op(tokenizer)) {
        children.push_back(new Op(tokenizer, indent));
        children.push_back(new BitwiseOr(tokenizer, indent));
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
BitwiseOr::BitwiseOr(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
BitwiseOr::~BitwiseOr() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void BitwiseOr::parse() {
    children.push_back(new BitwiseXor(tokenizer, indent));
    while (peek("BitwiseOr").value == "|") {
        eat_value("|", "BitwiseOr");
        children.push_back(new BitwiseXor(tokenizer, indent));
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
BitwiseXor::BitwiseXor(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
BitwiseXor::~BitwiseXor() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void BitwiseXor::parse() {
    children.push_back(new BitwiseAnd(tokenizer, indent));
    while (peek("BitwiseXor").value == "^") {
        eat_value("^", "BitwiseXor");
        children.push_back(new BitwiseAnd(tokenizer, indent));
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
BitwiseAnd::BitwiseAnd(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
BitwiseAnd::~BitwiseAnd() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void BitwiseAnd::parse() {
    children.push_back(new ShiftExpr(tokenizer, indent));
    while (peek("BitwiseAnd").value == "&") {
        eat_value("&", "BitwiseAnd");
        children.push_back(new ShiftExpr(tokenizer, indent));
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
ShiftExpr::ShiftExpr(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
ShiftExpr::~ShiftExpr() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void ShiftExpr::parse() {
    children.push_back(new Sum(tokenizer, indent));
    while (peek("ShiftExpr").value == "<<" || peek("ShiftExpr").value == ">>") {
        children.push_back(new Op(tokenizer, indent));
        children.push_back(new Sum(tokenizer, indent));
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
Sum::Sum(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Sum::~Sum() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Sum::parse() {
    children.push_back(new Term(tokenizer, indent));
    while(is_sum_op(peek("Sum").value)) {
        children.push_back(new Op(tokenizer, indent));
        children.push_back(new Term(tokenizer, indent));
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
Term::Term(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Term::~Term() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Term::parse() {
    children.push_back(new Factor(tokenizer, indent));
    while(is_term_op(peek("Term").value)) {
        children.push_back(new Op(tokenizer, indent));
        children.push_back(new Factor(tokenizer, indent));
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
Factor::Factor(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Factor::~Factor() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Factor::parse() {
    if (is_factor_op(peek("Factor").value)) {
        if (peek("Factor").value == "+") {
            eat_value("+", "Factor");  // the '+' doesn't really do anything
        }
        children.push_back(new Op(tokenizer, indent));
    }
    children.push_back(new Power(tokenizer, indent));
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
Power::Power(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Power::~Power() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Power::parse() {
    children.push_back(new AwaitPrimary(tokenizer, indent));
    if (peek("Power").value == "**") {
        eat_value("**", "Power");
        children.push_back(new Factor(tokenizer, indent));
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
AwaitPrimary::AwaitPrimary(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
AwaitPrimary::~AwaitPrimary() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void AwaitPrimary::parse() {
    // TODO: implement this completely
    children.push_back(new Primary(tokenizer, indent));
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
Primary::Primary(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Primary::~Primary() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Primary::parse() {
    // NOTE: this is supposed to be a left recusive production
    // im keeping it simple atm but its likely that a while loop will
    // be needed like Term, Sum, etc
    children.push_back(new Atom(tokenizer, indent));
    if (peek("Primary").value == ".") {
        // TODO: implement
        throw runtime_error("Primary: '.' not implemented");
    } else if(peek("Primary").value == "(") {
        children.push_back(new Op(tokenizer, indent));  // (
        children.push_back(new Arguments(tokenizer, indent));
        children.push_back(new Op(tokenizer, indent));  // )
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
            PyObject func_name = children.at(0)->evaluate(stack);
            PyObject arguments = children.at(2)->evaluate(stack);
            PyObject ret = stack.call_function(func_name, arguments);
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
Slices::Slices(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Slices::~Slices() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Slices::parse() {

}
PyObject Slices::evaluate(Stack stack) {
    throw runtime_error("Slices::evaluate() not implemented");
}
ostream& Slices::print(ostream& os) const {

    return os;
}

//===============================================================
// Slice

// slice:
//     | [expression] ':' [expression] [':' [expression] ] 
//     | expression 
Slice::Slice(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Slice::~Slice() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Slice::parse() {

}
PyObject Slice::evaluate(Stack stack) {
    throw runtime_error("Slice::evaluate() not implemented");
}
ostream& Slice::print(ostream& os) const {

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
Atom::Atom(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Atom::~Atom() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Atom::parse() {
    cout << peek("Atom").value.size() << endl;
    if (is_number(peek("Atom").value)) {
        children.push_back(new Number(tokenizer, indent));
    } 
    else if (peek("Atom").value == "True" || peek("Atom").value == "False") {
        children.push_back(new Bool(tokenizer, indent));
    }
    else if (peek("Atom").value == "(") {
        // TODO: figure out a cleaner way to determine if this should be
        // a tuple, group, or genexp
        children.push_back(new Tuple(tokenizer, indent));
    }
    else if (peek("Atom").value == "[") {
        // TODO: figure out a cleaner way to determine if this should be
        // a list or listcomp
        children.push_back(new List(tokenizer, indent));
    }
    else if (peek("Atom").value.at(0) == '"' || peek("Atom").value.at(0) == '\'') {
        children.push_back(new _String(tokenizer, indent));
    }
    else {
        children.push_back(new Name(tokenizer, indent));
    }
    cout << peek("Atom").value.size() << endl;
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
List::List(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
List::~List() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void List::parse() {
    eat_value("[", "List");
    children.push_back(new StarNamedExpressions(tokenizer, indent));
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
    if (children.size() > 0) {
        for (int i=0; i < children.size()-1; i++) {
            os << *(children.at(i)) << ",";
        }
        os << *children.back();
    }
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
Tuple::Tuple(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Tuple::~Tuple() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Tuple::parse() {
    eat_value("(", "Tuple");
    if (peek("Tuple").value != ")") {
        children.push_back(new StarNamedExpression(tokenizer, indent));
        eat_value(",", "Tuple");
    }
    if (peek("Tuple").value != ")") {
        children.push_back(new StarNamedExpressions(tokenizer, indent));
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
    if (children.size() > 0) {
        for (int i=0; i < children.size()-1; i++) {
            os << *(children.at(i)) << ",";
        }
        os << *children.back();
    }
    os << ")";
    return os;
}

//===============================================================
// Arguments

// arguments:
//     | args [','] &')' 
Arguments::Arguments(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Arguments::~Arguments() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Arguments::parse() {
    children.push_back(new Args(tokenizer, indent));
    if (peek("Arguments").value == ",") {
        eat_value(",", "Arguments");
    }
    if (peek("Arguments").value != ")") {
        throw runtime_error("SyntaxError: invalid syntax");
    }
}
PyObject Arguments::evaluate(Stack stack) {
    log("Arguments::evaluate()", DEBUG); add_indent(2);
    children.at(0)->evaluate(stack);
    sub_indent(2);
    return PyObject();
}
ostream& Arguments::print(ostream& os) const {
    os << *children.at(0);
    return os;
}

//===============================================================
// Args

// NOTE: the negative lookahead ( !'=' ) is the indicator to swap to kwargs

// args:
//     | ','.(starred_expression | named_expression !'=')+ [',' kwargs ] 
//     | kwargs 
Args::Args(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Args::~Args() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Args::parse() {
    if (peek("Args").value == ")") return;

    StarredExpression* se;
    NamedExpression* ne;
    while (1) {
        if (peek("Args").value == "*") {
            se = new StarredExpression(tokenizer, indent);
            if (peek("Args").value == "=") {
                delete se;
                break;
            } 
            else { children.push_back(se); }
            if (peek("Args").value == ")") {
                break;
            }
        }
        else {
            ne = new NamedExpression(tokenizer, indent);
            if (peek("Args").value == "=") {
                delete ne;
                break;
            } else { children.push_back(ne); }
            if (peek("Args").value == ")") {
                break;
            }
        }
        if (peek("Args").value == ",") {
            eat_value(",", "Args");
        }
    }
    if (peek("Args").value != ")") {
        children.push_back(new Kwargs(tokenizer, indent));
    }
}
PyObject Args::evaluate(Stack stack) {
    log("Args::evaluate()", DEBUG); add_indent(2);
    vector<PyObject> arguments;
    for (AST* child : children) {
        arguments.push_back(child->evaluate(stack));
    }
    sub_indent(2);
    return PyObject(arguments, "list");
}
ostream& Args::print(ostream& os) const {
    if (children.size() > 0) {
        for (int i=0; i < children.size()-1; i++) {
            os << *children.at(i) << ",";
        }
        os << *children.back();
    }
    return os;
}

//===============================================================
// Kwargs

// kwargs:
//     | ','.kwarg_or_starred+ ',' ','.kwarg_or_double_starred+ 
//     | ','.kwarg_or_starred+
//     | ','.kwarg_or_double_starred+
Kwargs::Kwargs(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
Kwargs::~Kwargs() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void Kwargs::parse() {

}
PyObject Kwargs::evaluate(Stack stack) {
    log("Kwargs::evaluate()", DEBUG); add_indent(2);

    sub_indent(2);
    return PyObject();
}
ostream& Kwargs::print(ostream& os) const {
    if (children.size() > 0) {
        for (int i=0; i < children.size()-1; i++) {
            os << *children.at(i) << ",";
        }
        os << *children.back();
    }
    return os;
}

//===============================================================
// StarredExpression

// starred_expression:
//     | '*' expression
StarredExpression::StarredExpression(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG); add_indent(2);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    sub_indent(2);
    log(__FUNCTION__ + (string)" - children.size() == " + to_string(children.size()), DEBUG);
}
StarredExpression::~StarredExpression() {
    log(__FUNCTION__, DEBUG); add_indent(2);
    for (AST* child : children) delete child;
    children.clear();
    sub_indent(2);
}
void StarredExpression::parse() {
    eat_value("*", "StarredExpression");
    children.push_back(new Expression(tokenizer, indent));
}
PyObject StarredExpression::evaluate(Stack stack) {
    log("StarredExpression::evaluate()", DEBUG); add_indent(2);
    PyObject ret = children.at(0)->evaluate(stack);
    sub_indent(2);
    return PyObject();
}
ostream& StarredExpression::print(ostream& os) const {
    os << "*" << *children.at(0);
    return os;
}

//===============================================================
// Op

Op::Op(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    log(__FUNCTION__ + (string)" - token.value == '" + token.value + "'", DEBUG);
}
Op::~Op() {
    log(__FUNCTION__, DEBUG);
    tokenizer->rewind(1);
}
void Op::parse() {
    this->token = tokenizer->next_token();
}
PyObject Op::evaluate(Stack stack) {
    log("Op::evaluate() - '" + this->token.value + "'", DEBUG);
    return PyObject(this->token.value, "str");
}
ostream& Op::print(ostream& os) const {
    os << token.value;
    return os;
}

//===============================================================
// String

_String::_String(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    log(__FUNCTION__ + (string)" - this->value == '" + this->value + "'", DEBUG);
}
_String::~_String() {
    log(__FUNCTION__, DEBUG);
    tokenizer->rewind(1);
}
void _String::parse() {
    this->token = tokenizer->next_token();
    if (token.value.size() > 0 && token.value.at(0) == '"') {
        this->value = token.value.substr(1, token.value.size()-2);
    } else {
        this->value = token.value;
    }
}
PyObject _String::evaluate(Stack stack) {
    log("_String::evaluate() - '" + this->value + "'", DEBUG);
    return PyObject(this->value, "str");
}
ostream& _String::print(ostream& os) const {
    os << token.value;
    return os;
}

//===============================================================
// Name

Name::Name(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    log(__FUNCTION__ + (string)" - this->value == '" + this->value + "'", DEBUG);
}
Name::~Name() {
    log(__FUNCTION__, DEBUG);
    tokenizer->rewind(1);
}
void Name::parse() {
    this->token = tokenizer->next_token();
    this->value = this->token.value;
}
PyObject Name::evaluate(Stack stack) {
    log("Name::evaluate() - '" + this->value + "'", DEBUG);
    return stack.get_value(this->value);
}
ostream& Name::print(ostream& os) const {
    os << value;
    return os;
}

//===============================================================
// Number

Number::Number(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    log(__FUNCTION__ + (string)" - token.value == '" + token.value + "'", DEBUG);
}
Number::~Number() {
    log(__FUNCTION__, DEBUG);
    tokenizer->rewind(1);
}
void Number::parse() {
    this->token = tokenizer->next_token();
    this->is_int = this->token.value.find(".") == string::npos;
}
PyObject Number::evaluate(Stack stack) {
    log("Number::evaluate() - " + token.value , DEBUG);
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

Bool::Bool(Tokenizer *tokenizer, string indent) {
    log(__FUNCTION__, DEBUG);
    this->tokenizer = tokenizer;
    this->indent = indent;
    parse();
    log(__FUNCTION__ + (string)" - token.value == '" + token.value + "'", DEBUG);
}
Bool::~Bool() {
    log(__FUNCTION__, DEBUG);
    tokenizer->rewind(1);
}
void Bool::parse() {
    this->token = tokenizer->next_token();
    this->bool_value = this->token.value == "True";
}
PyObject Bool::evaluate(Stack stack) {
    log("Bool::evaluate() - " + this->bool_value, DEBUG);
    PyObject res = PyObject(this->bool_value, "bool");
    return res;
}
ostream& Bool::print(ostream& os) const {
    os << token.value;
    return os;
}

//===============================================================
