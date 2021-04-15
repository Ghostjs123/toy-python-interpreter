#include <iostream>
#include <sstream>
#include <streambuf>
#include <vector>
#include <string>
#include <tuple>
#include <curses.h>
#include "logging.h"
#include "ast.h"
#include "parser.h"
#include "tokenizer.h"
#include "stack.h"
#include "util.h"
using namespace std;


void init_ncurses() {
	initscr();
	raw();
	noecho();
	keypad(stdscr, TRUE);
}

void decorate(int& y, int& x) {
	x = 0;
	move(y, x);
	printw(">>> ");
	refresh();
	x = 4;
}

void cursor_left(int& y, int& x, string& s) {
	if (x > 4) {
		x--;
		move(y, x);
		refresh();
	}
}

void cursor_right(int& y, int& x, string& s) {
	if (x < s.size()+4) {
		x++;
		move(y, x);
		refresh();
	}
}

void delete_left(int& y, int& x, string& s) {
	if (x > 4) {
		s.erase(x-5, 1);
		x--;
		mvdelch(y, x);
		refresh();
	}
}

void delete_right(int& y, int& x, string& s) {
	if (x < s.size()+4) {
		s.erase(x-4, 1);
		delch();
		refresh();
	}
}

void set_input(vector<string> prev_input, int& command_i, 
			   int& y, int& x, string& input) {
	// clear
	move(y, 4);
	clrtoeol();
	// print
	input = prev_input.at(command_i);
	printw(input.c_str());
	// update
	x = input.size()+4;
	move(y, x);
	refresh();
}

void prev_command(vector<string> prev_input, int& command_i, 
				  int& y, int& x, string& input) {
	if (command_i > 0) {
		command_i--;
	}
	if (input != prev_input.at(command_i)) {
		set_input(prev_input, command_i, y, x, input);
	}
}

void next_command(vector<string> prev_input, int& command_i, 
				  int& y, int& x, string& input) {
	if (command_i < prev_input.size()-1) {
		command_i++;
	}
	if (input != prev_input.at(command_i)) {
		set_input(prev_input, command_i, y, x, input);
	}
}

void print_char(int& y, int& x, int c, string& s) {
	addch(c);
	if (s.size() == 0) {
		s = string(1, c);
	} else {
		s.insert(x-4, string(1, c));
	}
	x++;
	refresh();
}

void next_line(int& y, int& x) {
	x = 0; y++;
	move(y, x);
}

void keyboard_interrupt(int& y, int& x) {
	next_line(y, x);
	printw("KeyboardInterrupt");
}

void print_debug(int c, int y, int x, bool has_color) {
	move(y+1, 0);
	if (has_color) attron(COLOR_PAIR(1));
	printw(to_string(c).c_str());
	if (has_color) attroff(COLOR_PAIR(1));
	move(y, x);
	refresh();
}

void interacive_terminal() {
	// interpreter variables
	string input, cout_str;
	Tokenizer tokenizer;
	Parser parser(&tokenizer);
	Interactive* parse_tree;
	Stack stack;
	PyObject ret;

	// previous inputs
	vector<string> prev_input;
	int c, command_i = -1;

	// swap cout to an ostringstream so it can be captured
	streambuf* old_cout = cout.rdbuf();
	stringstream new_cout;
	cout.rdbuf(new_cout.rdbuf());

	// enable color
	bool has_color = has_colors();
	if (has_color) {
		start_color();
		init_pair(1, COLOR_GREEN, COLOR_BLACK);
	}

	// init window
	int nlines = 100, ncols = 100, y0 = 0, x0 = 0;
	WINDOW* win = newwin(nlines, ncols, y0, x0);
	int x = 0, y = 0;

	bool halted = false;
	while (!halted) {
		decorate(y, x);
		input = "";
		while (1) {
			c=getch();
			// print_debug(c, y, x, has_color);  // uncomment for keycodes
			switch(c) {
			case KEY_UP:
				prev_command(prev_input, command_i, y, x, input);
				break;
			case KEY_DOWN:
				next_command(prev_input, command_i, y, x, input);
				break;
			case KEY_LEFT:
				cursor_left(y, x, input);
				break;
			case KEY_RIGHT:
				cursor_right(y, x, input);
				break;
			case KEY_BACKSPACE:
				delete_left(y, x, input);
				break;
			case 330:  // delete
				delete_right(y, x, input);
				break;
			case 10:  // enter
				goto finished;
				break;
			case 26:  // ctrl + z
				halted = true;
				goto finished;
				break;
			case 3:  // ctrl + c
				keyboard_interrupt(y, x);
				input = "";
				goto finished;
				break;
			default:
				print_char(y, x, c, input);
				break;
			}
		}
		finished:  // tag for exiting from the switch
		if (!halted) {
			next_line(y, x);

			if (input == "") continue;

			prev_input.push_back(input);
			command_i = prev_input.size();

			input += "\r\n";  // tokenizer is expecting this
			// tokenize
			tokenizer.tokenize_input(input);
			// parse
			parse_tree = dynamic_cast<Interactive*>(parser.parse("interactive"));
			// interpret
			ret = (*parse_tree).evaluate(stack);  

			// read in from stdout
			new_cout >> cout_str;
			new_cout.str(string());  // reset
			new_cout.clear();

			// print stdout result
			if (cout_str.size() > 0) {
				move(y, 0);
				printw(cout_str.c_str());
				next_line(y, x);
				cout_str.clear();
			}

			// print returned value
			if (ret.type != "None") {
				move(y, 0);
				printw(ret.as_string().c_str());
				next_line(y, x);
			}
			refresh();
			delete parse_tree;
		}
	}
	cout.rdbuf(old_cout);  // restore old cout
}

void cleanup() {
	// TODO: define destructors for all AST nodes and start deletion here
	
	Logger::get_instance()->close();
}

int main(int argc, char* argv[]) {
	if (argc > 1) {
		// interpreting input file
		vector<string> contents = read_lines(argv[1]);

		Tokenizer tokenizer(contents);
		if (argc > 2 && argv[2] == (string)"-v") {
			tokenizer.print();
		}

		Parser parser(&tokenizer);
    	File* parse_tree = dynamic_cast<File*>(parser.parse("file"));
		
		cout << endl << "AST:" << endl << *parse_tree << endl;

		Stack stack;
		cout << endl << "stdout:" << endl;
    	(*parse_tree).evaluate(stack);

		delete parse_tree;

	} else {
		// interactive terminal
		init_ncurses();

		// NOTE: endwin() must be called or bad things happen
		try {
			interacive_terminal();
		}
		catch (exception& e) {
			endwin();
			cout << "exception: " << e.what() << endl;
		}
		endwin();
	}
	cleanup();
	return 0;
}
