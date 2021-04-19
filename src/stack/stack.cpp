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
    this->returning = false;
    this->return_value = PyObject();  // None
    Logger::get_instance()->log("Created Frame with id: " + to_string(this->id), INFO);
}

Frame::Frame(int id, Frame* prev_frame) {
    this->id = id;
    this->globals = map<string, AST*>(prev_frame->globals);
    this->builtins = map<string, FnPtr>(prev_frame->builtins);
    this->returning = false;
    this->return_value = PyObject();  // None
    Logger::get_instance()->log("Created Frame with id: " + to_string(this->id), INFO);
}

Frame::Frame(int id, map<string, AST*> globals, map<string, FnPtr> builtins) {
    this->id = id;
    this->globals = map<string, AST*>(globals);
    this->builtins = map<string, FnPtr>(builtins);
    this->returning = false;
    this->return_value = PyObject();  // None
    Logger::get_instance()->log("Created Frame with id: " + to_string(this->id), INFO);
}

void Frame::assign(string name, PyObject value) {
    this->locals.insert(pair<string, PyObject>(name, value));
}

void Frame::set_return_value(PyObject value) {
    this->return_value = value;
    Logger::get_instance()->log("Set return value to: " + (string)value, DEBUG);
}

PyObject Frame::get_return_value() {
    return return_value;
}

bool Frame::is_returning() {
    return this->returning;
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
    this->frames.push_back(new Frame(frames.size()));
    Logger::get_instance()->log("Created the Stack", INFO);
}

void Stack::clean() {
    while (this->frames.size() > 0) this->pop_frame();
}

PyObject Stack::call_global(AST* function, vector<PyObject> args) {
    // need to push a new frame, update the params, then eval the block
    Frame* new_frame = new Frame(next_id(), current_frame());
    push_frame(new_frame);
    FunctionDef* func_t = dynamic_cast<FunctionDef*>(function);
    FunctionDefRaw* raw_t = dynamic_cast<FunctionDefRaw*>(func_t->raw);
    Logger::get_instance()->log("calling function '" + raw_t->name + "' with args: " + args_as_string(args), DEBUG);
    for (int i=0; i < raw_t->params.size(); i++) {
        new_frame->assign(raw_t->params.at(i)->evaluate(*this), args.at(i));
    }
    raw_t->body->evaluate(*this);
    PyObject ret = new_frame->get_return_value();
    pop_frame();
    Logger::get_instance()->log("function '" + raw_t->name + "' returning: " + ret.as_string(), DEBUG);
    return ret;
}

PyObject Stack::call_function(PyObject func_name, PyObject args) {
    tuple<bool, PyObject> res;
    Frame* cf = current_frame();

    // check/call builtin
    map<string, FnPtr>::iterator b_it = cf->builtins.find((string)func_name);
    if (b_it != cf->builtins.end()) {
        return b_it->second(args);
    }
    
    // check/call global
    map<string, AST*>::iterator g_it = cf->globals.find((string)func_name);
    if (g_it != cf->globals.end()) {
        return call_global(g_it->second, args);
    }

    throw runtime_error("stack - function \'" + (string)func_name + "\' is not defined");
}

void Stack::add_function(AST* function) {
    Frame* cf = current_frame();
    FunctionDef* function_t = dynamic_cast<FunctionDef*>(function);
    FunctionDefRaw* raw_t = dynamic_cast<FunctionDefRaw*>(function_t->raw);
    cf->globals.insert(pair<string, AST*>(raw_t->name, function_t));
    Logger::get_instance()->log("Added function '" + raw_t->name + "' to globals", INFO);
}

void Stack::set_return_value(PyObject value) {
    this->current_frame()->set_return_value(value);
}

bool Stack::is_returning(){
    return this->current_frame()->is_returning();
}

Frame* Stack::current_frame() {
    return this->frames.back();
}

int Stack::next_id() {
    return this->frames.size();
}

void Stack::push_frame(Frame* f) {
    if (this->frames.size() >= MAX_FRAMES) {
        throw runtime_error("Stack Overflow (" + to_string(MAX_FRAMES) + ")");
    }
    frames.push_back(f);
}

void Stack::pop_frame() {
    if (this->frames.size() == 0) {
        throw runtime_error("Stack Underflow");
    }
    Frame* b = frames.back();
    frames.pop_back();
    free(b);
    Logger::get_instance()->log("Popped Frame with id: " + to_string(b->id), INFO);
}
