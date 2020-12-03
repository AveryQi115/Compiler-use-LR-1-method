
#ifndef SYMTAB_H
#define SYMTAB_H

#include <string>
#include <vector>

#include "utils.h"

struct Symbol
{
    CAT_TYPE cat;
    std::string name;
    VAR_TYPE type;
    std::string val;

    int parameter_num = 0;
    int entry_addr;
    int func_sym_pos;
    int func_table_num;

    int offset = -1;
    int reg = -1;
};

enum TABLE_TYPE
{
    CONST_TABLE,
    TEMP_TABLE,
    GLOBAL_TABLE,
    FUNCTION_TABLE,
    WHILE_TABLE,
    IF_TABLE
};

class SymTab
{
private:
    std::vector<Symbol> _table;
    TABLE_TYPE _table_type;
    std::string _name;

public:
    SymTab(TABLE_TYPE, std::string = "");
    ~SymTab();
    TABLE_TYPE getTabType();
    int addSym(const Symbol &);
    int addSym(const std::string &);
    int addSym();

    int findSym(const std::string &) const;
    int findConst(const std::string &) const;
    bool setVal(int pos, std::string &);
    std::string getSymName(int pos) const;
    CAT_TYPE getSymCat(int pos) const;
    VAR_TYPE getSymType(int pos) const;
    std::string getTabName() const;
    Symbol &getSym(int pos);
    std::vector<Symbol> &getTab();
};

#endif