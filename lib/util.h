#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
using namespace std;

bool is_number(const string& s);
string &ltrim(string &s);
string read_file(string fname);
vector<string> read_lines(string fname);
bool file_exists(string fname);

#endif
