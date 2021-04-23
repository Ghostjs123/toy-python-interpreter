# MyPy - toy python interpreter

Current Progress (interpreted successfully):
- +, -, /, *, works for int, float, bool
- print() works
- functions work
- if-elif-else works

Features working:
- Interactive mode vs file mode (./mypy vs ./mypy \[filename\])

Notes:
- Attempted to follow pythons [grammar](https://docs.python.org/3/reference/grammar.html) as closely as possible for my AST nodes.
- For multivariate return types in python, I created a [pyobject](src/objects/pyobject.cpp) class that can contain any possible variable value.
- Most of the execution takes place in [pyobject.cpp](src/objects/pyobject.cpp) and [ast.cpp](src/ast/ast.cpp).
- The [tokenizer](src/tokenizer.cpp) is incredibly over engineered. However, my goal was to get it to work exactly like 'python -m tokenize \[filename\]' and its getting very close.

Next Steps:
- Get set, list, tuple, working in pyobject and AST parsing.
- Get a whole bunch of AST nodes implemented, see TODO's in [ast.cpp](src/ast/ast.cpp).

Large TODOs:
- [pyobject](src/objects/pyobject.cpp) is really over-engineered. I think I can fall back on implicit casting and remove large parts of the operator definitions. Additionally, I think I can remove the type string as a parameter of the constructors. Python's ops work nearly identical to c++'s ops and I am underleveraging them.
- Unsure on how bitwise op precedent will work in practice. ex: (1 | 1 ^ 1) == (1 | (1 ^ 1)), left->right precedence does not apply. I think the way the [Comparison](https://docs.python.org/3/reference/grammar.html) grammer is specified will result with BitwiseOr, BitwiseAnd, etc with no more than 3 children.
