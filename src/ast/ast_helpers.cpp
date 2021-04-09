#include <iostream>
#include <string>
#include <vector>
#include "ast_helpers.h"
#include "../objects/pyobject.h"
using namespace std;


bool in(string arr[], string val, size_t N) {
    for (int i=0; i < N; i++) {
        if (arr[i] == val) {
            return true;
        }
    }
    return false;
}

PyObject apply_comparison_op(PyObject left, string op, PyObject right) {
    if (op == "==") {
        return left == right;
    }
    if (op == "!=") {
        return left != right;
    }
    if (op == "<=") {
        return left <= right;
    }
    if (op == "<") {
        return left < right;
    }
    if (op == ">=") {
        return left >= right;
    }
    if (op == ">") {
        return left > right;
    }
    throw runtime_error("apply_comparison_op: \'" + op + "\' not implemented");
}

PyObject apply_shift_op(PyObject left, string op, PyObject right) {
    if (op == "<<") {
        return PyObject((int)left << (int)right, "int");
    }
    if (op == ">>") {
        return PyObject((int)left >> (int)right, "int");
    }
    throw runtime_error("apply_shift_op: \'" + op + "\' not implemented");
}

PyObject apply_sum_op(PyObject left, string op, PyObject right) {
    if (op == "+") {
        return left + right;
    }
    if (op == "-") {
        return left - right;
    }
    throw runtime_error("apply_sum_op: \'" + op + "\' not implemented");
}

PyObject apply_term_op(PyObject left, string op, PyObject right) {
    //     | term '*' factor 
    //     | term '/' factor 
    //     | term '//' factor 
    //     | term '%' factor 
    //     | term '@' factor 
    if (op == "*") {
        return left * right;
    }
    if (op == "/") {
        return left / right;
    }
    // TODO: figure out structure and implement
    // if (op == "//") {
    //     return left.int_div(right);
    // }
    if (op == "%") {
        return left % right;
    }
    // reference: https://www.python.org/dev/peps/pep-0465/
    // NOTE: this is for matrix multiply - way out of scope for this project
    // if (op == "@") {
    //     return left.matmul(right);
    // }
    throw runtime_error("apply_term_op: \'" + op + "\' not implemented");
}

bool is_comparison_op(Tokenizer *tokenizer) {
    // NOTE: to abbrv a long grammar I'm looking for
    // '==', '!=', '<=', '<', '>=', '>', 'not in', 'in', 'is not', 'is'
    string v1 = tokenizer->peek().value;
    string v2 = tokenizer->lookahead(1).value;

    // these will be split into two tokens by the tokenizer
    if (v1 == "not" && v2 == "in" || v1 == "is" && v2 == "not") {
        tokenizer->compound(2, " "); // combines the next two tokens
        return true;
    }

    string v1_opts[] = {"==", "!=", "<=", "<", ">=", ">", "in", "is"};
    if (in(v1_opts, v1, 8)) {
        return true;
    }
    return false;
}

bool is_sum_op(string v1) {
    return v1 == "+" || v1 == "-";
}

bool is_term_op(string v1) {
    return v1 == "*" || v1 == "/" || v1 == "%" || v1 == "@" || v1 == "//";
}

bool is_factor_op(string v1) {
    return v1 == "+" || v1 == "-" || v1 == "~";
}
