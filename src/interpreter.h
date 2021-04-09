#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "stack.h"


PyObject interpret(Expressions tree, Stack& stack);

#endif