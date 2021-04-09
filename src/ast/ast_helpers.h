#ifndef AST_HELPERS_H
#define AST_HELPERS_H

#include <string>
#include <vector>
#include "tokenizer.h"
#include "../objects/pyobject.h"
using namespace std;

bool in(string arr[], string val, size_t N);

// functions for accumulate
auto _boolean_or = [](bool b1, bool b2){ return b1 || b2; };
auto _boolean_and = [](bool b1, bool b2){ return b1 && b2; };
auto _bitwise_or = [](int b1, int b2){ return b1 | b2; };
auto _bitwise_xor = [](int b1, int b2){ return b1 ^ b2; };
auto _bitwise_and = [](int b1, int b2){ return b1 & b2; };

PyObject apply_comparison_op(PyObject left, string op, PyObject right);
PyObject apply_shift_op(PyObject left, string op, PyObject right);
PyObject apply_sum_op(PyObject left, string op, PyObject right);
PyObject apply_term_op(PyObject left, string op, PyObject right);

bool is_comparison_op(Tokenizer *tokenizer);
bool is_sum_op(string v1);
bool is_term_op(string v1);
bool is_factor_op(string v1);

#endif