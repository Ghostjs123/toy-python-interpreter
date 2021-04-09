#ifndef BUILTINS_H
#define BUILTINS_H

#include <string>
#include <vector>
#include <map>
#include <tuple>
#include "pyobject.h"
using namespace std;

typedef PyObject (*FnPtr)(vector<PyObject>);

map<string, FnPtr> build_builtins();
tuple<bool, PyObject> call_builtin(map<string, FnPtr> builtins, string fname, vector<PyObject> args);
PyObject print(vector<PyObject> p);


#endif