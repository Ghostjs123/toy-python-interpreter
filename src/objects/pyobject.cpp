#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <map>
#include "pyobject.h"
#include "ast.h"
using namespace std;

// TODO: add sets

// ==============================================================
// helper functions

string string_mul(string s, int amt) {
    string new_s = "";
    for (int i=0; i < amt; i++) {
        new_s += s;
    }
    return new_s;
}

string trim_trailing_zeros(string s) {
    s = s.substr(0, s.find_last_not_of('0')+1);
    if(s.find('.') == s.size()-1) {
        s += '0';
    }
    return s;
}

// ==============================================================
// constructors

// valid types:
// None, str, int, float, bool, list, dict, tuple, class, function

PyObject::PyObject() {
    this->type = "None";
    this->check_valid_type();
}
PyObject::PyObject(int i, string type) {
    // if (type == "bool") {
    if (type != "int") {
        // this->b_value = i == 1;
        cout << "WARNING: recieved \'int\' of type \'" << type << "\'" << endl;
    }
    else {
        this->i_value = i;
    }
    this->type = type;
    this->check_valid_type();
}
PyObject::PyObject(float d, string type) {
    // if (type == "bool") {
    if (type != "float") {
        // this->b_value = d == 1;
        cout << "WARNING: recieved \'float\' of type \'" << type << "\'" << endl;
    }
    else {
        this->f_value = d;
    }
    this->type = type;
    this->check_valid_type();
}
PyObject::PyObject(double d, string type) {
    // if (type == "bool") {
    if (type != "float") {
        // this->b_value = d == 1;
        cout << "WARNING: recieved \'float\' of type \'" << type << "\'" << endl;
    }
    else {
        this->f_value = d;
    }
    this->type = type;
    this->check_valid_type();
}
PyObject::PyObject(string s, string type) {
    this->s_value = s;
    this->type = type;
    this->check_valid_type();
}
PyObject::PyObject(bool b, string type) {
    this->b_value = b;
    this->type = type;
    this->check_valid_type();
}
// lists
PyObject::PyObject(vector<PyObject> li, string type) {
    this->li_value = li;
    this->type = type;
    this->check_valid_type();
}
// dicts
PyObject::PyObject(map<string, PyObject> m, string type) {
    this->dict_value = m;
    this->type = type;
    this->check_valid_type();
}
PyObject::PyObject(AST* function, string type) {
    this->func_value = function;
    this->type = type;
    this->check_valid_type();
}

// sets - TODO:


// ==============================================================
// misc class functions

bool PyObject::is_valid_type(string type) {
    return type == "None" || type == "str" ||
           type == "int" || type == "float" ||
           type == "bool" || type == "list" ||
           type == "dict" || type == "tuple" ||
           type == "class" || type == "function";
}

// NOTE: this method is mostly a spelling check atm
void PyObject::check_valid_type() {
    if (!this->is_valid_type(this->type)) {
        throw runtime_error("Invalid type for PyObject: \'" + this->type + "\'");
    }
}

// returns value as string
string PyObject::as_string() const {
    if (this->type == "str") {
        return this->s_value;
    } 
    else if (this->type == "float") {
        return trim_trailing_zeros(to_string(this->f_value));
    }
    else if (this->type == "bool") {
        return this->b_value ? "True" : "False";
    }
    else if (this->type == "int") {
        return to_string(this->i_value);
    }
    else if (this->type == "list") {
        string s = "[";
        if (this->li_value.size() == 1) {
            s += this->li_value.front().as_string();
        } else {
            for (int i=0; i < this->li_value.size()-1; i++) {
                s += this->li_value.at(i).as_string() + ", ";
            }
            s += this->li_value.back().as_string();
        }
        s += "]";
        return s;
    }
    else if (this->type == "None") {
        return "None";
    }
    else if (this->type == "tuple") {
        string s = "(";
        if (this->li_value.size() == 1) {
            s += this->li_value.front().as_string() + ",";
        } else {
            for (int i=0; i < this->li_value.size()-1; i++) {
                s += this->li_value.at(i).as_string() + ", ";
            }
            s += this->li_value.back().as_string();
        }
        s += ")";
        return s;
    }
    // TODO: handle list and dict to strings
    throw runtime_error("as_string() not defined for type " + this->type);
}

