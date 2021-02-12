// https://docs.python.org/3/reference/grammar.html
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <numeric>
#include "tokenizer.h"
#include "token.h"
#include "util.h"
#include "ast.h"
#include "ast_helpers.h"
#include "../objects/pyobject.h"
#include "stack.h"
using namespace std;

#define VERBOSE 0
#define INTERPRETER_DEBUG 1


// AST: parent class of all nodes
AST::AST() { }
AST::AST(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
}
Token AST::peek() { 
    return (*tokenizer).peek(); 
}
Token AST::lookahead(int amt) { 
    return (*tokenizer).lookahead(amt);
}
Token AST::next_token() { 
    return (*tokenizer).next_token();
}
void AST::eat(string exp_value) {
    Token next = next_token();
    if (next.value != exp_value) {
        cout << "ERROR: ate " << next.value << " expected " << exp_value << endl;
        exit(EXIT_FAILURE);
    }
}
PyObject AST::evaluate(Stack stack) {
    cout << "Attempted to evaluate an AST - start evaluation at a subclass" << endl;
    throw;
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

Op::Op(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    token = next_token();
}
PyObject Op::evaluate(Stack stack) {
    return PyObject(token.value, "str");
}
ostream& Op::print(ostream& os) const {
    os << token.value;
    return os;
}

String::String(Token token) {
    this->token = token;
    parse();
    if (VERBOSE) cout << "Created String: " << token.value << endl;
}
String::String(Tokenizer *tokenizer) {
    this->token = tokenizer->next_token();
    parse();
    if (VERBOSE) cout << "Created String: " << token.value << endl;
}
void String::parse() {
    if (token.value.size() > 0 && token.value.at(0) == '"') {
        this->value = token.value.substr(1, token.value.size()-2);
    } else {
        this->value = token.value;
    }
}
PyObject String::evaluate(Stack stack) {
    return PyObject(this->value, "str");
}
ostream& String::print(ostream& os) const {
    os << token.value;
    return os;
}

Number::Number(Token token) {
    this->token = token;
    this->is_int = token.value.find(".") == string::npos;
    if (VERBOSE) cout << "Created Number: " << token.value << endl;
}
Number::Number(Tokenizer *tokenizer) {
    this->token = tokenizer->next_token();
    if (VERBOSE) cout << "Created Number: " << token.value << endl;
}
PyObject Number::evaluate(Stack stack) {
    if (this->is_int) {
        return PyObject(stoi(token.value), "int");
    }
    return PyObject(stof(token.value), "float");
}
ostream& Number::print(ostream& os) const {
    os << token.value;
    return os;
}

// if_stmt:
//     | 'if' named_expression ':' block elif_stmt 
//     | 'if' named_expression ':' block [else_block] 
IfStmt::IfStmt(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void IfStmt::parse() {

}
PyObject IfStmt::evaluate(Stack stack) {
    if (INTERPRETER_DEBUG) {
        cout << "IfStmt::evaluate : " << children.size() << endl;
    }
    return PyObject(); // returns None
}
ostream& IfStmt::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// elif_stmt:
//     | 'elif' named_expression ':' block elif_stmt 
//     | 'elif' named_expression ':' block [else_block] 
ElifStmt::ElifStmt(Tokenizer *tokenizer) {

}
void ElifStmt::parse() {

}
PyObject ElifStmt::evaluate(Stack stack) {
    if (INTERPRETER_DEBUG) {
        cout << "ElifStmt::evaluate : " << children.size() << endl;
    }
    return PyObject(); // returns None
}
ostream& ElifStmt::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// else_block: 'else' ':' block 
ElseBlock::ElseBlock(Tokenizer *tokenizer) {

}
void ElseBlock::parse() {

}
PyObject ElseBlock::evaluate(Stack stack) {
    if (INTERPRETER_DEBUG) {
        cout << "ElseBlock::evaluate : " << children.size() << endl;
    }
    return PyObject(); // returns None
}
ostream& ElseBlock::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// for_stmt:
//     | 'for' star_targets 'in' ~ star_expressions ':' [TYPE_COMMENT] block [else_block] 
//     | ASYNC 'for' star_targets 'in' ~ star_expressions ':' [TYPE_COMMENT] block [else_block]
ForStmt::ForStmt(Tokenizer *tokenizer) {
    // Loop better: a deeper look at iteration in Python
    // https://www.youtube.com/watch?v=V2PkkMS2Ack
}
void ForStmt::parse() {

}
PyObject ForStmt::evaluate(Stack stack) {
    if (INTERPRETER_DEBUG) {
        cout << "ForStmt::evaluate : " << children.size() << endl;
    }
    return PyObject(); // returns None
}
ostream& ForStmt::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// named_expression:
//     | NAME ':=' ~ expression 
//     | expression !':='
NamedExpression::NamedExpression(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void NamedExpression::parse() {

}
PyObject NamedExpression::evaluate(Stack stack) {
    if (INTERPRETER_DEBUG) {
        cout << "NamedExpression::evaluate : " << children.size() << endl;
    }
    return PyObject(); // returns None
}
ostream& NamedExpression::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// expressions:
//     | expression (',' expression )+ [','] 
//     | expression ',' 
//     | expression
Expressions::Expressions(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Expressions::parse() {

}
PyObject Expressions::evaluate(Stack stack) {
    if (INTERPRETER_DEBUG) {
        cout << "Expressions::evaluate : " << children.size() << endl;
    }
    return PyObject(); // returns None
}
ostream& Expressions::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// expression:
//     | disjunction 'if' disjunction 'else' expression 
//     | disjunction
//     | lambdef
Expression::Expression(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Expression::parse() {
    // NOTE: skipping lambdef
    if (lookahead(1).value == "if") {

    }
    children.push_back(new Disjunction(tokenizer));
}
PyObject Expression::evaluate(Stack stack) {
    // TODO: implement case (1)
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
    }
    cout << "ERROR: reached end of Expression::evaluate() without returning" << endl;
    throw;
}
ostream& Expression::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// disjunction:
//     | conjunction ('or' conjunction )+ 
//     | conjunction
Disjunction::Disjunction(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Disjunction::parse() {
    children.push_back(new Conjunction(tokenizer));
    while(peek().value == "or") {
        eat("or");
        children.push_back(new Conjunction(tokenizer));
    }
}
PyObject Disjunction::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
    }
    vector<bool> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    bool result = accumulate(
        results.begin(), results.end(), results.at(0), _boolean_or);
    return PyObject(result, "bool");
}
ostream& Disjunction::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// conjunction:
//     | inversion ('and' inversion )+ 
//     | inversion
Conjunction::Conjunction(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Conjunction::parse() {
    children.push_back(new Inversion(tokenizer));
    while(peek().value == "and") {
        eat("and");
        children.push_back(new Inversion(tokenizer));
    }
}
PyObject Conjunction::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
    }
    vector<bool> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    bool result = accumulate(
        results.begin(), results.end(), results.at(0), _boolean_and);
    return PyObject(result, "bool");
}
ostream& Conjunction::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// inversion:
//     | 'not' inversion 
//     | comparison
Inversion::Inversion(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Inversion::parse() {
    if (peek().value == "not") {
        children.push_back(new String(tokenizer));
    }
    children.push_back(new Comparison(tokenizer));
}
PyObject Inversion::evaluate(Stack stack) {
    PyObject s = children.at(0)->evaluate(stack);
    if (s.type == "str" && s.as_string() == "not") {
        bool b = children.at(1)->evaluate(stack);
        return PyObject(!b, "bool");
    }
    return s;
}
ostream& Inversion::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
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
    this->tokenizer = tokenizer;
    parse();
}
void Comparison::parse() {
    children.push_back(new BitwiseOr(tokenizer));
    while (is_comparison_op(tokenizer)) {
        children.push_back(new Op(tokenizer));
        children.push_back(new BitwiseOr(tokenizer));
    }
}
PyObject Comparison::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
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
    return results.front();
}
ostream& Comparison::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// bitwise_or:
//     | bitwise_or '|' bitwise_xor 
//     | bitwise_xor
BitwiseOr::BitwiseOr(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void BitwiseOr::parse() {
    children.push_back(new BitwiseXor(tokenizer));
    while (peek().value == "|") {
        eat("|");
        children.push_back(new BitwiseXor(tokenizer));
    }
}
PyObject BitwiseOr::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
    }
    vector<int> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    int result = accumulate(
        results.begin(), results.end(), results.at(0), _bitwise_or);
    return PyObject(result, "int");
}
ostream& BitwiseOr::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// bitwise_xor:
//     | bitwise_xor '^' bitwise_and 
//     | bitwise_and
BitwiseXor::BitwiseXor(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void BitwiseXor::parse() {
    children.push_back(new BitwiseAnd(tokenizer));
    while (peek().value == "^") {
        eat("^");
        children.push_back(new BitwiseAnd(tokenizer));
    }
}
PyObject BitwiseXor::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
    }
    vector<int> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    int result = accumulate(
        results.begin(), results.end(), results.at(0), _bitwise_xor);
    return PyObject(result, "int");
}
ostream& BitwiseXor::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// bitwise_and:
//     | bitwise_and '&' shift_expr 
//     | shift_expr
BitwiseAnd::BitwiseAnd(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void BitwiseAnd::parse() {
    children.push_back(new ShiftExpr(tokenizer));
    while (peek().value == "&") {
        eat("&");
        children.push_back(new ShiftExpr(tokenizer));
    }
}
PyObject BitwiseAnd::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
    }
    vector<int> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    int result = accumulate(
        results.begin(), results.end(), results.at(0), _bitwise_and);
    return PyObject(result, "int");
}
ostream& BitwiseAnd::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// shift_expr:
//     | shift_expr '<<' sum 
//     | shift_expr '>>' sum 
//     | sum
ShiftExpr::ShiftExpr(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void ShiftExpr::parse() {
    children.push_back(new Sum(tokenizer));
    while (peek().value == "<<" || peek().value == ">>") {
        children.push_back(new Op(tokenizer));
        children.push_back(new Sum(tokenizer));
    }
}
PyObject ShiftExpr::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
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
    return results.front();
}
ostream& ShiftExpr::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// sum:
//     | sum '+' term 
//     | sum '-' term 
//     | term
Sum::Sum(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Sum::parse() {
    children.push_back(new Term(tokenizer));
    while(is_sum_op(peek().value)) {
        children.push_back(new Op(tokenizer));
        children.push_back(new Term(tokenizer));
    }
}
PyObject Sum::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
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
    return results.front();
}
ostream& Sum::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// term:
//     | term '*' factor 
//     | term '/' factor 
//     | term '//' factor 
//     | term '%' factor 
//     | term '@' factor 
//     | factor
Term::Term(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Term::parse() {
    children.push_back(new Factor(tokenizer));
    if (VERBOSE) cout << "Term: " << *children.back() << endl;
    while(is_term_op(peek().value)) {
        children.push_back(new Op(tokenizer));
        if (VERBOSE) cout << "Term: " << *children.back() << endl;
        children.push_back(new Factor(tokenizer));
        if (VERBOSE) cout << "Term: " << *children.back() << endl;
    }
}
PyObject Term::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
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
    return results.front();
}
ostream& Term::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// factor:
//     | '+' factor 
//     | '-' factor 
//     | '~' factor 
//     | power
Factor::Factor(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Factor::parse() {
    if (is_factor_op(peek().value)) {
        if (peek().value == "+") {
            next_token();  // the '+' doesn't really do anything
        }
        children.push_back(new Op(tokenizer));
    }
    children.push_back(new Power(tokenizer));
}
PyObject Factor::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
    }
    string op = children.at(0)->evaluate(stack);
    PyObject val = children.at(1)->evaluate(stack);
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
            cout << "Unsupported PyObject of type \'" 
                 << val.type << "\' in Factor::evaluate()" << endl;
            throw;
        }
    }
    if (op == "~") {
        return PyObject((int)(~val), "int");
    }
    cout << "ERROR: reached end of Factor::evaluate() without returning" << endl;
    throw;
}
ostream& Factor::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// power:
//     | await_primary '**' factor 
//     | await_primary
Power::Power(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Power::parse() {
    children.push_back(new AwaitPrimary(tokenizer));
    if (peek().value == "**") {
        eat("**");
        children.push_back(new Factor(tokenizer));
    }
}
PyObject Power::evaluate(Stack stack) {
    if (children.size() == 1){
        return children.at(0)->evaluate(stack);
    }
    return children.at(0)->evaluate(stack)._pow(children.at(1)->evaluate(stack));
}
ostream& Power::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// await_primary:
//     | AWAIT primary 
//     | primary
AwaitPrimary::AwaitPrimary(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void AwaitPrimary::parse() {
    // TODO: implement this completely
    children.push_back(new Primary(tokenizer));
}
PyObject AwaitPrimary::evaluate(Stack stack) {
    return children.at(0)->evaluate(stack);
}
ostream& AwaitPrimary::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// primary:
//     | invalid_primary  # must be before 'primay genexp' because of invalid_genexp
//     | primary '.' NAME 
//     | primary genexp 
//     | primary '(' [arguments] ')' 
//     | primary '[' slices ']' 
//     | atom
Primary::Primary(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Primary::parse() {
    // NOTE: this is supposed to be a left recusive production
    // im keeping it simle atm but its likely that a while loop will
    // be needed like Term, Sum, etc
    children.push_back(new Atom(tokenizer));
    if (VERBOSE) cout << "Primary: " << *children.back() << endl;
    if (peek().value == ".") {
        // TODO: implement
    } else if(peek().value == "(") {
        children.push_back(new String(tokenizer));
        children.push_back(new Arguments(tokenizer));
        children.push_back(new String(tokenizer));
    }
    if (VERBOSE) cout << "Primary: " << *children.back() << endl;
}
PyObject Primary::evaluate(Stack stack) {
    if (children.size() == 1) {
        return children.at(0)->evaluate(stack);
    }
    else {
        PyObject c1 = children.at(1)->evaluate(stack);
        if ((string)c1 == "(") {
            return stack.call_function(
                children.at(0)->evaluate(stack), children.at(2)->evaluate(stack));
        }
    }
    cout << "ERROR: reached end of Primary::evaluate() without returning" << endl;
    throw;
}
ostream& Primary::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// slices:
//     | slice !',' 
//     | ','.slice+ [','] 
Slices::Slices(Tokenizer *tokenizer) {

}
void Slices::parse() {

}
PyObject Slices::evaluate(Stack stack) {
    cout << "Slices::evaluate() not implemented" << endl;
    throw;
}
ostream& Slices::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// slice:
//     | [expression] ':' [expression] [':' [expression] ] 
//     | expression 
Slice::Slice(Tokenizer *tokenizer) {

}
void Slice::parse() {

}
PyObject Slice::evaluate(Stack stack) {
    cout << "Slice::evaluate() not implemented" << endl;
    throw;
}
ostream& Slice::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
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
    this->tokenizer = tokenizer;
    parse();
}
void Atom::parse() {
    Token token = next_token();
    if (is_number(token.value)) {
        children.push_back(new Number(token));
        if (VERBOSE) cout << "Atom: " << *children.back() << endl;
    } else {
        children.push_back(new String(token));
        if (VERBOSE) cout << "Atom: " << *children.back() << endl;
    }
}
PyObject Atom::evaluate(Stack stack) {
    return children.at(0)->evaluate(stack);
}
ostream& Atom::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
// arguments:
//     | args [','] &')' 
Arguments::Arguments(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Arguments::parse() {
    children.push_back(new Args(tokenizer));
    while (peek().value == ",") {
        eat(",");
        children.push_back(new Args(tokenizer));
    }
}
PyObject Arguments::evaluate(Stack stack) {
    vector<PyObject> results;
    for (AST *child : children) {
        results.push_back(child->evaluate(stack));
    }
    return PyObject(results, "list");
}
ostream& Arguments::print(ostream& os) const {
    for (int i=0; i < children.size()-1; i++) {
        os << *(children[i]) << ",";
    }
    os << *children.back();
    return os;
}
// args:
//     | ','.(starred_expression | named_expression !'=')+ [',' kwargs ] 
//     | kwargs 
Args::Args(Tokenizer *tokenizer) {
    this->tokenizer = tokenizer;
    parse();
}
void Args::parse() {
    //TODO: args are really complicated, im simplifying them for now
    children.push_back(new Expression(tokenizer));
}
PyObject Args::evaluate(Stack stack) {
    if (children.size() != 1) {
        cout << "ERROR: Args::evaluate() called with children of size: " 
             << children.size() << endl;
        throw;
    }
    return children.at(0)->evaluate(stack);
}
ostream& Args::print(ostream& os) const {
    for (AST *child : children) {
        os << *child;
    }
    return os;
}
