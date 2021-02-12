#include <iostream>
#include <vector>
#include <string>
using namespace std;

// https://www.learncpp.com/cpp-tutorial/printing-inherited-classes-using-operator/
// learning how virtual functions and operator definitions work so I can plan out my AST Nodes

class AST {
    public:
        vector<AST *> children;

        virtual ostream& print(ostream& out) const {
            out << "AST";
            return out;
        }
        friend ostream& operator<<(ostream &out, const AST &b) {
            return b.print(out);
        }
};

class Atom: public AST {
    public:
        int value;

        Atom(int value) {
            this->value = value;
        }
        virtual ostream& print(ostream& out) const override {
            out << "Atom: " << value;
            return out;
        }
};

class Sum: public AST {
    public:
        Sum() {
            children.push_back(new Atom(123));
            children.push_back(new Atom(456));
        }
        ~Sum() {
            for (int i=0; i < children.size(); i++) {
                delete children[i];
            }
        }
        virtual ostream& print(ostream& out) const override {
            for (AST *child : children) {
                out << *child;
            }
            return out;
        }
};

int main() {
	Sum s{};
	cout << s << endl;

	return 0;
}
