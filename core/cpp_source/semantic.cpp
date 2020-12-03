#include "semantic.h"

Semantic::Semantic()
{
    _next_state = 1;
    _bp_lv = 0;
    _main_line = -1;

    _symtab.push_back(SymTab(GLOBAL_TABLE, "global_table"));
    _cur_symtab_stack.push_back(0);

    _symtab.push_back(SymTab(TEMP_TABLE, "temp_table"));

    _ir_tmp_w_hd.open(path::semantic::IR_TMP_PATH);
    if (!_ir_tmp_w_hd.is_open())
    {
        std::cerr << "用来保存输出中间代码的临时文件打开失败！" << std::endl;
    }
}

Semantic::~Semantic()
{
    if (_ir_tmp_w_hd.is_open())
        _ir_tmp_w_hd.close();

    if (_ir_w_hd.is_open())
        _ir_w_hd.close();

    if (_ir_tmp_r_hd.is_open())
        _ir_tmp_r_hd.close();

    remove(path::semantic::IR_TMP_PATH.c_str());
}

int Semantic::getNextState()
{
    return _next_state++;
}

int Semantic::peekNextState()
{
    return _next_state;
}

std::string Semantic::getArg(SymPos &sp, bool is_ret)
{
    std::string arg_name = _symtab[sp.table_pos].getSymName(sp.sym_pos);

    if (!is_ret)
    {
        if (VARIABLE == _symtab[sp.table_pos].getSymCat(sp.sym_pos))
        {
            arg_name += "-" +
                        _symtab[sp.table_pos].getTabName() + "Var";
        }
    }
    else
    {
        arg_name += "-" +
                    _symtab[0].getSym(_symtab[0].findSym(_symtab[sp.table_pos].getTabName())).name +
                    "_paramerter " +
                    std::to_string(sp.sym_pos);
    }

    return arg_name;
}

bool Semantic::_printTuple4(const Tuple4 &tuple4)
{
    if (_bp_lv == 0)
    {
        _ir_tmp_w_hd << tuple4.num << " (" << tuple4.op << ", "
                     << tuple4.arg1 << ", " << tuple4.arg2 << ", " << tuple4.result << ")"
                     << std::endl;
    }
    else
    {
        _tuple4_stack.push_back(tuple4);
    }

    return true;
}

bool Semantic::_printTuple4()
{
    _ir_w_hd.open(path::semantic::IR_PATH);
    if (!_ir_w_hd.is_open())
    {
        std::cerr << "用来保存输出中间代码的文件打开失败！" << std::endl;
        return false;
    }

    _ir_tmp_r_hd.open(path::semantic::IR_TMP_PATH);
    if (!_ir_tmp_r_hd.is_open())
    {
        std::cerr << "读取用来保存输出中间代码的临时文件打开失败！" << std::endl;
        return false;
    }

    if (_ir_tmp_w_hd.is_open())
    {
        _ir_tmp_w_hd.close();
    }

    _ir_w_hd << 0 << " (j, "
             << "-, "
             << "-, " << _main_line << ")" << std::endl;

    while (!_ir_tmp_r_hd.eof())
    {
        std::string str;
        std::getline(_ir_tmp_r_hd, str);

        if (str == "")
        {
            continue;
        }

        _ir_w_hd << str << std::endl;
    }

    if (_ir_tmp_r_hd.is_open())
    {
        _ir_tmp_r_hd.close();
    }

    if (remove(path::semantic::IR_TMP_PATH.c_str()) != 0)
    {
        std::cerr << "中间代码临时文件删除失败！" << std::endl;
    }

    return true;
}

bool Semantic::_initSymTab(TABLE_TYPE table_type, const std::string &table_name)
{
    _symtab.push_back(SymTab(table_type, table_name));
    _cur_symtab_stack.push_back(_symtab.size() - 1);
    return true;
}

