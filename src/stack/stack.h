#pragma once
#include "stack.fwd.h"
#include "ast.fwd.h"


#ifndef STACK_H
#define STACK_H

#include <vector>
#include <string>
#include <map>
#include "pyobject.h"
#include "ast.h"
using namespace std;

typedef PyObject (*FnPtr)(vector<PyObject>);

class Frame {
    private:
        PyObject return_value;
        bool returning;
    public:
        int id;

        map<string, FnPtr> builtins;
        map<string, AST*> globals;
        map<string, PyObject> locals;
        vector<PyObject> arg_stack;

        Frame(int id);
        Frame(int id, Frame* prev_frame);
        Frame(int id, map<string, AST*> globals, map<string, FnPtr> builtins);

        // functions
        void push_arg(PyObject value);
        vector<PyObject> build_args();

        // locals
        void assign(string name, PyObject value);
        PyObject get_value(string name);

        // for ReturnStmt
        PyObject get_return_value();
        void set_return_value(PyObject value);
        bool is_returning();
};

class Stack {
    private:
        vector<Frame*> frames;
    public:
        Stack();

        // === ast management ===
        // functions
        vector<PyObject> build_args();
        PyObject call_function(PyObject func_name);
        PyObject call_global(AST* functiondef);
        void add_function(AST* function);
        void push_arg(PyObject value);
        // locals
        PyObject get_value(string name);
        // for ReturnStmt
        void set_return_value(PyObject value);
        bool is_returning();

        // === frame management ===
        void clean();
        Frame* current_frame();
        int next_id();
        void push_frame(Frame* f);
        void pop_frame();
};

#endif
