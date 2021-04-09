#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
using namespace std;

bool is_number(const string& s);

string pad_string(string str, int size, char c=' ');

// template <class T>
// bool vector_contains(vector<T> vec, T x);

string ltrim(string s);

string read_file(string fname);
vector<string> read_lines(string fname);
bool file_exists(string fname);

#endif