bool Semantic::doSemanticCheck(std::vector<GramSym> &sym_stack, const Prod &prod)
{
    if ("Const_value" == prod.left)
    {
        GramSym gram_sym = sym_stack.back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = gram_sym.txt_val;

        int len = 0;
        if ("$" != prod.right[0])
        {
            len = prod.right.size();
        }

        for (int i = 0; i < len; ++i)
        {
            sym_stack.pop_back();
        }

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Factor" == prod.left &&
             "Const_value" == prod.right[0])
    {

        GramSym gram_sym = sym_stack.back();

        int sym_pos = _symtab[1].addSym(gram_sym.txt_val);
        std::string sym_name = _symtab[1].getSymName(sym_pos);

        GramSym gram_sym_con = {prod.left,
                                sym_name,
                                {1, sym_pos}};

        _printTuple4(Tuple4({getNextState(), ":=", gram_sym.txt_val, "-", sym_name}));

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
        {
            sym_stack.pop_back();
        }

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Factor" == prod.left && prod.right[0] == "(")
    {

        GramSym sym_exp = sym_stack[sym_stack.size() - 2];

        GramSym gram_sym_con = {prod.left,
                                _symtab[sym_exp.pos.table_pos].getSymName(sym_exp.pos.sym_pos),
                                sym_exp.pos};

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Factor" == prod.left && prod.right[0] == "identifier")
    {

        GramSym &sym_identifier = sym_stack[sym_stack.size() - 2];
        GramSym sym_ftype = sym_stack.back();

        int identifier_pos = -1;
        int identifier_layer = _cur_symtab_stack.size() - 1;
        for (; identifier_layer >= 0; --identifier_layer)
        {
            SymTab search_table = _symtab[_cur_symtab_stack[identifier_layer]];
            identifier_pos = search_table.findSym(sym_identifier.txt_val);
            if (identifier_pos != -1)
                break;
        }

        if (-1 == identifier_pos)
        {
            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "没有定义！" << std::endl;
            return false;
        }

        sym_identifier.pos.table_pos = _cur_symtab_stack[identifier_layer];
        sym_identifier.pos.sym_pos = identifier_pos;

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        if (sym_ftype.txt_val == "")
        {

            if (TEMP == _symtab[sym_identifier.pos.table_pos].getSymCat(sym_identifier.pos.sym_pos))
            {

                gram_sym_con.pos = {sym_identifier.pos.table_pos, sym_identifier.pos.sym_pos};
                gram_sym_con.txt_val = sym_identifier.txt_val;
            }
            else
            {

                int sym_pos = _symtab[1].addSym(sym_identifier.txt_val);
                std::string sym_name = _symtab[1].getSymName(sym_pos);

                _printTuple4(Tuple4({getNextState(), ":=", getArg(sym_identifier.pos), "-", sym_name}));

                gram_sym_con.pos = {1, sym_pos};
            }
        }
        else
        {

            int table_pos, sym_pos = 0;
            table_pos = _symtab[sym_ftype.pos.table_pos].getSym(sym_ftype.pos.sym_pos).func_sym_pos;
            gram_sym_con.txt_val = "";

            int sym_pos_t = _symtab[1].addSym(sym_identifier.txt_val);
            std::string sym_name = _symtab[1].getSymName(sym_pos_t);

            int func_table_pos_t = _symtab[0].getSym(sym_identifier.pos.sym_pos).func_sym_pos;
            SymPos ret_sym_pos = {func_table_pos_t, 0};
            _printTuple4(Tuple4({getNextState(), ":=", getArg(ret_sym_pos), "-", sym_name}));
            gram_sym_con.pos = {1, sym_pos_t};
        }

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Factor_loop" == prod.left && prod.right[0] == "$")
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Factor_loop" == prod.left && prod.right[0] == "Factor_loop")

    {

        GramSym sym_factor = sym_stack[sym_stack.size() - 2];
        GramSym sym_factor_loop = sym_stack[sym_stack.size() - 3];

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        if (sym_factor_loop.txt_val == "")
        {

            gram_sym_con.txt_val = sym_stack.back().sym_name;

            if (TEMP == _symtab[sym_factor.pos.table_pos].getSymCat(sym_factor.pos.sym_pos))
            {

                gram_sym_con.pos = sym_factor.pos;
            }
            else
            {

                int sym_pos = _symtab[1].addSym(sym_factor.txt_val);
                std::string sym_name = _symtab[1].getSymName(sym_pos);

                _printTuple4(Tuple4({getNextState(), ":=", getArg(sym_factor.pos), "-", sym_name}));
                gram_sym_con.pos = {1, sym_pos};
            }
        }
        else
        {

            std::string arg2_name = getArg(sym_factor.pos);
            std::string arg1_name = getArg(sym_factor_loop.pos);

            gram_sym_con.txt_val = sym_factor_loop.txt_val;
            gram_sym_con.pos = sym_factor_loop.pos;

            if ("*" == sym_factor_loop.txt_val)
            {
                _printTuple4(Tuple4({getNextState(), "*", arg1_name, arg2_name, arg1_name}));
            }
            else
            {
                _printTuple4(Tuple4({getNextState(), "/", arg1_name, arg2_name, arg1_name}));
            }
        }

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Item" == prod.left)
    {

        GramSym sym_factor_loop = sym_stack[sym_stack.size() - 2];
        GramSym sym_factor = sym_stack.back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        if (sym_factor_loop.txt_val == "")
        {
            gram_sym_con.txt_val = "";
            gram_sym_con.pos = sym_factor.pos;
        }
        else
        {

            std::string arg2_name = getArg(sym_factor.pos);
            std::string arg1_name = getArg(sym_factor_loop.pos);

            if ("*" == sym_factor_loop.txt_val)
            {
                _printTuple4(Tuple4({getNextState(), "*", arg1_name, arg2_name, arg1_name}));
            }
            else
            {
                _printTuple4(Tuple4({getNextState(), "/", arg1_name, arg2_name, arg1_name}));
            }

            gram_sym_con.txt_val = sym_factor_loop.txt_val;
            gram_sym_con.pos = sym_factor_loop.pos;
        }

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Item_loop" == prod.left && prod.right[0] == "$")
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Item_loop" == prod.left && prod.right[0] == "Item_loop")

    {

        GramSym sym_item = sym_stack[sym_stack.size() - 2];
        GramSym sym_item_loop = sym_stack[sym_stack.size() - 3];

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        if (sym_item_loop.txt_val == "")
        {

            gram_sym_con.txt_val = sym_stack.back().sym_name;
            gram_sym_con.pos = sym_item.pos;
        }
        else
        {

            std::string arg2_name = getArg(sym_item.pos);
            std::string arg1_name = getArg(sym_item_loop.pos);

            if ("+" == sym_item_loop.txt_val)
            {
                _printTuple4(Tuple4({getNextState(), "+", arg1_name, arg2_name, arg1_name}));
            }
            else
            {
                _printTuple4(Tuple4({getNextState(), "-", arg1_name, arg2_name, arg1_name}));
            }

            gram_sym_con.txt_val = sym_stack.back().sym_name;
            gram_sym_con.pos = sym_item_loop.pos;
        }

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Add_expression" == prod.left)
    {

        GramSym sym_item_loop = sym_stack[sym_stack.size() - 2];
        GramSym sym_item = sym_stack.back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        if (sym_item_loop.txt_val == "")
        {
            gram_sym_con.txt_val = "";
            gram_sym_con.pos = sym_item.pos;
        }
        else
        {

            std::string arg2_name = getArg(sym_item.pos);
            std::string arg1_name = getArg(sym_item_loop.pos);

            if ("+" == sym_item_loop.txt_val)
            {
                _printTuple4(Tuple4({getNextState(), "+", arg1_name, arg2_name, arg1_name}));
            }
            else
            {
                _printTuple4(Tuple4({getNextState(), "-", arg1_name, arg2_name, arg1_name}));
            }

            gram_sym_con.txt_val = sym_item_loop.txt_val;
            gram_sym_con.pos = sym_item_loop.pos;
        }

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Add_expression_loop" == prod.left && prod.right[0] == "$")
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Add_expression_loop" == prod.left && prod.right[0] == "Add_expression_loop")

    {

        GramSym sym_add_exp = sym_stack[sym_stack.size() - 2];
        GramSym sym_add_exp_loop = sym_stack[sym_stack.size() - 3];

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        if (sym_add_exp_loop.txt_val == "")
        {

            gram_sym_con.txt_val = sym_stack.back().txt_val;
            gram_sym_con.pos = sym_add_exp.pos;
        }
        else
        {
            std::string arg2_name = getArg(sym_add_exp.pos);
            std::string arg1_name = getArg(sym_add_exp_loop.pos);
            std::string op_name = "j" + sym_add_exp_loop.txt_val;

            int next_state_num = getNextState();
            _printTuple4(Tuple4({next_state_num, op_name, arg1_name, arg2_name, std::to_string(next_state_num + 3)}));
            _printTuple4(Tuple4({getNextState(), ":=", "0", "-", arg1_name}));
            _printTuple4(Tuple4({getNextState(), "j", "-", "-", std::to_string(next_state_num + 4)}));
            _printTuple4(Tuple4({getNextState(), ":=", "1", "-", arg1_name}));

            std::string op = sym_add_exp_loop.txt_val;

            gram_sym_con.txt_val = sym_add_exp_loop.txt_val;
            gram_sym_con.pos = sym_add_exp_loop.pos;
        }

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Relop" == prod.left)
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = sym_stack.back().txt_val;

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Expression" == prod.left)
    {

        GramSym sym_add_exp_loop = sym_stack[sym_stack.size() - 2];
        GramSym sym_add_exp = sym_stack.back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        if (sym_add_exp_loop.txt_val == "")
        {
            gram_sym_con.txt_val = sym_add_exp.txt_val;
            gram_sym_con.pos = sym_add_exp.pos;
        }
        else
        {

            std::string arg2_name = getArg(sym_add_exp.pos);
            std::string arg1_name = getArg(sym_add_exp_loop.pos);
            std::string op_name = "j" + sym_add_exp_loop.txt_val;

            int next_state_num = getNextState();
            _printTuple4(Tuple4({next_state_num, op_name, arg1_name, arg2_name, std::to_string(next_state_num + 3)}));
            _printTuple4(Tuple4({getNextState(), ":=", "0", "-", arg1_name}));
            _printTuple4(Tuple4({getNextState(), "j", "-", "-", std::to_string(next_state_num + 4)}));
            _printTuple4(Tuple4({getNextState(), ":=", "1", "-", arg1_name}));

            gram_sym_con.txt_val = sym_add_exp_loop.txt_val;
            gram_sym_con.pos = sym_add_exp_loop.pos;
        }

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Internal_variable_stmt" == prod.left)
    {
        GramSym sym_identifier = sym_stack.back();

        SymTab &cur_table = _symtab[_cur_symtab_stack.back()];

        if (-1 != cur_table.findSym(sym_identifier.txt_val))
        {
            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "重复定义！" << std::endl;
            return false;
        }

        int search_sym_pos = _symtab[0].findSym(sym_identifier.txt_val);
        if (-1 != search_sym_pos && _symtab[0].getSymCat(search_sym_pos) == FUNCTION)
        {

            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "已经被定义为函数！" << std::endl;
            return false;
        }

        Symbol variable_sym;
        variable_sym.cat = VARIABLE;
        variable_sym.name = sym_identifier.txt_val;
        variable_sym.type = INT;
        variable_sym.val = "";

        int sym_pos = cur_table.addSym(variable_sym);

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        gram_sym_con.txt_val = sym_identifier.txt_val;
        gram_sym_con.pos = {_cur_symtab_stack.back(), sym_pos};

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Assign_sentence" == prod.left)
    {

        GramSym sym_exp = sym_stack[sym_stack.size() - 2];
        GramSym sym_identifier = sym_stack[sym_stack.size() - 4];

        int identifier_pos = -1;
        int identifier_layer = _cur_symtab_stack.size() - 1;
        for (; identifier_layer >= 0; --identifier_layer)
        {
            SymTab search_table = _symtab[_cur_symtab_stack[identifier_layer]];
            identifier_pos = search_table.findSym(sym_identifier.txt_val);
            if (identifier_pos != -1)
                break;
        }

        if (-1 == identifier_pos)
        {
            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "没有定义！" << std::endl;
            return false;
        }

        SymPos sp;
        sp.table_pos = _cur_symtab_stack[identifier_layer];
        sp.sym_pos = identifier_pos;
        std::string result_name = getArg(sp);
        std::string arg1_name = getArg(sym_exp.pos);
        _printTuple4(Tuple4({getNextState(), ":=", arg1_name, "-", result_name}));

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Create_Function_table" == prod.left)
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        GramSym sym_identifier = sym_stack.back();

        if (-1 != _symtab[0].findSym(sym_identifier.txt_val))
        {
            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "重复定义！" << prod.left << std::endl;
            return false;
        }

        SymTab new_table(FUNCTION_TABLE, sym_identifier.txt_val);
        _symtab.push_back(new_table);

        Symbol new_function_sym;
        new_function_sym.cat = FUNCTION;
        new_function_sym.name = sym_identifier.txt_val;
        new_function_sym.type = VOID;
        new_function_sym.func_sym_pos = _symtab.size() - 1;
        _symtab[0].addSym(new_function_sym);

        _cur_symtab_stack.push_back(_symtab.size() - 1);

        GramSym sym_ret_type = sym_stack[sym_stack.size() - 2];

        Symbol variable_sym;
        variable_sym.cat = RETURN_VAR;
        variable_sym.name = _symtab[_cur_symtab_stack.back()].getTabName() + "-ret_value";
        if (sym_ret_type.txt_val == "int")
            variable_sym.type = INT;
        else if (sym_ret_type.txt_val == "void")
            variable_sym.type = VOID;
        else
        {
            std::cerr << "函数返回类型只能有int 和 void！语义分析错误! " << prod.left << std::endl;
            return false;
        }
        variable_sym.val = "";

        if (sym_identifier.txt_val == "main")
            _main_line = peekNextState();

        _printTuple4(Tuple4({getNextState(), sym_identifier.txt_val, "-", "-", "-"}));

        _symtab[_cur_symtab_stack.back()].addSym(variable_sym);

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Exit_Function_table" == prod.left)
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        _cur_symtab_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Variavle_stmt" == prod.left)
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = ";";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Stmt_type" == prod.left && prod.right[0] == "Variavle_stmt")
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = ";";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Stmt" == prod.left && prod.right[0] == "int")
    {

        GramSym sym_identifier = sym_stack[sym_stack.size() - 2];
        GramSym sym_Stmt_type = sym_stack.back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        if (sym_Stmt_type.txt_val == ";")
        {

            int identifier_pos = -1;
            int identifier_layer = _cur_symtab_stack.size() - 1;
            for (; identifier_layer >= 0; --identifier_layer)
            {
                SymTab search_table = _symtab[_cur_symtab_stack[identifier_layer]];
                identifier_pos = search_table.findSym(sym_identifier.txt_val);
                if (identifier_pos != -1)
                    break;
            }

            if (-1 != identifier_pos)
            {
                std::cerr << "语义错误！！！" << sym_identifier.txt_val << "重复定义！" << prod.left << std::endl;
                return false;
            }

            Symbol variable_sym;
            variable_sym.cat = VARIABLE;
            variable_sym.name = sym_identifier.txt_val;
            variable_sym.type = INT;
            variable_sym.val = "";

            int sym_pos = _symtab[_cur_symtab_stack.back()].addSym(variable_sym);

            gram_sym_con.txt_val = sym_identifier.txt_val;
        }
        else
        {
            ;
        }

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Return_expression" == prod.left && prod.right[0] == "$")
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        if (VOID != _symtab[_cur_symtab_stack.back()].getSymType(0))
        {

            std::cerr << "语义错误！！！" << _symtab[_cur_symtab_stack.back()].getTabName()
                      << "函数定义的返回值为int类型。 但是return语句返回void类型！" << std::endl;
            return false;
        }

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Return_expression" == prod.left && prod.right[0] == "Expression")
    {

        if (INT != _symtab[_cur_symtab_stack.back()].getSymType(0))
        {

            std::cerr << "语义错误！！！" << _symtab[_cur_symtab_stack.back()].getTabName()
                      << "函数定义的返回值为void类型。 但是return语句返回int类型！" << std::endl;
            return false;
        }

        GramSym sym_exp = sym_stack.back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "int";
        gram_sym_con.pos = sym_exp.pos;

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Return_sentence" == prod.left)
    {
        GramSym sym_ret_exp = sym_stack[sym_stack.size() - 2];

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        SymTab &cur_function_table = _symtab[_cur_symtab_stack.back()];
        int function_pos = _symtab[0].findSym(cur_function_table.getTabName());
        Symbol &cur_function_sym = _symtab[0].getSym(function_pos);

        if ("" != sym_ret_exp.txt_val)
        {
            SymPos result_pos;
            result_pos.table_pos = _cur_symtab_stack.back();
            result_pos.sym_pos = 0;
            std::string result_name = getArg(result_pos);
            std::string arg1_name = getArg(sym_ret_exp.pos);

            _printTuple4(Tuple4({getNextState(), ":=", arg1_name, "-", result_name}));

            gram_sym_con.txt_val = "int";
            gram_sym_con.pos = sym_ret_exp.pos;
        }
        else
            gram_sym_con.txt_val = "";

        _printTuple4(Tuple4({getNextState(), "return", "-", "-", cur_function_table.getTabName()}));

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Parameter" == prod.left)
    {
        GramSym sym_identifier = sym_stack.back();

        SymTab &cur_table = _symtab[_cur_symtab_stack.back()];

        int table_pos = _symtab[0].findSym(cur_table.getTabName());

        if (-1 != cur_table.findSym(sym_identifier.txt_val))
        {
            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "函数参数 重复定义！" << std::endl;
            return false;
        }

        int search_sym_pos = _symtab[0].findSym(sym_identifier.txt_val);
        if (-1 != search_sym_pos && _symtab[0].getSymCat(search_sym_pos) == FUNCTION)
        {
            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "已经被定义为函数！" << prod.left << std::endl;
            return false;
        }

        Symbol variable_sym;
        variable_sym.cat = VARIABLE;
        variable_sym.name = sym_identifier.txt_val;
        variable_sym.type = INT;
        variable_sym.val = "";

        int sym_pos = cur_table.addSym(variable_sym);

        _symtab[0].getSym(table_pos).parameter_num++;

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        gram_sym_con.txt_val = sym_identifier.txt_val;
        gram_sym_con.pos = {_cur_symtab_stack.back(), sym_pos};

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Sentence_block" == prod.left)
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = std::to_string(peekNextState());

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("While_sentence_m2" == prod.left)
    {

        GramSym sym_exp = sym_stack[sym_stack.size() - 2];
        std::string sym_name = _symtab[sym_exp.pos.table_pos].getSymName(sym_exp.pos.sym_pos);

        _printTuple4(Tuple4({getNextState(), "j=", sym_name, "0", "---j="}));
        _bp_dot_pos.push_back(_tuple4_stack.size() - 1);

        _printTuple4(Tuple4({getNextState(), "j", "-", "-", "---j"}));
        _bp_dot_pos.push_back(_tuple4_stack.size() - 1);

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = std::to_string(peekNextState());

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("While_sentence_m1" == prod.left)
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = std::to_string(peekNextState());

        _bp_lv++;

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("While_sentence" == prod.left)

    {

        GramSym sym_while_sentence_m1 = sym_stack[sym_stack.size() - 6];
        GramSym sym_while_sentence_m2 = sym_stack[sym_stack.size() - 2];
        GramSym sym_sentence_block = sym_stack.back();
        _printTuple4(Tuple4({getNextState(), "j", "-", "-", sym_while_sentence_m1.txt_val}));

        int batch_pos = _bp_dot_pos.back();
        _bp_dot_pos.pop_back();
        _tuple4_stack[batch_pos].set(_tuple4_stack[batch_pos].num, _tuple4_stack[batch_pos].op,
                                     _tuple4_stack[batch_pos].arg1, _tuple4_stack[batch_pos].arg2, sym_while_sentence_m2.txt_val);

        batch_pos = _bp_dot_pos.back();
        _bp_dot_pos.pop_back();
        _tuple4_stack[batch_pos].set(_tuple4_stack[batch_pos].num, _tuple4_stack[batch_pos].op,
                                     _tuple4_stack[batch_pos].arg1, _tuple4_stack[batch_pos].arg2, std::to_string(peekNextState()));

        --_bp_lv;

        if (_bp_lv == 0)
        {
            for (auto iter = _tuple4_stack.begin(); iter != _tuple4_stack.end(); ++iter)
                _printTuple4(*iter);
            int len = _tuple4_stack.size();
            for (int i = 0; i < len; ++i)
                _tuple4_stack.pop_back();
        }

        /*int next_state_num = peekNextState();*/

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("If_sentence_m0" == prod.left)
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = std::to_string(peekNextState());

        _bp_lv++;

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("If_sentence_n" == prod.left)
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;

        _printTuple4(Tuple4({getNextState(), "j", "-", "-", "---j-if-n"}));
        _bp_dot_pos.push_back(_tuple4_stack.size() - 1);

        gram_sym_con.txt_val = std::to_string(peekNextState());

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("If_sentence_m1" == prod.left)
    {

        GramSym sym_exp = sym_stack[sym_stack.size() - 2];
        std::string sym_name = _symtab[sym_exp.pos.table_pos].getSymName(sym_exp.pos.sym_pos);

        _printTuple4(Tuple4({getNextState(), "j=", sym_name, "0", "---j="}));
        _bp_dot_pos.push_back(_tuple4_stack.size() - 1);

        _printTuple4(Tuple4({getNextState(), "j", "-", "-", "---j"}));
        _bp_dot_pos.push_back(_tuple4_stack.size() - 1);

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = std::to_string(peekNextState());

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("If_expression" == prod.left && prod.right[0] == "$")
    {

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("If_expression" == prod.left && prod.right[0] == "If_sentence_n")

    {
        GramSym sym_if_sentence_n = sym_stack[sym_stack.size() - 3];

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = sym_if_sentence_n.txt_val;

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("If_sentence" == prod.left)

    {
        GramSym sym_if_sentence_m1 = sym_stack[sym_stack.size() - 3];
        GramSym sym_if_expression = sym_stack.back();

        if ("" == sym_if_expression.txt_val)
        {
            int batch_pos = _bp_dot_pos.back();
            _bp_dot_pos.pop_back();
            _tuple4_stack[batch_pos].set(_tuple4_stack[batch_pos].num, _tuple4_stack[batch_pos].op,
                                         _tuple4_stack[batch_pos].arg1, _tuple4_stack[batch_pos].arg2, sym_if_sentence_m1.txt_val);

            batch_pos = _bp_dot_pos.back();
            _bp_dot_pos.pop_back();
            _tuple4_stack[batch_pos].set(_tuple4_stack[batch_pos].num, _tuple4_stack[batch_pos].op,
                                         _tuple4_stack[batch_pos].arg1, _tuple4_stack[batch_pos].arg2, std::to_string(peekNextState()));
        }
        else
        {
            int batch_pos = _bp_dot_pos.back();
            _bp_dot_pos.pop_back();
            _tuple4_stack[batch_pos].set(_tuple4_stack[batch_pos].num, _tuple4_stack[batch_pos].op,
                                         _tuple4_stack[batch_pos].arg1, _tuple4_stack[batch_pos].arg2, std::to_string(peekNextState()));

            batch_pos = _bp_dot_pos.back();
            _bp_dot_pos.pop_back();
            _tuple4_stack[batch_pos].set(_tuple4_stack[batch_pos].num, _tuple4_stack[batch_pos].op,
                                         _tuple4_stack[batch_pos].arg1, _tuple4_stack[batch_pos].arg2, sym_if_sentence_m1.txt_val);

            batch_pos = _bp_dot_pos.back();
            _bp_dot_pos.pop_back();
            _tuple4_stack[batch_pos].set(_tuple4_stack[batch_pos].num, _tuple4_stack[batch_pos].op,
                                         _tuple4_stack[batch_pos].arg1, _tuple4_stack[batch_pos].arg2, sym_if_expression.txt_val);
        }

        --_bp_lv;

        if (_bp_lv == 0)
        {
            for (auto iter = _tuple4_stack.begin(); iter != _tuple4_stack.end(); ++iter)
                _printTuple4(*iter);
            int len = _tuple4_stack.size();
            for (int i = 0; i < len; ++i)
                _tuple4_stack.pop_back();
        }

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Call_func_check" == prod.left)
    {
        GramSym sym_identifier = sym_stack[sym_stack.size() - 2];

        int identifier_pos = -1;
        int identifier_layer = _cur_symtab_stack.size() - 1;
        for (; identifier_layer >= 0; --identifier_layer)
        {
            SymTab search_table = _symtab[_cur_symtab_stack[identifier_layer]];
            identifier_pos = search_table.findSym(sym_identifier.txt_val);
            if (identifier_pos != -1)
                break;
        }
        if (-1 == identifier_pos)
        {
            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "没有定义！" << std::endl;
            return false;
        }

        if (FUNCTION != _symtab[_cur_symtab_stack[identifier_layer]].getSymCat(identifier_pos))
        {
            std::cerr << "语义错误！！！" << sym_identifier.txt_val << "不是函数，无法被调用！" << std::endl;
            return false;
        }

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";
        gram_sym_con.pos = {_cur_symtab_stack[identifier_layer], identifier_pos};

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }
    else if ("Expression_loop" == prod.left && prod.right[0] == "$")
    {
        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "0";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Expression_loop" == prod.left && prod.right[0] == "Expression_loop")
    {
        GramSym sym_call_func_check = sym_stack[sym_stack.size() - 4];
        GramSym sym_exp_loop = sym_stack[sym_stack.size() - 3];
        GramSym sym_exp = sym_stack[sym_stack.size() - 2];
        Symbol &called_function = _symtab[sym_call_func_check.pos.table_pos].getSym(sym_call_func_check.pos.sym_pos);
        int parameters_num = called_function.parameter_num;
        int passed_parameters_num = std::stoi(sym_exp_loop.txt_val);
        if (passed_parameters_num >= parameters_num)
        {
            std::cerr << "语义错误！！！" << called_function.name << "传递的参数过多！" << std::endl;
            return false;
        }

        int table_pos = called_function.func_sym_pos;

        SymPos result_pos;
        result_pos.table_pos = table_pos;
        result_pos.sym_pos = passed_parameters_num + 1;

        std::string result_name = getArg(result_pos, true);

        std::string arg1_name = getArg(sym_exp.pos);

        _printTuple4(Tuple4({getNextState(), ":=", arg1_name, "-", result_name}));

        passed_parameters_num++;

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = std::to_string(passed_parameters_num);

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Actual_parameter_list" == prod.left && prod.right[0] == "Expression_loop")
    {
        GramSym sym_call_func_check = sym_stack[sym_stack.size() - 3];
        GramSym sym_exp_loop = sym_stack[sym_stack.size() - 2];
        GramSym sym_exp = sym_stack.back();
        Symbol &called_function = _symtab[sym_call_func_check.pos.table_pos].getSym(sym_call_func_check.pos.sym_pos);
        int parameters_num = called_function.parameter_num;
        int passed_parameters_num = std::stoi(sym_exp_loop.txt_val);
        if (passed_parameters_num >= parameters_num)
        {
            std::cerr << "语义错误！！！" << called_function.name << "传递的参数过多！" << std::endl;
            return false;
        }

        int table_pos = called_function.func_sym_pos;

        SymPos result_pos;
        result_pos.table_pos = table_pos;
        result_pos.sym_pos = passed_parameters_num + 1;

        std::string result_name = getArg(result_pos, true);

        std::string arg1_name = getArg(sym_exp.pos);

        _printTuple4(Tuple4({getNextState(), ":=", arg1_name, "-", result_name}));

        passed_parameters_num++;

        if (passed_parameters_num < parameters_num)
        {
            std::cerr << "语义错误！！！" << called_function.name << "传递的参数过少！" << std::endl;
            return false;
        }

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = std::to_string(passed_parameters_num);

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Actual_parameter_list" == prod.left && prod.right[0] == "$")
    {
        GramSym sym_call_func_check = sym_stack.back();
        Symbol &called_function = _symtab[sym_call_func_check.pos.table_pos].getSym(sym_call_func_check.pos.sym_pos);
        int parameters_num = called_function.parameter_num;

        if (0 != parameters_num)
        {
            std::cerr << "语义错误！！！" << called_function.name << "函数的调用需要参数，但是没有传入参数！" << std::endl;
            return false;
        }

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "0";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("Call_func" == prod.left)
    {
        GramSym sym_identifier = sym_stack[sym_stack.size() - 5];
        GramSym sym_call_func_check = sym_stack[sym_stack.size() - 3];
        _printTuple4(Tuple4({getNextState(), "call", "-", "-", sym_identifier.txt_val}));

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";
        gram_sym_con.pos = sym_call_func_check.pos;

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();

        sym_stack.push_back(gram_sym_con);
    }

    else if ("FTYPE" == prod.left && prod.right[0] == "Call_func")
    {
        GramSym sym_call_func = sym_stack.back();

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "call_func";
        gram_sym_con.pos = sym_call_func.pos;

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else if ("Program" == prod.left)
    {
        if (_main_line == -1)
        {
            std::cerr << "语义错误！！！"
                      << "main函数没有定义！" << std::endl;
            return false;
        }
        _printTuple4();
        std::cout << "中间代码生成完毕！" << std::endl;

        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    else
    {
        GramSym gram_sym_con;
        gram_sym_con.sym_name = prod.left;
        gram_sym_con.txt_val = "";

        int len = 0;
        if ("$" != prod.right[0])
            len = prod.right.size();

        for (int i = 0; i < len; ++i)
            sym_stack.pop_back();
        sym_stack.push_back(gram_sym_con);
    }

    return true;
}