// returns value as bool
bool PyObject::as_bool() const {
    if (this->type == "bool") {
        return this->b_value;
    }
    if (this->type == "str") {
        return this->s_value != "";
    }
    if (this->type == "int") {
        return this->i_value != 0;
    }
    if (this->type == "float") {
        return this->f_value != 0;
    }
    if (this->type == "list") {
        return this->li_value.size() != 0;
    }
    throw runtime_error("as_bool() not defined for type " + this->type);
}

vector<PyObject> PyObject::as_list() const {
    if (this->type == "tuple" || this->type == "list" || this->type == "set") {
        return this->li_value;
    }
    throw runtime_error("as_list() called on PyObject of type: \'" + this->type + "\'");
}

AST* PyObject::get_function() const {
    if (this->type == "function") {
        return this->func_value;
    }
    throw runtime_error("get_function() called on PyObject of type: \'" + this->type + "\'");
}

int PyObject::size() const {
    if (this->type == "bool") {
        throw runtime_error("TypeError: object of type 'bool' has no len()");
    }
    if (this->type == "str") {
        return this->s_value.size();
    }
    if (this->type == "int") {
        throw runtime_error("TypeError: object of type 'int' has no len()");
    }
    if (this->type == "float") {
        throw runtime_error("TypeError: object of type 'float' has no len()");
    }
    if (this->type == "list") {
        return this->li_value.size();
    }
    throw runtime_error("size() not defined for type " + this->type);
}

PyObject PyObject::at(int i) const {
    if (this->type == "bool") {
        throw runtime_error("TypeError: 'bool' object is not subscriptable");
    }
    if (this->type == "str") {
        return PyObject(this->s_value.at(i), "str");
    }
    if (this->type == "int") {
        throw runtime_error("TypeError: 'int' object is not subscriptable");
    }
    if (this->type == "float") {
        throw runtime_error("TypeError: 'float' object is not subscriptable");
    }
    if (this->type == "list") {
        return PyObject(this->li_value.at(i));
    }
    throw runtime_error("at() not defined for type " + this->type);
}

// error function for undefined ops
void PyObject::error_undefined(string op, string t1, string t2) const {
    throw runtime_error("undefined op \'" + op + "\' for \'" + t1 
         + "\' and \'" + t2 + "\' - needs implemented");
}

// error function for unsupported ops
void PyObject::error_unsupported_op(string op, string t1, string t2) const {
    throw runtime_error("\'" + op + "\' not supported between instances of \'" 
            + t1 + "\' and \'" + t2 + "\'");
}

// error function for unsupported unary ops
void PyObject::error_unsupported_unary_op(string op, string t1) const {
    throw runtime_error("bad operand type for unary " + op + ": \'" + t1 + "\'");
}

// error function for unsupported operands
void PyObject::error_unsupported_operand(string op, string t1, string t2) const {
    // ex: unsupported operand type(s) for -: 'str' and 'str'
    throw runtime_error("unsupported operand type(s) for " + op + 
            ": \'" + t1 + "\' and \'" + t2 + "\'");
}

// ==============================================================
// operator definitions

// references:
// strings:
// https://www.journaldev.com/23511/python-string-comparison
// NOTE: most of these operations can be tested in a python interpreter
// additionally, a lot of ops work the same in c++ as in python

// TODO: this very likely needs a massive refactor. I think I can get away
// with using implicit casting and overloaded instructors to no longer have to
// pass the type param around everywhere. If I indeed can do that, then a lot of
// these functions become a couple lines and I can stop checking type in most cases.
// Edge cases will still exist, ex: int-div //
// If I don't get a ton of warnings from the if statements on the int and float
// constructors then I'm prob fine to delete a ton of stuff

