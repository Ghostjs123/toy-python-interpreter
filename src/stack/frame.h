#ifndef FRAME_H
#define FRAME_H

#include <vector>
#include <string>
#include <map>
#include "ast.h"
#include "pyobject.h"
#include "builtins.h"
using namespace std;

class Frame {
    private:
        int id;
        vector<Frame> frames;
    public:
        map<string, FnPtr> builtins;
        map<string, PyObject> globals;
        map<string, PyObject> locals;

        Frame(int id);
        Frame(int id, map<string, PyObject> globals, map<string, FnPtr> builtins);
};

#endif
