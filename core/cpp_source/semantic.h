#ifndef SEMANTIC_H
#define SEMANTIC_H

#include <fstream>
#include <iostream>
#include <vector>

#include "config.h"
#include "utils.h"
#include "symtab.h"

class Semantic
{
private:
    std::vector<SymTab> _symtab;
    std::vector<int> _cur_symtab_stack;
    bool _initSymTab(TABLE_TYPE, const std::string &);

    int _next_state;
    std::ofstream _ir_tmp_w_hd;
    std::ifstream _ir_tmp_r_hd;
    std::ofstream _ir_w_hd;

    int _bp_lv;
    std::vector<Tuple4> _tuple4_stack;
    std::vector<int> _bp_val;
    std::vector<int> _bp_dot_pos;
    int _main_line;
    bool _printTuple4(const Tuple4 &);
    bool _printTuple4();

public:
    Semantic();
    ~Semantic();

    bool doSemanticCheck(std::vector<GramSym> &, const Prod &);
    int getNextState();
    int peekNextState();

    std::string getArg(SymPos &, bool is_ret = false);
};

#endif
