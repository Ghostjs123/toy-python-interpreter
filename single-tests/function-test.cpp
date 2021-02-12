#include <iostream>
#include <string>
#include <vector>
#include <map>
#include "../src/objects/pyobject.h"
using namespace std;

// User defined functions are fairly standard since I have an AST
// Node for them. I am testing function pointers in a map here.

PyObject print(vector<PyObject> p) {
    if (p.size() > 0) {
        for (int i=0; i < p.size()-1; i++) {
            cout << p.at(i).as_string() << " ";
        }
        cout << p.back().as_string();
    }
    cout << endl;
    return PyObject();  // always returns None
}

typedef PyObject (*FnPtr)(vector<PyObject>);

bool call_builtin(map<string, FnPtr> builtins, string fname, vector<PyObject> args) {
    try {
        FnPtr x = builtins.at(fname);
        x(args);
        return true;
    }
    catch (out_of_range& e) {
        return false;
    }
}

int main() {
    map<string, FnPtr> builtins;
    builtins["print"] = print;

    string fname = "print";

    vector<PyObject> args;
    args.push_back(PyObject(1, "int"));
    args.push_back(PyObject((string)"2", "str"));
    args.push_back(PyObject((string)"3", "str"));
    args.push_back(PyObject((string)"4", "str"));

    if (call_builtin(builtins, fname, args)) {
        cout << "called builtin function \'" << fname << "\'" << endl; 
    } else {
        cout << "failed to call builtin function \'" << fname << "\'" << endl; 
    }

    return 0;
}