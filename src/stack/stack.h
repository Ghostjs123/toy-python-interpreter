#pragma once
#include "stack.fwd.h"
#include "ast.fwd.h"


#ifndef STACK_H
#define STACK_H

#include <vector>
#include <string>
#include "frame.h"
using namespace std;

class Stack {
    private:
        vector<Frame> frames;
    public:
        int top;

        Stack();

        PyObject call_function(PyObject func_name, PyObject args);
        Frame current_frame();
        bool push(Frame f);
        Frame pop();
};

#endif
