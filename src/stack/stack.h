#pragma once
#include "stack.fwd.h"
#include "ast.fwd.h"


#ifndef STACK_H
#define STACK_H

#include <vector>
#include <deque>
#include <string>
#include <map>
#include "pyobject.h"
#include "ast.h"
using namespace std;

typedef PyObject (*FnPtr)(PyObject);

class Frame {
    private:
        PyObject return_value;
        bool returning;
    public:
        int id;
        string function_name = "";
        
        PyObject parameters;
        int parameter_idx;

        map<string, FnPtr> builtins;
        map<string, AST*> globals;
        map<string, PyObject> locals;

        Frame();
        Frame(int id, Frame* prev_frame);
        Frame(int id, map<string, AST*> globals, map<string, FnPtr> builtins);
        void set_default_values();

        // functions
        string get_function_name();
        PyObject next_param();

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
        PyObject call_global(AST* functiondef);
        PyObject call_function(PyObject func_name, PyObject arguments);
        string get_function_name();
        PyObject next_param();
        void add_function(AST* function);
        // locals
        void assign(PyObject name, PyObject value);
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
