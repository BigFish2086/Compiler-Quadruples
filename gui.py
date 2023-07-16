from tkinter import *
from tkinter import ttk
import tkinter.scrolledtext as scrolledtext
from tkinter.filedialog import askopenfilename
import re
from os import path
from subprocess import Popen, PIPE

# ====================================================================================================
def search_re(pattern, text, groupid=0):
    matches = []
    text = text.splitlines()
    for i, line in enumerate(text):
        for match in re.finditer(pattern, line):

            matches.append(
                (f"{i + 1}.{match.start()}", f"{i + 1}.{match.end()}")
            )
    return matches

def rgb(rgb):
    return "#%02x%02x%02x" % rgb

# Define colors for the variouse types of tokens
normal = rgb((234, 234, 234))
keywords = rgb((234, 95, 95))
comments = rgb((95, 234, 165))
string = rgb((234, 162, 95))
function = rgb((95, 211, 234))
background = rgb((42, 42, 42))
font = 'Consolas 10'

# Define a list of Regex Pattern that should be colored in a certain way
repl = [
    ['(^| )(const|int|flt|bool|str|return|enum|true|false)($| )', keywords],
    ['".*?"', string],
    ['\'.*?\'', string],
    ['//.*?$', comments],
    ['print|for|while|if|else|repeat|until|switch|case|default', function],
]

codeText = ""
quadText = ""
symbolTableText = ""
consoleText = ""

isCompiled = False
compilerPath = "./bin/compiler"
executorPath = "./exec.py"
filesdir = "./files/"
codeFile = filesdir + "code.in"
quadFile = filesdir + "quads.out"
symbolTableFile = filesdir + "symboltable.log"

def buildCompiler():
    global compilerPath, consoleText, isCompiled
    isCompiled = True
    proc = Popen(["make", "build"], stdout=PIPE, stderr=PIPE)
    output, error = proc.communicate()
    compilerPath = "bin/compiler"
    if path.isfile(compilerPath):
        consoleText = "Compiler built successfully"
    else:
        consoleText = "Error building compiler"
    consoleText += "\n\n" + output.decode("utf-8") + "\n\n" + error.decode("utf-8")


# ====================================================================================================
def changes(event=None):
    global codeText
    if editArea.get('1.0', END) == codeText:
        return
    for tag in editArea.tag_names():
        editArea.tag_remove(tag, "1.0", "end")
    i = 0
    for pattern, color in repl:
        for start, end in search_re(pattern, editArea.get('1.0', END)):
            editArea.tag_add(f'{i}', start, end)
            editArea.tag_config(f'{i}', foreground=color)
            i+=1
    codeText = editArea.get('1.0', END) 

def compile(event=None):
    global codeText, quadText, symbolTableText, consoleText, compilerPath, isCompiled
    with open(codeFile, "w") as file:
        file.write(editArea.get('1.0', END))
    # check if compiler binary is there in bin folder
    if path.isfile(compilerPath):
        proc = Popen([compilerPath, codeFile], stdout=PIPE, stderr=PIPE)
        output, error = proc.communicate()
        res = output.decode("utf-8") + "\n\n" + error.decode("utf-8")
        consoleText = "" if not isCompiled else consoleText
        consoleText += res
        with open(quadFile, "r") as file:
            quadText = file.read()
        with open(symbolTableFile, "r") as file:
            symbolTableText = file.read()

        quadArea.config(state=NORMAL)
        quadArea.delete('1.0', END)
        quadArea.insert('1.0', quadText)
        quadArea.config(state=DISABLED)

        symbolTable.config(state=NORMAL)
        symbolTable.delete('1.0', END)
        symbolTable.insert('1.0', symbolTableText)
        symbolTable.config(state=DISABLED)

        console.config(state=NORMAL)
        console.delete('1.0', END)
        console.insert('1.0', consoleText)
        console.config(state=DISABLED)

    # otherwise compile the compiler and then compile the code
    else:
        buildCompiler()
        compile()
        isCompiled = False

