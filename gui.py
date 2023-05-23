from tkinter import *
import tkinter.scrolledtext as scrolledtext
from tkinter.filedialog import askopenfilename
import re

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

# Setup Tkinter
window_size = (1080, 720)
frame_size = (int(window_size[0] * 0.90), int(window_size[1] - 20))
root = Tk()
root.title("A3E")
root.configure(background=background)
root.geometry(f"{window_size[0]}x{window_size[1]}")

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

    
# Line Number Wrapper
class LineNumbers(Text):
    def __init__(self, master, text_widget, **kwargs):
        super().__init__(master, **kwargs)
        self.text_widget = text_widget
        self.text_widget.bind('<KeyPress>', self.on_key_press)
        self.insert(1.0, '1')
        self.configure(state='disabled')

    def on_key_press(self, event=None):
        final_index = str(self.text_widget.index(END))
        num_of_lines = final_index.split('.')[0]
        line_numbers_string = "\n".join(str(no + 1) for no in range(int(num_of_lines)))
        width = len(str(num_of_lines))
        self.configure(state='normal', width=width)
        self.delete(1.0, END)
        self.insert(1.0, line_numbers_string)
        self.configure(state='disabled')


# TOP FRAME
topFrame = Frame(root, width=frame_size[0], height=frame_size[1], bg=background, borderwidth=5, relief="flat")
symbolTable = scrolledtext.ScrolledText(topFrame, width=50, height=28, borderwidth=3, relief="ridge", wrap="none", font=font, bg=background, fg=normal)
symbolTable.vbar.configure(bg=background)
symbolTable.configure(state='disabled')
editArea = Text(
    topFrame,
    bg=background,
    fg=normal,
    insertbackground=normal,
    relief="ridge",
    width=50, height=28,
    borderwidth=3,
    font=font
)
lineNumbers = LineNumbers(topFrame, editArea, width=3, bg=background, fg=normal, borderwidth=0, font=font)

# BOTTOM FRAME
bottomFrame = Frame(root, width=frame_size[0], height=frame_size[1], bg=background, borderwidth=5, relief="flat")
errorsArea = Text(bottomFrame, width=80, height=18, borderwidth=3, relief="ridge", wrap="none", font=font, bg=background, fg=normal)
errorsArea.configure(state='disabled')
compileBtn = Button(bottomFrame, text="Compile", width=15, height=1, fg='#2b2d35', font=font) # command=compile_code)
selecFileBtn = Button(bottomFrame, text="Select file", width=15, height=1, fg='#2b2d35', font=font) # command=select_file)

# PACKING TOP FRAME
topFrame.pack()
lineNumbers.pack(side="left", pady=30, fill="y")
editArea.pack(side="left", fill="y", pady=30)
symbolTable.pack(side="right", padx=20, pady=30)

# PACKING BOTTOM FRAME
bottomFrame.pack()
errorsArea.pack(side="left", padx=(20, 20), pady=(0, 30))
selecFileBtn.pack(side="top", pady=(0, 20))
compileBtn.pack(side="top", pady=(0, 20))

# BINDING EVENTS
editArea.insert('1.0', "// Code Here")
editArea.bind('<KeyRelease>', changes)

# MAIN LOOP
changes()
root.mainloop()
