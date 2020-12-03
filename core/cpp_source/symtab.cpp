#include "symtab.h"

SymTab::SymTab(TABLE_TYPE t_type, std::string name)
{
    _table_type = t_type;
    _name = name;
}

SymTab::~SymTab()
{
}

std::vector<Symbol> &SymTab::getTab()
{
    return _table;
}

TABLE_TYPE SymTab::getTabType()
{
    return _table_type;
}

Symbol &SymTab::getSym(int pos)
{
    return _table[pos];
}

int SymTab::addSym(const Symbol &sb)
{

    if (-1 != findSym(sb.name))
        return -1;

    _table.push_back(sb);
    return _table.size() - 1;
}

int SymTab::addSym(const std::string &str)
{
    std::string tmp_name = "T" + std::to_string(_table.size());
    Symbol sb;
    sb.name = tmp_name;
    sb.cat = TEMP;
    sb.val = str;
    _table.push_back(sb);
    return _table.size() - 1;
}

int SymTab::addSym()
{
    std::string tmp_name = "T" + std::to_string(_table.size());
    Symbol sb;
    sb.name = tmp_name;
    sb.cat = TEMP;
    _table.push_back(sb);
    return _table.size() - 1;
}

std::string SymTab::getTabName() const
{
    return _name;
}

std::string SymTab::getSymName(int pos) const
{
    return _table[pos].name;
}

int SymTab::findSym(const std::string &name) const
{
    for (auto iter = _table.begin(); iter != _table.end(); ++iter)
    {
        if (name == iter->name)
            return iter - _table.begin();
    }

    return -1;
}

int SymTab::findConst(const std::string &const_value) const
{
    for (auto iter = _table.begin(); iter != _table.end(); ++iter)
    {
        if (iter->cat != CONST)
            continue;
        if (const_value == iter->val)
            return iter - _table.begin();
    }

    return -1;
}

bool SymTab::setVal(int pos, std::string &value)
{
    _table[pos].val = value;
    return true;
}
CAT_TYPE SymTab::getSymCat(int pos) const
{
    return _table[pos].cat;
}
VAR_TYPE SymTab::getSymType(int pos) const
{
    return _table[pos].type;
}