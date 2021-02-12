#include <iostream>
#include <string>
#include <vector>
using namespace std;

// NOTE: this file is just for testing random lines of c++ code

int main() {

    string num_s1 = "1";
    string num_s2 = "1.";
    string num_s3 = "1.0";
    float num_f1 = stof(num_s1);
    float num_f2 = stof(num_s2);
    float num_f3 = stof(num_s3);
    cout << "num_f1: " << num_f1 << endl;
    cout << "num_f2: " << num_f2 << endl;
    cout << "num_f3: " << num_f3 << endl;

    return 0;
}
