from tkinter import *
from tkinter import ttk
import tkinter.scrolledtext as scrolledtext
from tkinter.filedialog import askopenfilename
import re

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
    ['(^| )(const|int|flt|bool|string|return|for|while|if|else|repeat|until|enum|true|false|switch|case|default)($| )', keywords],
    ['".*?"', string],
    ['\'.*?\'', string],
    ['//.*?$', comments],
    ['print', function],
]

# ====================================================================================================
codeText = ""
# Event Handler for Coda Area
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


# ====================================================================================================
# Setup Tkinter
window_size = (1080, 720)
frame_size = (int(window_size[0] * 0.90), int(window_size[1] - 20))
root = Tk()
root.title("A3E")
root.configure(background=background)
root.option_add("*tearOff", False)

# Create a custom Style object with a background color
style = ttk.Style()
style.configure("Custom.TNotebook", background=background)

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
