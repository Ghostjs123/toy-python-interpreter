#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include "builtins.h"
#include "pyobject.h"
using namespace std;


map<string, FnPtr> build_builtins() {
    map<string, FnPtr> builtins;
    builtins["print"] = print;
    return builtins;
}

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

