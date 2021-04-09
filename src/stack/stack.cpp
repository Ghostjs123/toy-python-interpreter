#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include "frame.h"
#include "stack.h"
#include "builtins.h"
using namespace std;
    
#define MAX_FRAMES 100


Stack::Stack() {
    this->top = 0;
    frames.push_back(Frame(frames.size()));
}

PyObject Stack::call_function(PyObject func_name, PyObject args) {
    tuple<bool, PyObject> res;
    Frame cf = this->current_frame();

    // check/call builtin
    res = call_builtin(cf.builtins, func_name, args);
    if (get<0>(res)) {
        // function was a builtin
        return get<1>(res);
    }

    // TODO: check global namespace for the function

    
    cout << "name \'" << (string)func_name << "\' is not defined" << endl;
    throw;
}

Frame Stack::current_frame() {
    return this->frames.back();
}

bool Stack::push(Frame f) {
    top++;
    if (top >= MAX_FRAMES) {
        cout << "Stack Overflow (" << MAX_FRAMES << ")" << endl;
        return false;
    }
    frames.push_back(f);
    return true;
}

Frame Stack::pop() {
    top--;
    if (top < 0) {
        cout << "Stack Underflow" << endl;
        throw;
    }
    Frame b = frames.back();
    frames.pop_back();
    return b;
}
