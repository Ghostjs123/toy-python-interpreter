#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include "../src/objects/pyobject.h"
using namespace std;

// initial testing of PyObject class for multivariate returns

class String {
public:
    struct PyObject value;

    String(string value) {
        this->value = PyObject(value, "str");
    }

    PyObject evaluate() {
        return value;
    }

    friend ostream& operator<<(ostream& os, String& s) {
        os << s.value;
        return os;
    }
};

class Number {
public:
    struct PyObject value;

    Number(string value) {
        float d_value = stof(value);
        this->value = PyObject(d_value, "double");
    }

    PyObject evaluate() {
        return value;
    }

    friend ostream& operator<<(ostream& os, Number& n) {
        os << n.value;
        return os;
    }
};

int main() {
    cout << "test:" << endl;
    // testing interactions with AST nodes
    Number n("1.0");
    String s("asd");
    cout << "n: " << n.evaluate() << endl;
    cout << "s: " << s.evaluate() << endl;

    // testing PyObject addition (double + string)
    PyObject res = n.evaluate() + s.evaluate();
    cout << "n + s: " << res << endl;

    // testing implicit cast to bool
    PyObject p(true, "bool");
    cout << "p: " << p << endl;
    bool b = p;
    cout << "b: " << b << endl;

    return 0;
}
