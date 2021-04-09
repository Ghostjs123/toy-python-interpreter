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

string &ltrim(string &s) {
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
