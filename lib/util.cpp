#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <algorithm>
#include <vector>
#include <string>
using namespace std;

bool is_number(const string& s) {
    try {
        stod(s);
        return true;
    }
    catch(...) {
        return false;
    }
}

string pad_string(string str, int size, char c=' ') {
    if (str.size() > size) {
        string msg = "ERROR: attempted to pad \'" 
             + str + "\' of length " + to_string(str.size()) 
             + " to a length of " + to_string(size);
        throw runtime_error(msg);
    }
    str.insert(0, size - str.size(), c);
    return str;
}

// template <class T>
// bool vector_contains(vector<T> vec, T x) {
//     return find(vec.begin(), vec.end(), x) != vec.end();
// }

string ltrim(string s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](int c) { return !isspace(c); }));
    return s;
}

string read_file(string fname) {
    ifstream ifs(fname);
    string contents( (istreambuf_iterator<char>(ifs) ),
                     (istreambuf_iterator<char>()    ) );
    return contents;
}

vector<string> read_lines(string fname) {
    vector<string> results;

    fstream file;
    file.open(fname, ios::in);
    if (file.is_open()) {
        string tp;
        while(getline(file, tp)) {
            results.push_back(tp);
        }
    }

    return results;
}

bool file_exists(string fname) {
    struct stat buffer;
    return (stat (fname.c_str(), &buffer) == 0);
}