def execute(event=None):
    global quadFile, consoleText, executorPath
    res = ""
    if path.isfile(executorPath):
        proc = Popen(["python3", executorPath, quadFile], stdout=PIPE, stderr=PIPE)
        output, error = proc.communicate()
        res = output.decode("utf-8") + "\n\n" + error.decode("utf-8")
    else:
        res = "Executor not found\n\n"
    consoleText = "" if not isCompiled else consoleText
    consoleText += "[*] Run:\n" + res
    console.config(state=NORMAL)
    console.delete('1.0', END)
    console.insert('1.0', consoleText)
    console.config(state=DISABLED)

def selectFile(event=None):
    file_name = askopenfilename(initialdir=".", title="Select file", filetypes=(("all files", "*.*"), ("text files", "*.txt")))
    if not file_name:
        return
    with open(file_name, "r") as file:
        global code_area
        editArea.delete("1.0", END)
        editArea.insert("1.0", file.read())
        changes()


# ====================================================================================================
# Setup Tkinter
window_size = (1080, 720)
frame_size = (int(window_size[0] * 0.90), int(window_size[1] - 20))
root = Tk()
root.title("Compiler")
root.configure(background=background)
root.option_add("*tearOff", False)

# Create a custom Style object with a background color
style = ttk.Style()
style.configure("Custom.TNotebook", background=background)
style.configure("Custom.TNotebook.Tab", background="green3")

notebook = ttk.Notebook(root, style="Custom.TNotebook", padding=(10, 10, 10, 10))
tab_1 = ttk.Frame(notebook)
notebook.add(tab_1, text="   Editor   ")
notebook.pack(fill="both", expand=True)
editArea = scrolledtext.ScrolledText(
    tab_1,
    bg=background,
    fg=normal,
    insertbackground=normal,
    relief="flat",
    width=99, height=30,
    borderwidth=3,
    font=font,
    insertofftime=0,
    padx=10,
    pady=10,
)
editArea.pack(fill=BOTH, expand=1)
editArea.insert('1.0',
"""
// Code Here
// <Ctrl+c> to compile
// <Ctrl+r> to run
// <Ctrl+i> to load a file
""")
editArea.bind('<KeyRelease>', changes)
editArea.bind('<Control-c>', compile)
editArea.bind('<Control-r>', execute)
editArea.bind('<Control-i>', selectFile)

# ====================================================================================================
tab_2 = ttk.Frame(notebook)
notebook.add(tab_2, text="   Quads   ")
notebook.pack(fill="both", expand=True)
quadArea = scrolledtext.ScrolledText(
    tab_2,
    bg=background,
    fg=normal,
    insertbackground=normal,
    relief="flat",
    width=99, height=30,
    borderwidth=3,
    font=font,
    insertofftime=0,
    padx=10,
    pady=10,
    state=DISABLED
)
quadArea.pack(fill=BOTH, expand=1)

# ====================================================================================================
tab_3 = ttk.Frame(notebook)
notebook.add(tab_3, text="   Symbol Table   ")
notebook.pack(fill="both", expand=True)
symbolTable = scrolledtext.ScrolledText(
    tab_3,
    bg=background,
    fg=normal,
    insertbackground=normal,
    relief="flat",
    width=99, height=30,
    borderwidth=3,
    font=font,
    insertofftime=0,
    padx=10,
    pady=10,
    state=DISABLED
)
symbolTable.pack(fill=BOTH, expand=1)

# ====================================================================================================
tab_4 = ttk.Frame(notebook)
notebook.add(tab_4, text="   Console   ")
notebook.pack(fill="both", expand=True)
console = scrolledtext.ScrolledText(
    tab_4,
    bg=background,
    fg=normal,
    insertbackground=normal,
    relief="flat",
    width=99, height=30,
    borderwidth=3,
    font=font,
    insertofftime=0,
    padx=10,
    pady=10,
    state=DISABLED
)
console.pack(fill=BOTH, expand=1)

# ====================================================================================================





changes()
root.mainloop()
