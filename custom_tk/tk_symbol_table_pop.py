import tkinter as tk
from collections import OrderedDict
from .tk_multi_column_listbox import TkMultiColumnListbox


class TkSymbolTablePop(tk.Toplevel):
    def __init__(self):
        super().__init__()
        self.title('Tokenized result')

        symbols = self._getSymbols()

        self.table = TkMultiColumnListbox(self, symbols)
        self.table.pack(fill='both', expand=True)

    def _getSymbols(self, path='core\\out\\token\\tokenizer_result.txt'):
        type, value = [], []

        for line in open(path):
            _, t, v = line.split(' , ')
            t = t.lstrip('type: ')
            v = v.lstrip('value: ')

            type.append(t)
            value.append(v)

        return OrderedDict(type=type, value=value)
