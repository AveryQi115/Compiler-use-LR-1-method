import tkinter as tk
from collections import OrderedDict
from .tk_multi_column_listbox import TkMultiColumnListbox


class TkIRPop(tk.Toplevel):
    def __init__(self):
        super().__init__()
        self.title('IR')

        ir = self._getIR()

        self.table = TkMultiColumnListbox(self, ir, width=3)
        self.table.pack(fill='both', expand=True)

    def _getIR(self, path='core\\out\\ir\\IR.txt'):
        lst0, lst1, lst2, lst3 = [], [], [], []

        for line in open(path):
            quat = line.split(' (')[1].rstrip('\n')
            quat = quat.rstrip(')')

            q0, q1, q2, q3 = quat.split(', ')

            lst0.append(q0)
            lst1.append(q1)
            lst2.append(q2)
            lst3.append(q3)

        return OrderedDict(q0=lst0, q1=lst1, q2=lst2, q3=lst3)
