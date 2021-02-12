includes = -Ilib -Isrc -Itests -Isrc/stack -Isrc/objects -Isrc/ast
default_args = -pedantic

libs = lib/util.cpp
pyobject = src/objects/pyobject.cpp
stack = src/stack/stack.cpp src/stack/frame.cpp
ast = src/ast/ast.cpp src/ast/ast_helpers.cpp

tokenizer = src/tokenizer.cpp src/objects/token.cpp $(libs)
parser = src/parser.cpp  $(tokenizer) $(pyobject) $(ast) $(stack)
interpreter = src/interpreter.cpp src/objects/builtins.cpp $(parser)

# mains for testing
interpreter-main: $(interpreter)
	g++ $(interpreter) $(default_args) $(includes) -D INT_MAIN -o interpreter-main
parser-main: $(parser)
	g++ $(parser) $(default_args) $(includes) -D PARS_MAIN -o parser-main
tokenizer-main: $(tokenizer)
	g++ $(tokenizer) $(default_args) $(includes) -D TOK_MAIN -o tokenizer-main

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

