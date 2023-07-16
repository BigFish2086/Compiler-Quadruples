import sys
import re

def error(msg):
    print(f"Error: {msg}")
    sys.exit(1)

def is_expr(expr):
    """Returns True if the given string is an immediate value, False otherwise."""
    return (
        # Strings, negative numbers, and fractions.
        expr[0] in ['"', '-', '.'] or
        # Booleans.
        expr in ["true", "false"] or
        # Numbers.
        expr[0].isdigit()
    )

def to_expr(expr):
    """Converts a string representing an immediate value to a pythonic object."""
    # Strings
    if expr[0] == '"':
        return expr[1:-1]
    # True and False
    elif expr in ["true", "false"]:
        return {"true": True, "false": False}[expr]
    # Floats
    elif '.' in expr:
        return float(expr)
    # Integers
    else:
        return int(expr)

def exec(quadFile):
    with open(quadFile, "r") as f:
        asm = f.read().splitlines()
    if len(asm) == 0:
        error("no code to execute.")

    # Initialize the VM.
    stack = []          # The stack of the VM.
    variables = {}      # Runtime variables.
    labels = {}         # For labels and functions.
    index_stack = []    # For CALL and RET.

    # First, find all the labels and functions.
    for index, line in enumerate(asm):
        if line.startswith(("label", "def")):
            labels[line.split()[1][:-1]] = index

    # Run the program.
    index = 0
    while True:
        line = asm[index]
        if line == "":
            pass
        elif line.startswith("/*"):
            pass
        elif line.startswith(("label", "def")):
            pass
        elif line == "int2flt":
            stack.append(float(stack.pop()))
        elif line == "flt2int":
            stack.append(int(stack.pop()))
        elif line == "pop":
            stack.pop()
        elif line == "print":
            print(stack.pop())
        elif line.startswith("push"):
            to_push = line.split(maxsplit=1)[1]
            if is_expr(to_push):
                stack.append(to_expr(to_push))
            else:
                if variables.get(to_push) is None:
                    error("variable " + to_push + " is being used without being initialized.")
                stack.append(variables[to_push])
        elif line.startswith("dup"):
            stack.append(stack[-1])
        elif line.startswith("pop"):
            variables[line.split()[1]] = stack.pop()
        elif line == "plus":
            stack.append(stack.pop() + stack.pop())
        # NOTE(MINUS, DIV, LT, GT, ...): stack[-2] is the first operand & stack[-1] is the second. Popping happens in reverse order.
        elif line == "minus":
            stack.append(-stack.pop() + stack.pop())
        elif line == "mult":
            stack.append(stack.pop() * stack.pop())
        elif line == "div":
            second = stack.pop()
            first = stack.pop()
            if second == 0:
                error("division by zero.")
            # Note that both operands gonna be of the same type anyways (int or float).
            if isinstance(first, int):
                stack.append(first // second)
            else:
                stack.append(first / second)
        elif line == "neg":
            stack.append(-stack.pop())
        elif line == "lt":
            stack.append(stack.pop() > stack.pop())
        elif line == "gt":
            stack.append(stack.pop() < stack.pop())
        elif line == "lteq":
            stack.append(stack.pop() >= stack.pop())
        elif line == "gteq":
            stack.append(stack.pop() <= stack.pop())
        elif line == "eq":
            stack.append(stack.pop() == stack.pop())
        elif line == "neq":
            stack.append(stack.pop() != stack.pop())
        elif line == "qand":
            stack.append(stack.pop() and stack.pop())
        elif line == "qor":
            stack.append(stack.pop() or stack.pop())
        elif line == "qnot":
            stack.append(not stack.pop())
        elif line.startswith("jmp"):
            index = labels[line.split()[1]]
        elif line.startswith("jz"):
            if stack.pop() == 0:
                index = labels[line.split()[1]]
        elif line.startswith("call"):
            index_stack.append(index)
            index = labels[line.split()[1]]
        elif line == "ret":
            index = index_stack.pop()
        else:
            error("invalid instruction: " + line)

        index += 1
        if index >= len(asm):
            break


if __name__ == "__main__":
    if len(sys.argv) != 2:
        error("Invalid number of arguments")
    exec(sys.argv[1])

