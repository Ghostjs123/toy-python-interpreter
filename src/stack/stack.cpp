#include <iostream>
#include <vector>
#include <string>
#include <tuple>
#include <map>
#include <string>
#include "logging.h"
#include "stack.h"
#include "builtins.h"
#include "ast.h"
#include "pyobject.h"
using namespace std;
    
#define MAX_FRAMES 100


//==========================================================

Frame::Frame(int id) {
    this->id = id;
    this->builtins = build_builtins();
    Logger::get_instance()->log("Created Frame with id: " + to_string(this->id), INFO);
}

Frame::Frame(int id, Frame* prev_frame) {
    this->id = id;
    this->globals = map<string, AST*>(prev_frame->globals);
    this->builtins = map<string, FnPtr>(prev_frame->builtins);
    Logger::get_instance()->log("Created Frame with id: " + to_string(this->id), INFO);
}

Frame::Frame(int id, map<string, AST*> globals, map<string, FnPtr> builtins) {
    this->id = id;
    this->globals = map<string, AST*>(globals);
    this->builtins = map<string, FnPtr>(builtins);
    Logger::get_instance()->log("Created Frame with id: " + to_string(this->id), INFO);
}

void Frame::assign(string name, PyObject value) {
    this->locals.insert(pair<string, PyObject>(name, value));
}


//==========================================================

string args_as_string(vector<PyObject> args) {
    if (args.size() == 0) return "()";
    if (args.size() == 1) return "(" + args.at(0).as_string() + ")";
    string s = "(";
    for (int i=0; i < args.size()-1; i++) {
        s += args.at(i).as_string() + ", ";
    }
    s += args.back().as_string() + ")";
    return s;
}

Stack::Stack() {
    this->top = 0;
    this->frames.push_back(new Frame(frames.size()));
    Logger::get_instance()->log("Created the Stack", INFO);
}

PyObject Stack::call_global(AST* function, vector<PyObject> args) {
    // need to push a new frame, update the params, then eval the block
    Frame* new_frame = new Frame(next_id(), current_frame());
    push(new_frame);
    FunctionDef* func_t = dynamic_cast<FunctionDef*>(function);
    FunctionDefRaw* raw_t = dynamic_cast<FunctionDefRaw*>(func_t->raw);
    Logger::get_instance()->log("calling function '" + raw_t->name + "' with args: " + args_as_string(args), DEBUG);
    for (int i=0; i < raw_t->params.size(); i++) {
        new_frame->assign(raw_t->params.at(i)->evaluate(*this), args.at(i));
    }
    PyObject ret = raw_t->body->evaluate(*this);
    pop();
    Logger::get_instance()->log("function '" + raw_t->name + "' returning: " + ret.as_string(), DEBUG);
    return ret;
}

PyObject Stack::call_function(PyObject func_name, PyObject args) {
    tuple<bool, PyObject> res;
    Frame* cf = current_frame();

    // check/call builtin
    res = call_builtin(cf->builtins, func_name, args);
    if (get<0>(res)) {
        // function was a builtin - return value
        return get<1>(res);
    }

    // map<string, AST*>::iterator _it = this->globals.begin();
    // for (_it=this->globals.begin(); _it!=this->globals.end(); ++_it)
    //     cout << _it->first << " => " << _it->second << '\n';
    

    map<string, AST*>::iterator it = cf->globals.find((string)func_name);
    if (it != cf->globals.end()) {
        return call_global(it->second, args);
    }

    throw runtime_error("stack - function \'" + (string)func_name + "\' is not defined");
}

Frame* Stack::current_frame() {
    return this->frames.back();
}

int Stack::next_id() {
    return this->frames.size();
}

void Stack::add_function(AST* function) {
    Frame* cf = current_frame();
    FunctionDef* function_t = dynamic_cast<FunctionDef*>(function);
    FunctionDefRaw* raw_t = dynamic_cast<FunctionDefRaw*>(function_t->raw);
    cf->globals.insert(pair<string, AST*>(raw_t->name, function_t));
    Logger::get_instance()->log("Added function '" + raw_t->name + "' to globals", INFO);
}

void Stack::push(Frame* f) {
    top++;
    if (top >= MAX_FRAMES) {
        throw runtime_error("Stack Overflow (" + to_string(MAX_FRAMES) + ")");
    }
    frames.push_back(f);
}

void Stack::pop() {
    top--;
    if (top < 0) {
        throw runtime_error("Stack Underflow");
    }
    Frame* b = frames.back();
    frames.pop_back();
    free(b);
    Logger::get_instance()->log("Popped Frame with id: " + to_string(b->id), INFO);
}
