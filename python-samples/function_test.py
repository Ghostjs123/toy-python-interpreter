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
      
print(a())
print(b())
print(c())
