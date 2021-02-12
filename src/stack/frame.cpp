#include <vector>
#include <string>
#include <map>
#include "ast.h"
#include "frame.h"
#include "pyobject.h"
#include "builtins.h"
using namespace std;

Frame::Frame(int id) {
    this->id = id;
    this->builtins = build_builtins();
}

Frame::Frame(int id, map<string, PyObject> globals, map<string, FnPtr> builtins) {
    this->id = id;
    this->globals = globals;
    this->builtins = builtins;
}
