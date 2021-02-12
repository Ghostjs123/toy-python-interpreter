#include <iostream>
#include <string>
using namespace std;

// testing how similar c++ ops are to python ops
// if they function the same then I am safe to use them for PyObjects

string string_mul(string s, int amt) {
    string new_s = "";
    for (int i=0; i < amt; i++) {
        new_s += s;
    }
    return new_s;
}

int main() {
    cout << "testing string ops" << endl;
    cout << "result : expected" << endl;
    bool b = "Apple" <= "ApplePie";
    cout << b << " : 1" << endl;
    b = "aaa" < "ab";
    cout << b << " : 1" << endl;
    string s1 = "abc";
    // s1 - s1 * 4;  // NOTE: not implemented
    s1 = string_mul(s1, 4);
    cout << s1 << " : abcabcabcabc" << endl;

    cout << "\ntesting bool ops" << endl;
    cout << "result : expected" << endl;
    bool b1 = true;
    bool b2 = true;
    b = b1 <= b2;  // True <= True
    cout << b << " : 1" << endl;
    cout << b1 + b2 << " : 2" << endl;
    b2 = false;
    cout << b1 + b2 << " : 1" << endl;
    b1 = false;
    cout << b1 + b2 << " : 0" << endl;
    b1 = true;
    b1 = ~b1;
    cout << b1 << " : 1" << endl;

    cout << "\ntesting double ops" << endl;
    cout << "result : expected" << endl;
    double d1 = 10;
    bool b3 = true;
    cout << d1 + b3 << " : 11" << endl;
    int i1 = 10;
    bool b4 = i1 == d1;
    cout << b4 << " : 1" << endl;
    double d2 = 10;

    return 0;
}