PyObject PyObject::operator+(const PyObject& p) const {
    // if either are strings then concat as a new string
    if (this->type == "str" || p.type == "str") {
        string s = this->as_string() + p.as_string();
        return PyObject(s, "str");
    }
    if (this->type == "float") {
        if (p.type == "int") {
            // float+int
            return PyObject(this->f_value + p.i_value, "float");
        }
        if (p.type == "bool") {
            // float+bool
            return PyObject(this->f_value + p.b_value, "float");
        }
        // float+float
        return PyObject(this->f_value + p.f_value, "float");
    }
    if (this->type == "int") {
        if (p.type == "float") {
            // int+float
            return PyObject(this->i_value + p.f_value, "float");
        }
        if (p.type == "bool") {
            // int+bool
            return PyObject(this->i_value + p.b_value, "int");
        }
        // int+int
        return PyObject(this->i_value + p.i_value, "int");
    }
    if (this->type == "bool") {
        if (p.type == "int") {
            // bool+int
            return PyObject(this->b_value + p.i_value, "int");
        }
        if (p.type == "float") {
            // bool+float
            return PyObject(this->b_value + p.f_value, "float");
        }
        // bool+bool
        return PyObject(this->b_value + p.b_value, "int");
    }
    this->error_undefined("+", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator-(const PyObject& p) const {
    // string-(any) or (any)-string
    if (this->type == "str" || p.type == "str") {
        this->error_unsupported_operand("-", this->type, p.type);
    }
    if (this->type == "float") {
        if (p.type == "int") {
            // float-int
            return PyObject(this->f_value - p.i_value, "float");
        }
        if (p.type == "bool") {
            // float-bool
            return PyObject(this->f_value - p.b_value, "float");
        }
        // float-float
        return PyObject(this->f_value - p.f_value, "float");
    }
    if (this->type == "bool") {
        if (p.type == "int") {
            // bool-int
            return PyObject(this->b_value - p.i_value, "int");
        }
        if (p.type == "float") {
            // bool-float
            return PyObject(this->b_value - p.f_value, "float");
        }
        // bool-bool
        return PyObject(this->b_value - p.b_value, "int");
    }
    if (this->type == "int") {
        if (p.type == "float") {
            // int-float
            return PyObject(this->i_value - p.f_value, "float");
        }
        if (p.type == "bool") {
            // int-bool
            return PyObject(this->i_value - p.b_value, "int");
        }
        // int-int
        return PyObject(this->i_value - p.i_value, "int");
    }
    this->error_undefined("-", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator*(const PyObject& p) const {
    // string*string
    if (this->type == "str" && p.type == "str") {
        throw runtime_error("can't multiply sequence by non-int of type " + p.type);
    }
    // string*float or float*string
    if ( (this->type == "str" && p.type == "float") ||
         (this->type == "float" && p.type == "str") ) {
        string s;
        int amt;
        if (this->type == "float") {
            s = p.s_value;
            amt = this->f_value;
        }
        else {
            s = this->s_value;
            amt = p.f_value;
        }
        return PyObject(string_mul(s, amt), "str");
    }
    if (this->type == "float") {
        if (p.type == "int") {
            // float*int
            return PyObject(this->f_value * p.i_value, "float");
        }
        if (p.type == "bool") {
            // float*bool
            return PyObject(this->f_value * p.b_value, "float");
        }
        // float*float
        return PyObject(this->f_value * p.f_value, "float");
    }
    if (this->type == "bool") {
        if (p.type == "int") {
            // bool*int
            return PyObject(this->b_value * p.i_value, "int");
        }
        if (p.type == "float") {
            // bool*float
            return PyObject(this->b_value * p.f_value, "float");
        }
        // bool*bool
        return PyObject(this->b_value * p.b_value, "bool");
    }
    if (this->type == "int") {
        if (p.type == "float") {
            // int*float
            return PyObject(this->i_value * p.f_value, "float");
        }
        if (p.type == "bool") {
            // int*bool
            return PyObject(this->i_value * p.b_value, "int");
        }
        // int*int
        return PyObject(this->i_value * p.i_value, "int");
    }
    this->error_undefined("*", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator/(const PyObject& p) const {
    // string/(any) or (any)/string
    if (this->type == "str" || p.type == "str") {
        this->error_unsupported_operand("/", this->type, p.type);
    }
    if (this->type == "float") {
        if (p.type == "int") {
            // float/int
            return PyObject(this->f_value / p.i_value, "float");
        }
        if (p.type == "bool") {
            // float/bool
            return PyObject(this->f_value / p.b_value, "float");
        }
        // float/float
        return PyObject(this->f_value / p.f_value, "float");
    }
    if (this->type == "bool") {
        if (p.type == "int") {
            // bool/int
            return PyObject(this->b_value / p.i_value, "float");
        }
        if (p.type == "float") {
            // bool/float
            return PyObject(this->b_value / p.f_value, "float");
        }
        // bool/bool
        return PyObject(this->b_value / p.b_value, "float");
    }
    if (this->type == "int") {
        if (p.type == "float") {
            // int/float
            return PyObject(this->i_value / p.f_value, "float");
        }
        if (p.type == "bool") {
            // int/bool
            return PyObject(this->i_value / p.b_value, "float");
        }
        // int/int
        return PyObject(this->i_value / p.i_value, "float");
    }
    this->error_undefined("/", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator%(const PyObject& p) const {
    // NOTE/TODO: % works like a format string
    // >>> "%s %s" % ("Hello", "World")
    // 'Hello World'

    if (this->type == "float") {
        if (p.type == "int") {
            // float%int
            return PyObject(fmod(this->f_value, p.i_value), "float");
        }
        if (p.type == "bool") {
            // float%bool
            return PyObject(fmod(this->f_value,  p.b_value), "float");
        }
        // float%float
        return PyObject(fmod(this->f_value, p.f_value), "float");
    }
    if (this->type == "bool") {
        if (p.type == "int") {
            // bool%int
            return PyObject((int)(this->b_value % p.i_value), "int");
        }
        if (p.type == "float") {
            // bool%float
            return PyObject(fmod(this->b_value, p.f_value), "float");
        }
        // bool%bool
        return PyObject((int)(this->b_value % p.b_value), "int");
    }
    if (this->type == "int") {
        if (p.type == "float") {
            // int%float
            return PyObject(fmod(this->i_value, p.f_value), "float");
        }
        if (p.type == "bool") {
            // int%bool
            return PyObject((int)(this->i_value % p.b_value), "int");
        }
        // int%int
        return PyObject((int)(this->i_value % p.i_value), "int");
    }
    this->error_undefined("%", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator==(const PyObject& p) const {
    if (this->type != p.type) {
        return PyObject(false, "bool");
    }
    if (this->type == "str") {
        return PyObject(this->s_value == p.s_value, "bool");
    }
    if (this->type == "float" || this->type == "bool") {
        return PyObject(this->as_bool() == p.as_bool(), "bool");
    }
    if (this->type == "int") {
        return PyObject(this->i_value == p.i_value, "bool");
    }
    if (this->type == "None") {
        return PyObject(this->type == p.type, "bool");
    }
    // TODO: implement list and dict

    this->error_undefined("==", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator!=(const PyObject& p) const {
    if (this->type != p.type) {
        return PyObject(true, "bool");
    }
    if (this->type == "str") {
        return PyObject(this->s_value != p.s_value, "bool");
    }
    if (this->type == "float" || this->type == "bool") {
        return PyObject(this->as_bool() != p.as_bool(), "bool");
    }
    if (this->type == "int") {
        return PyObject(this->i_value != p.i_value, "bool");
    }
    if (this->type == "None") {
        return PyObject(this->type != p.type, "bool");
    }
    // TODO: implement list and dict

    this->error_undefined("!=", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator<=(const PyObject& p) const {
    if ( (this->type == "str" && p.type != "str") || 
         (this->type != "str" && p.type == "str") ) {
        this->error_unsupported_op("<=", this->type, p.type);
    }
    // NOTE: checking p's type should be redundant after first if
    if (this->type == "str" && p.type == "str") {  
        return PyObject(this->s_value <= p.s_value, "bool");
    }
    if (this->type == "float" || this->type == "bool") {
        return PyObject(this->as_bool() <= p.as_bool(), "bool");
    }
    if (this->type == "int") {
        return PyObject(this->i_value <= p.i_value, "bool");
    }
    // TODO: implement list and dict

    this->error_undefined("<=", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator<(const PyObject& p) const {
    if ( (this->type == "str" && p.type != "str") || 
         (this->type != "str" && p.type == "str") ) {
        this->error_unsupported_op("<", this->type, p.type);
    }
    // NOTE: checking p's type should be redundant after first if
    if (this->type == "str" && p.type == "str") {  
        return PyObject(this->s_value < p.s_value, "bool");
    }
    if (this->type == "float" || this->type == "bool") {
        return PyObject(this->as_bool() < p.as_bool(), "bool");
    }
    if (this->type == "int") {
        return PyObject(this->i_value < p.i_value, "bool");
    }
    // TODO: implement list and dict
    
    this->error_undefined("<", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator>=(const PyObject& p) const {
    if ( (this->type == "str" && p.type != "str") || 
         (this->type != "str" && p.type == "str") ) {
        this->error_unsupported_op(">=", this->type, p.type);
    }
    // NOTE: checking p's type should be redundant after first if
    if (this->type == "str" && p.type == "str") {  
        return PyObject(this->s_value >= p.s_value, "bool");
    }
    if (this->type == "float" || this->type == "bool") {
        return PyObject(this->as_bool() >= p.as_bool(), "bool");
    }
    if (this->type == "int") {
        return PyObject(this->i_value >= p.i_value, "bool");
    }
    // TODO: implement list and dict
    
    this->error_undefined(">=", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator>(const PyObject& p) const {
    if ( (this->type == "str" && p.type != "str") || 
         (this->type != "str" && p.type == "str") ) {
        this->error_unsupported_op(">", this->type, p.type);
    }
    // NOTE: checking p's type should be redundant after first if
    if (this->type == "str" && p.type == "str") {  
        return PyObject(this->s_value > p.s_value, "bool");
    }
    if (this->type == "float" || this->type == "bool") {
        return PyObject(this->as_bool() > p.as_bool(), "bool");
    }
    if (this->type == "int") {
        return PyObject(this->i_value > p.i_value, "bool");
    }
    // TODO: implement list and dict
    
    this->error_undefined(">", this->type, p.type);
    return PyObject();
}

PyObject PyObject::operator~() {
    if (this->type != "int") {
        this->error_unsupported_unary_op("~", this->type);
    }
    return PyObject(~((int)(*this)), "int");
}

PyObject PyObject::_pow(PyObject p) {
    if (this->type == "int" && p.type == "int") {
        return PyObject((int)pow((int)*this, (int)p), "int");
    }
    return PyObject((float)pow((float)*this, (float)p), "float"); 
}

ostream& operator<<(ostream& os, const PyObject& s) {
    os << s.as_string() << "(" << s.type << ")";
    return os;
}

// ==============================================================
// implicit casting

PyObject::operator string() {
    return this->as_string();
}

PyObject::operator int() {
    if (this->type == "float") {
        return (int)(this->f_value);
    }
    if (this->type == "bool") {
        return this->b_value ? 1 : 0;
    }
    return this->i_value;
}

PyObject::operator float() {
    if (this->type == "int") {
        return this->i_value;
    }
    if (this->type == "bool") {
        return this->b_value ? 1.0 : 0.0;
    }
    return this->f_value;
}

PyObject::operator long() {
    if (this->type == "int") {
        return this->i_value;
    }
    if (this->type == "bool") {
        return this->b_value ? 1.0 : 0.0;
    }
    return (long)(this->f_value);
}

PyObject::operator bool() {
    return this->as_bool();
}

PyObject::operator vector<PyObject>() {
    return this->li_value;
}

PyObject::operator map<string, PyObject>() {
    return this->dict_value;
}
