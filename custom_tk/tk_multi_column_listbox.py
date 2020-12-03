import tkinter as tk
from collections import OrderedDict

def _multiple(*func_list):
    return lambda *args, **kw: [func(*args, **kw) for func in func_list]; None

def _scroll_to_view(scroll_set, *view_funcs):
    ''' Allows one widget to control the scroll bar and other widgets
    scroll set: the scrollbar set function
    view_funcs: other widget's view functions
    '''
    def closure(start, end):
        scroll_set(start, end)
        for func in view_funcs:
            func('moveto', start)
    return closure


class TkMultiColumnListbox(tk.Frame):
    def __init__(self, master=None, data=OrderedDict(), row_select=True, **kwargs):
        '''makes a multicolumn listbox by combining a bunch of single listboxes
        with a single scrollbar
        :columns:
          (int) the number of columns
          OR (1D list or strings) the column headers
        :data:
          (1D iterable) auto add some data
        :row_select:
          (boolean) When True, clicking a cell selects the entire row
        All other kwargs are passed to the Listboxes'''
        tk.Frame.__init__(self, master, borderwidth=1, highlightthickness=1, relief=tk.SUNKEN)
        self.rowconfigure(1, weight=1)
        self.columns = data.keys()
        
        for col, text in enumerate(self.columns):
            tk.Label(self, text=text).grid(row=0, column=col)

        self.boxes = []
        for col in range(len(self.columns)):
            box = tk.Listbox(self, exportselection=not row_select, **kwargs)
            if row_select:
                box.bind('<<ListboxSelect>>', self.selected)
            box.grid(row=1, column=col, sticky='nsew')
            self.columnconfigure(col, weight=1)
            self.boxes.append(box)
        vsb = tk.Scrollbar(
            self,
            orient=tk.VERTICAL,
            command=_multiple(*[box.yview for box in self.boxes]))
        vsb.grid(row=1, column=col+1, sticky='ns')
        for box in self.boxes:
            box.config(yscrollcommand=_scroll_to_view(vsb.set,
                       *[b.yview for b in self.boxes if b is not box]))
        self._init_data(data)

    def selected(self, event=None):
        row = event.widget.curselection()[0]
        for lbox in self.boxes:
            lbox.select_clear(0, tk.END)
            lbox.select_set(row)

    def _init_data(self, data):
        for box, lst in zip(self.boxes, data.values()):
            for v in lst:
                if v is None:
                    continue

                box.insert(tk.END, v)
          
    def __getitem__(self, index):
        '''get a row'''
        return [box.get(index) for box in self.boxes]

    def __delitem__(self, index):
        '''delete a row'''
        [box.delete(index) for box in self.boxes]

    def curselection(self):
        '''get the currently selected row'''
        selection = self.boxes[0].curselection()
        return selection[0] if selection else None
