#ifndef PYOBJECT_H
#define PYOBJECT_H

#include <string>
#include <vector>
#include <map>
using namespace std;

class PyObject {
private:
    string s_value;
    int i_value;
    float f_value;
    bool b_value;
    vector<PyObject> li_value;
    map<string, PyObject> dict_value;
    void *class_value;  // TODO: when implementing classes

    bool is_valid_type(string type);
    void check_valid_type();
public:
    string type;

    PyObject();
    PyObject(int i, string type);
    PyObject(float d, string type);
    PyObject(double d, string type);
    PyObject(string s, string type);
    PyObject(bool b, string type);
    PyObject(vector<PyObject> li, string type);
    PyObject(map<string, PyObject> m, string type);

    string as_string() const;
    bool as_bool() const;
    void error_undefined(string op, string t1, string t2) const;
    void error_unsupported_op(string op, string t1, string t2) const;
    void error_unsupported_unary_op(string op, string t1) const;
    void error_unsupported_operand(string op, string t1, string t2) const;

    PyObject operator+(const PyObject& p) const;
    PyObject operator-(const PyObject& p) const;
    PyObject operator*(const PyObject& p) const;
    PyObject operator/(const PyObject& p) const;
    PyObject operator==(const PyObject& p) const;
    PyObject operator!=(const PyObject& p) const;
    PyObject operator<=(const PyObject& p) const;
    PyObject operator<(const PyObject& p) const;
    PyObject operator>=(const PyObject& p) const;
    PyObject operator>(const PyObject& p) const;
    PyObject operator~();
    PyObject _pow(PyObject p);

    friend ostream& operator<<(ostream& os, const PyObject& s);
    
    operator string();
    operator int();
    operator float();
    operator long();
    operator bool();
    operator vector<PyObject>();
    operator map<string, PyObject>();
};


#endif
