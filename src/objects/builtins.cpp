#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <tuple>
#include "builtins.h"
#include "pyobject.h"
using namespace std;


map<string, FnPtr> build_builtins() {
    map<string, FnPtr> builtins;
    builtins["print"] = print;
    return builtins;
}
PyObject print(PyObject arguments) {
    string sep = " ";

    if (arguments.size() > 0) {
        for (int i=0; i < arguments.size()-1; i++) {
            cout << arguments.at(i).as_string() << sep;
        }
        cout << arguments.at(-1).as_string();
    }
    cout << endl;

    return PyObject();  // always returns None
}
