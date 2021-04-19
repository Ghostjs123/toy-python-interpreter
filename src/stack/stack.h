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

        Frame(int id);
        Frame(int id, Frame* prev_frame);
        Frame(int id, map<string, AST*> globals, map<string, FnPtr> builtins);

        void assign(string name, PyObject value);
        void set_return_value(PyObject value);
        PyObject get_return_value();
        bool is_returning();
};

class Stack {
    private:
        vector<Frame*> frames;
    public:
        Stack();

        // ast management
        PyObject call_function(PyObject func_name, PyObject args);
        PyObject call_global(AST* function, vector<PyObject> args);
        void add_function(AST* function);
        void set_return_value(PyObject value);
        bool is_returning();

        // frame management
        void clean();
        Frame* current_frame();
        int next_id();
        void push_frame(Frame* f);
        void pop_frame();
};

#endif
