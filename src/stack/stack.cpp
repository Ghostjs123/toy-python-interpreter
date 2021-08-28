#include <iostream>
#include <vector>
#include <deque>
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

Frame::Frame() {
    this->id = id;
    this->builtins = build_builtins();
    this->set_default_values();
    Logger::get_instance()->log("Created Frame with id: " + to_string(this->id), INFO);
}

Frame::Frame(int id, Frame* prev_frame) {
    this->id = id;
    this->globals = map<string, AST*>(prev_frame->globals);
    this->builtins = map<string, FnPtr>(prev_frame->builtins);
    this->set_default_values();
    Logger::get_instance()->log("Created Frame with id: " + to_string(this->id), INFO);
}

Frame::Frame(int id, map<string, AST*> globals, map<string, FnPtr> builtins) {
    this->id = id;
    this->globals = map<string, AST*>(globals);
    this->builtins = map<string, FnPtr>(builtins);
    this->set_default_values();
    Logger::get_instance()->log("Created Frame " + to_string(this->id), INFO);
}

void Frame::set_default_values() {
    this->returning = false;
    this->return_value = PyObject();  // None
    this->parameters = PyObject();  // None
    this->parameter_idx = 0;
}

string Frame::get_function_name() {
    return this->function_name;
}

PyObject Frame::next_param() {
    PyObject next_p = this->parameters.at(this->parameter_idx);
    this->parameter_idx++;

    return next_p;
}

void Frame::assign(string name, PyObject value) {
    this->locals.insert(pair<string, PyObject>(name, value));
    Logger::get_instance()->log(
        "Frame " + to_string(this->id) + (string)": " + name + " = " + (string)value,
        DEBUG
    );
}

PyObject Frame::get_value(string name) {
    // check globals / builtins - if its a function then return its name
    map<string, FnPtr>::iterator b_it = this->builtins.find(name);
    map<string, AST*>::iterator g_it = this->globals.find(name);
    if (b_it == this->builtins.end() && g_it == this->globals.end()) {
        // not in globals / builtins
        map<string, PyObject>::iterator locals_it = this->locals.find(name);

        if (locals_it != this->locals.end()) {
            return locals_it->second;
        }
        Logger::get_instance()->log(
            "Frame " + to_string(this->id) + " failed to find Name '" + (string)name + "'",
            DEBUG
        );
        throw runtime_error("NameError: name '" + (string)name + "' is not defined");
    }
    else {
        return PyObject(name, "str");
    }
}

void Frame::set_return_value(PyObject value) {
    this->return_value = value;
    this->returning = true;
    Logger::get_instance()->log("Set return value to: " + (string)value, DEBUG);
}

PyObject Frame::get_return_value() {
    return return_value;
}

bool Frame::is_returning() {
    return this->returning;
}


//==========================================================

// TODO: Notes for later
// print("a: ", a())
// this is not working, each Frame needs its own arg_stack bc atm
// the a() call is thrashing print()'s arg_stack

Stack::Stack() {
    this->frames.push_back(new Frame());
    Logger::get_instance()->log("Created the Stack", INFO);
}

void Stack::clean() {
    while (this->frames.size() > 0) this->pop_frame();
}

PyObject Stack::call_global(AST* functiondef) {
    // need to push a new frame, update the params, then eval the block
    Frame* new_frame = new Frame(next_id(), current_frame());
    push_frame(new_frame);
    FunctionDefRaw* raw = dynamic_cast<FunctionDefRaw*>(dynamic_cast<FunctionDef*>(functiondef)->raw);
    Logger::get_instance()->log("calling function '" + raw->name + "'", DEBUG);
    new_frame->function_name = raw->name;
    raw->params->evaluate(*this);
    raw->body->evaluate(*this);
    PyObject ret = new_frame->get_return_value();
    pop_frame();
    Logger::get_instance()->log("function '" + raw->name + "' returning: " + ret.as_string(), DEBUG);
    return ret;
}

PyObject Stack::call_function(PyObject func_name, PyObject arguments) {
    Logger::get_instance()->log("searching for function: '" + (string)func_name + "'", DEBUG);
    Frame* current_f = current_frame();

    // check/call builtin
    map<string, FnPtr>::iterator builtin_it = current_f->builtins.find((string)func_name);

    if (builtin_it != current_f->builtins.end()) {
        Logger::get_instance()->log("calling builtin: '" + (string)func_name + "'", DEBUG);
        PyObject ret = builtin_it->second(arguments);
        Logger::get_instance()->log("function '" + (string)func_name + "' returning: " + ret.as_string(), DEBUG);

        return ret;
    }
    
    // check/call global
    map<string, AST*>::iterator g_it = current_f->globals.find((string)func_name);

    if (g_it != current_f->globals.end()) {
        return call_global(g_it->second);
    }

    throw runtime_error("stack - function \'" + (string)func_name + "\' is not defined");
}

string Stack::get_function_name() {
    return this->current_frame()->get_function_name();
}

PyObject Stack::next_param() {
    return this->current_frame()->next_param();
}

void Stack::add_function(AST* function) {
    Frame* cf = current_frame();
    FunctionDef* function_t = dynamic_cast<FunctionDef*>(function);
    FunctionDefRaw* raw_t = dynamic_cast<FunctionDefRaw*>(function_t->raw);
    cf->globals.insert(pair<string, AST*>(raw_t->name, function_t));
    Logger::get_instance()->log("Added function '" + raw_t->name + "' to globals", INFO);
}

void Stack::assign(PyObject name, PyObject value) {
    this->current_frame()->assign(name, value);
}

PyObject Stack::get_value(string name) {
    return this->current_frame()->get_value(name);
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
    Logger::get_instance()->log("Popped Frame with id: " + to_string(b->id), INFO);
    delete b;
}
