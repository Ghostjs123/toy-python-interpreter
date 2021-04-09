#ifndef TOKEN_H
#define TOKEN_H

#include <vector>
#include <string>
using namespace std;

class Token {
	public:
		string value;
		int line_num;
		
		Token();
		Token(int line_num, string value);
		Token(int line_num, char value);

		void append(string s);
        friend ostream& operator<<(ostream& os, const Token& token);
		friend bool operator==(const Token& lhs, const Token& rhs);
		friend bool operator!=(const Token& lhs, const Token& rhs);
};

#endif
