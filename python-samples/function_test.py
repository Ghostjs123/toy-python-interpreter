# simple as possible
def a():
    return "worked"

# testing one liner
def b(): return "worked"

# testing if-elif-else
def c():
    if 0:
        return "didnt"
    elif 1:
        return "worked"
    else:
        return "didnt"

# testing multiple nested blocks 1
def d():
    if 1:
        if 0:
            return "didnt"
        elif 1:
            return "worked"
    return "didnt"

# testing multiple nested blocks 2
def e():
    if 1:
        if 1:
            if 1:
                if 0:
                    return "didnt"
                elif 0:
                    return "didnt"
                elif 0: return "didnt"
                elif 1:
                    return "worked"
    return "didnt"

# testing multiple nested blocks 3 - specifically else
def f():
    if 1:
        if 1:
            if 1:
                if 0:
                    return "didnt"
                elif 0:
                    return "didnt"
                elif 0: return "didnt"
                elif 0:
                    return "didnt"
                else:
                    return "worked"
    return "didnt"

# testing arguments
      
print("a: ", a())
print("b: ", b())
print("c: ", c())
print("d: ", d())
print("e: ", e())
print("f: ", f())
