includes = -Ilib -Isrc -Itests -Isrc/stack -Isrc/objects -Isrc/ast
default_args = -pedantic

libs = util.o
pyobject = pyobject.o
stack = stack.o # frame.o
ast = ast.o ast_helpers.o

tokenizer = tokenizer.o token.o logging.o $(libs) -lncurses
parser = parser.o $(tokenizer) $(ast) $(pyobject) $(stack) builtins.o
interpreter = interpreter.o $(parser)

# default
mypy: $(interpreter)
	g++ $(interpreter) $(default_args) $(includes) -o mypy

# mains for testing
parser-main: tests/parser-main.cpp $(parser)
	g++ tests/parser-main.cpp $(parser) $(default_args) $(includes) -D PARS_MAIN -o parser-main
tokenizer-main: tests/tokenizer-main.cpp $(tokenizer)
	g++ tests/tokenizer-main.cpp $(tokenizer) $(default_args) $(includes) -D TOK_MAIN -o tokenizer-main

# test cases
interpreter-tests: tests/interpreter-tests.cpp $(interpreter) 
	g++ tests/interpreter-tests.cpp $(interpreter) $(includes) -o interpreter-tests
parser-tests: tests/parser-tests.cpp $(parser)
	g++ tests/parser-tests.cpp $(parser) $(includes) -o parser-tests
tokenizer-tests: tests/tokenizer-tests.cpp $(tokenizer)
	g++ tests/tokenizer-tests.cpp $(tokenizer) $(includes) -o tokenizer-tests
util-tests: tests/util-tests.cpp lib/util.cpp
	g++ tests/util-tests.cpp lib/util.cpp $(includes) -o util-tests.e

# single tests
ast_inheritance-test: single-tests/ast_inheritance-test.cpp
	g++ single-tests/ast_inheritance-test.cpp -o ast_inheritance-test
default_op_behaviors-test: single-tests/default_op_behaviors-test.cpp
	g++ single-tests/default_op_behaviors-test.cpp -o default_op_behaviors-test
function-test: single-tests/function-test.cpp $(pyobject)
	g++ single-tests/function-test.cpp $(pyobject) -o function-test
misc_cpp-test: single-tests/misc_cpp-test.cpp $(pyobject)
	g++ single-tests/misc_cpp-test.cpp $(pyobject) -o misc_cpp-test
pyobject-test: single-tests/pyobject-test.cpp $(pyobject)
	g++ single-tests/pyobject-test.cpp $(pyobject) -o pyobject-test

# ===============================================================================
# Object Files

clean:
	rm -rf *.o

# src/

tokenizer.o: src/tokenizer.cpp src/tokenizer.h
	g++ src/tokenizer.cpp $(includes) -c -o tokenizer.o

parser.o: src/parser.cpp src/parser.h
	g++ src/parser.cpp $(includes) -c -o parser.o

interpreter.o: src/interpreter.cpp src/interpreter.h
	g++ src/interpreter.cpp $(includes) -c -o interpreter.o

# src/ast/

ast_helpers.o: src/ast/ast_helpers.cpp src/ast/ast_helpers.h
	g++ src/ast/ast_helpers.cpp $(includes) -c -o ast_helpers.o

ast.o: src/ast/ast.cpp src/ast/ast.h
	g++ src/ast/ast.cpp $(includes) -c -o ast.o

# src/objects/

builtins.o: src/objects/builtins.cpp src/objects/builtins.h
	g++ src/objects/builtins.cpp $(includes) -c -o builtins.o

pyobject.o: src/objects/pyobject.cpp src/objects/pyobject.h
	g++ src/objects/pyobject.cpp $(includes) -c -o pyobject.o

token.o: src/objects/token.cpp src/objects/token.h
	g++ src/objects/token.cpp $(includes) -c -o token.o

# src/stack/

stack.o: src/stack/stack.cpp src/stack/stack.h
	g++ src/stack/stack.cpp $(includes) -c -o stack.o

# lib/

util.o: lib/util.cpp lib/util.h
	g++ lib/util.cpp $(includes) -c -o util.o

logging.o: lib/logging.cpp lib/logging.h
	g++ lib/logging.cpp $(includes) -c -o logging.o
