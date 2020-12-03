import tkinter as tk
import tkinter.filedialog as F
import tkinter.messagebox as M
import networkx as nx
from functools import partial

import subprocess

from .tk_symbol_table_pop import TkSymbolTablePop
from .tk_ir_pop import TkIRPop


class TkMain(tk.Tk):
    def __init__(self):
        super().__init__()

        self.filename = ""

        self._initMenuBar()

        self.code_input = tk.Text(self)
        self.code_input.pack(side=tk.LEFT)
        self.focus_set()

    def _initMenuBar(self):
        self.menu_bar = tk.Menu(self)

        file_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.menu_bar.add_cascade(label='File', menu=file_menu)

        file_menu.add_command(label='Import File', command=self._importFile)
        file_menu.add_separator()
        file_menu.add_command(label='Exit', command=self.destroy)

        compile_menu = tk.Menu(self.menu_bar, tearoff=0)
        self.menu_bar.add_cascade(label='Compile', menu=compile_menu)

        compile_menu.add_command(label='Tokenize', command=self._symbolTable)
        compile_menu.add_command(label='IR', command=self._IR)

        self.config(menu=self.menu_bar)

    def _importFile(self):
        filename = F.askopenfilename(filetypes=
            [("C source files", "*.c"),
             ("text files", "*.txt"),
             ("All files", "*.*")])

        self.code_input.delete("1.0", tk.END)
        for i in open(filename, encoding="UTF-8"):
            self.code_input.insert(tk.END, i)

    def _symbolTable(self):
        self._genTmpFile()

        p = subprocess.Popen(['./core/compile.exe', '-l'],
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        out, err = p.communicate()
        out, err = out.decode('gbk').rstrip('\r\n'), err.decode('gbk').rstrip('\r\n')

        if len(err) != 0:
            M.showerror(title='Error',
                        message=err)
        else:
            self.wait_window(TkSymbolTablePop())

    def _IR(self):
        self._genTmpFile()

        p = subprocess.Popen(['./core/compile.exe'],
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
        out, err = p.communicate()
        out, err = out.decode('gbk').rstrip('\r\n'), err.decode('gbk').rstrip('\r\n')

        if len(err) != 0:
            M.showerror(title='Error',
                        message=err)
        else:
            symbol_tabel_popup = TkSymbolTablePop()
            ir_popup = TkIRPop()
            self.wait_window(symbol_tabel_popup)
            self.wait_window(ir_popup)

    def _genTmpFile(self, path='core/in/tmp.txt'):
        with open(path, mode='w') as file:
            file.write(self.code_input.get("1.0", tk.END).rstrip('\n'))

    def _setMenuEntryState(self, dic, stat):
        for submenu, cmds in dic.items():
            for cmd in cmds:
                self.menu_bar.children[submenu].entryconfig(cmd, state=stat)
