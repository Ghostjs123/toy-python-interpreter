#ifndef TOKEN_H
#define TOKEN_H

#include <vector>
#include <string>
#include <tuple>
using namespace std;

class Token {
	public:
		string type, value;
		int line_start, line_end, column_start, column_end;
		
		Token();
		Token(string type, char value, tuple<int, int> start, tuple<int, int> end);
		Token(string type, string value, tuple<int, int> start, tuple<int, int> end);

		void append(string s, Token t);
        friend ostream& operator<<(ostream& os, const Token& token);
		friend bool operator==(const Token& lhs, const Token& rhs);
		friend bool operator!=(const Token& lhs, const Token& rhs);

    	operator string();
};

#endif
