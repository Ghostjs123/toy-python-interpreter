#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include "stack.h"


PyObject interpret(Statements tree, Stack& stack);

#endif