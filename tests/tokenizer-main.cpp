#include <iostream>
#include <string>
#include <array>
#include <memory>
#include <stdio.h>
#include <sstream>
#include <algorithm>
#include "token.h"
#include "tokenizer.h"
#include "util.h"
using namespace std;

// https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
string execute_python_tokenizer(const char* cmd) {
	array<char, 128> buffer;
	string result;
	unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
	if (!pipe) {
		throw runtime_error("popen() failed");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}
	return result;
}

vector<int> as_char_codes(string line) {
	vector<int> codes;
	for (char c : line) {
		codes.push_back(+c);
	}
	return codes;
}

vector<string> split_newline(char* result) {
	vector<string> lines;
	stringstream ss(result);
	string to;
	if (result != NULL) {
		while (getline(ss, to, '\n')) {
			lines.push_back(to);
		}
	}
	return lines;
}

int codes_as_int(vector<int> codes) {
	string temp = "";
	for (int i : codes) {
		temp += char(i);
	}
	return stoi(temp);
}

tuple<int, int, int, int> parse_position(vector<int> codes) {
	// ex: 1,0-1,2     except char codes
	vector<int>::iterator iter;
	iter = find(codes.begin(), codes.end(), +',');
	int first_comma = distance(codes.begin(), iter);
	iter = find(codes.begin(), codes.end(), +'-');
	int dash = distance(codes.begin(), iter);
	iter = find(codes.begin()+dash, codes.end(), +',');
	int second_comma = distance(codes.begin(), iter);

	tuple<int, int, int, int> result;
	// line start
	get<0>(result) = codes_as_int(vector<int>(codes.begin(), codes.begin()+first_comma));
	// column start
	get<1>(result) = codes_as_int(vector<int>(codes.begin()+first_comma+1, codes.begin()+dash));
	// line end
	get<2>(result) = codes_as_int(vector<int>(codes.begin()+dash+1, codes.begin()+second_comma));
	// column end
	get<3>(result) = codes_as_int(vector<int>(codes.begin()+second_comma+1, codes.end()));
	return result;
}

bool not_space(int i) {
	return i != +' ';
}

string codes_as_string(vector<int> codes) {
	string result = "";
	for (int i : codes) {
		result += char(i);
	}
	return result;
}

vector<Token> parse_tokens(string result) {
	vector<Token> python_tokens;
	vector<string> lines = split_newline(&result[0]);

	int colon = +':';
	int apostrophe = +'\'';
	int space = +' ';

	vector<int> codes;
	vector<int>::iterator iter;

	for (string s : lines) {
		codes = as_char_codes(s);
		
		// determine the position of the token
		iter = find(codes.begin(), codes.end(), colon);
		int pos_end = distance(codes.begin(), iter);
		tuple<int, int, int, int> position = parse_position(vector<int>(codes.begin(), codes.begin()+pos_end));
		tuple<int, int> pos1(get<0>(position), get<1>(position));
		tuple<int, int> pos2(get<2>(position), get<3>(position));

		// determine the type of the token
		iter = find_if(codes.begin()+pos_end+1, codes.end(), not_space);
		int type_start = distance(codes.begin(), iter);
		iter = find(codes.begin()+type_start, codes.end(), space);
		int type_end = distance(codes.begin(), iter);
		string type = codes_as_string(vector<int>(codes.begin()+type_start, codes.begin()+type_end));

		// determine the value of the token
		iter = find(codes.begin()+type_end, codes.end(), apostrophe);
		int value_start = distance(codes.begin(), iter);
		iter = find(codes.begin()+value_start+1, codes.end(), apostrophe);
		int value_end = distance(codes.begin(), iter);
		string value = codes_as_string(vector<int>(codes.begin()+value_start+1, codes.begin()+value_end));;

		// add token
		python_tokens.push_back(Token(type, value, pos1, pos2));
	}
	return python_tokens;
}

bool compare(string fname, Tokenizer tokenizer) {
	string cmd = "python3 -m tokenize " + fname;
	string result = execute_python_tokenizer(cmd.c_str());
	vector<Token> python_tokens = parse_tokens(result);
	python_tokens.erase(python_tokens.begin());  // delete encoding token

	Token t = python_tokens.at(0);  // ignore initial value
	for (int i=0; i < python_tokens.size(); i++) {
		if (python_tokens.at(i) != (t = tokenizer.next_token())) {
			cout << endl << "Tokens did not match on line: " << t.line_start << " column: " << t.column_start << endl;
			cout << "mine:    " << t << endl;
			cout << "theirs:  " << python_tokens.at(i) << endl;
			return false;
		}
	}
	return true;
}

int main(int argc, char* argv[]) {
	if (argc == 1) {
		cout << "Missing input filename\n";
		return 0;
	}
	if (!file_exists(argv[1])) {
		cout << "No file named \"" << argv[1] << "\"" << endl;
		return 0;
	}

	vector<string> contents = read_lines(argv[1]);

	Tokenizer tokenizer(contents);

    cout << "tokens:" << endl;
	tokenizer.prettyprint();

	if (argc == 3 && argv[2] == (string)"-c") {
		bool match = compare(argv[1], tokenizer);
		if (match) {
			cout << endl << "Tokens match Python" << endl;
		} else {
			cout << endl << "Tokens DID NOT match Python" << endl;
		}
	}

	return 0;
}