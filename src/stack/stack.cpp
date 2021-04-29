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
    Logger::get_instance()->log("Created Frame " + to_string(this->id), INFO);
}

vector<PyObject> Frame::build_args() {
    vector<PyObject> args;
    while (this->arg_stack.size() > 0) {
        args.push_back(arg_stack.back());
        arg_stack.pop_back();
    }
    return args;
}

// TODO: going to need a 2nd one of these that interprets
// the arg_stack with the FunctionDefRaw's Params as well
// to correctly handle defaults
// vector<PyObject> Frame::build_args() {
//     vector<PyObject> args;
    
//     return args;
// }

void Frame::push_arg(PyObject value) {
    Logger::get_instance()->log("Frame " + to_string(this->id) 
                                + " pushed arg '" + (string)value + "'", DEBUG);
    this->arg_stack.push_back(value);
}

void Frame::assign(string name, PyObject value) {
    this->locals.insert(pair<string, PyObject>(name, value));
    Logger::get_instance()->log("Frame " + to_string(this->id) 
                                + (string)": " + name + " = " + (string)value, DEBUG);
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
        Logger::get_instance()->log("Frame " + to_string(this->id) 
                                    + " failed to find Name '" + (string)name + "'", DEBUG);
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

vector<PyObject> Stack::build_args() {
    return this->current_frame()->build_args();
}

PyObject Stack::call_global(AST* functiondef) {
    // need to push a new frame, update the params, then eval the block
    Frame* new_frame = new Frame(next_id(), current_frame());
    push_frame(new_frame);
    FunctionDef* func_t = dynamic_cast<FunctionDef*>(functiondef);
    FunctionDefRaw* raw_t = dynamic_cast<FunctionDefRaw*>(func_t->raw);
    vector<PyObject> args = build_args();
    Logger::get_instance()->log("calling function '" + raw_t->name + "' with args: " + args_as_string(args), DEBUG);
    // for (int i=0; i < raw_t->params.size(); i++) {
    //     cout << raw_t->params.at(i)->evaluate(*this) << endl;
    //     new_frame->assign(raw_t->params.at(i)->evaluate(*this), args.at(i));
    // }
    raw_t->body->evaluate(*this);
    PyObject ret = new_frame->get_return_value();
    pop_frame();
    Logger::get_instance()->log("function '" + raw_t->name + "' returning: " + ret.as_string(), DEBUG);
    return ret;
}

PyObject Stack::call_function(PyObject func_name) {
    Logger::get_instance()->log("searching for function: '" + (string)func_name + "'", DEBUG);
    tuple<bool, PyObject> res;
    Frame* cf = current_frame();

    // check/call builtin
    map<string, FnPtr>::iterator b_it = cf->builtins.find((string)func_name);
    if (b_it != cf->builtins.end()) {
        Logger::get_instance()->log("calling builtin: '" + (string)func_name + "'", DEBUG);
        PyObject ret = b_it->second(build_args());
        Logger::get_instance()->log("function '" + (string)func_name + "' returning: " + ret.as_string(), DEBUG);
        return ret;
    }
    
    // check/call global
    map<string, AST*>::iterator g_it = cf->globals.find((string)func_name);
    if (g_it != cf->globals.end()) {
        return call_global(g_it->second);
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

void Stack::push_arg(PyObject value) {
    this->current_frame()->push_arg(value);
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
