#include "syntactic.h"

std::vector<std::string> splitStr(const std::string &str, const std::string &pat)
{
    std::vector<std::string> res;
    if (str == "")
        return res;

    std::string str2 = str + pat;
    size_t pos = str2.find(pat);

    while (pos != str2.npos)
    {
        std::string temp = str2.substr(0, pos);
        res.push_back(temp);

        str2 = str2.substr(pos + 1, str2.size());
        pos = str2.find(pat);
    }

    return res;
}

std::set<std::string> Syntactic::_getProdFirstSet(const std::vector<std::string> &sym_string)
{
    std::set<std::string> first;
    for (auto iter1 = sym_string.begin(); iter1 != sym_string.end(); ++iter1)
    {

        if (!_isNonTerminalSym(*iter1))
        {
            first.insert(*iter1);
            break;
        }

        for (auto iter2 = _first_map[*iter1].begin(); iter2 != _first_map[*iter1].end(); ++iter2)
        {
            if ((*iter2) != "$" && first.find(*iter2) == first.end())
                first.insert(*iter2);
        }

        if (_first_map[*iter1].find("$") == _first_map[*iter1].end())
            break;

        if (iter1 + 1 == sym_string.end() && first.find("$") == first.end())
            first.insert("$");
    }
    return first;
}

bool Syntactic::_genProd(const std::string path)
{
    std::ifstream gram_r_hd;

    gram_r_hd.open(path);

    if (!gram_r_hd.is_open())
    {
        std::cerr << "语法分析过程中，grammar文件打开失败！" << std::endl;
        return false;
    }

    std::string gram_str;
    while (!gram_r_hd.eof())
    {
        getline(gram_r_hd, gram_str);
        if (gram_str.length() == 0)
        {
            continue;
        }

        std::string left;
        std::vector<std::string> right;

        size_t left_pos = gram_str.find_first_of("->");
        if (left_pos < 0)
        {
            std::cerr << "grammar出现错误，某个产生式没有\" -> \" 符号，请规范！" << std::endl;
            gram_r_hd.close();
            return false;
        }
        left = gram_str.substr(0, left_pos);

        gram_str = gram_str.substr(left_pos + 2);

        while (true)
        {
            size_t right_pos = gram_str.find_first_of("|");
            if (right_pos == std::string::npos)
            {
                right.push_back(gram_str);

                std::vector<std::string> split_prod = splitStr(gram_str, " ");
                _prods.push_back({left, split_prod});

                for (auto iter = split_prod.begin(); iter != split_prod.end(); ++iter)
                    if (!_isNonTerminalSym(*iter) && (*iter) != "$")
                    {
                        _gram_sym.insert(*iter);
                    }

                break;
            }

            right.push_back(gram_str.substr(0, right_pos));

            std::vector<std::string> split_prod = splitStr(gram_str.substr(0, right_pos), " ");
            _prods.push_back({left, split_prod});

            for (auto iter = split_prod.begin(); iter != split_prod.end(); ++iter)
                if (!_isNonTerminalSym(*iter) && (*iter) != "$")
                {
                    _gram_sym.insert(*iter);
                }

            gram_str = gram_str.substr(right_pos + 1);
        }

        _grams.push_back({left, right});
    }

    gram_r_hd.close();
    return true;
}

void Syntactic::_genAugGram()
{

    std::string left = "Start";
    std::vector<std::string> right;
    right.push_back(_prods[0].left);
    _prods.insert(_prods.begin(), {left, right});
    return;
}

void Syntactic::_genFirstSet()
{
    for (auto iter = _prods.begin(); iter != _prods.end(); ++iter)
    {

        _first_map.insert({iter->left, std::set<std::string>{}});
        _follow_map.insert({iter->left, std::set<std::string>{}});
        if (iter->right[0] == "$")
        {
            if (_first_map[iter->left].find("$") == _first_map[iter->left].end())
            {
                _first_map[iter->left].insert("$");
            }
        }
    }

    while (true)
    {
        bool changed = false;
        for (auto iter1 = _prods.begin(); iter1 != _prods.end(); ++iter1)
        {
            for (auto iter2 = iter1->right.begin(); iter2 != iter1->right.end(); ++iter2)
            {
                if ((*iter2) == "$")
                {
                    break;
                }

                if (!_isNonTerminalSym(*iter2))
                {
                    if (_first_map[iter1->left].find(*iter2) == _first_map[iter1->left].end())
                    {
                        _first_map[iter1->left].insert(*iter2);
                        changed = true;
                    }
                    break;
                }

                for (auto iter3 = _first_map[*iter2].begin(); iter3 != _first_map[*iter2].end(); ++iter3)
                {
                    if ((*iter3) != "$" && _first_map[iter1->left].find(*iter3) == _first_map[iter1->left].end())
                    {
                        _first_map[iter1->left].insert(*iter3);
                        changed = true;
                    }
                }

                if (_first_map[*iter2].find("$") == _first_map[*iter2].end())
                {
                    break;
                }

                if (iter2 + 1 == iter1->right.end() && _first_map[iter1->left].find("$") == _first_map[iter1->left].end())
                {
                    _first_map[iter1->left].insert("$");
                    changed = true;
                }
            }
        }

        if (!changed)
        {
            break;
        }
    }
    return;
}

void Syntactic::_genFollowSet()
{

    _follow_map[_prods[0].left].insert("#");

    while (true)
    {
        bool changed = false;
        for (auto iter1 = _prods.begin(); iter1 != _prods.end(); ++iter1)
        {
            for (auto iter2 = iter1->right.begin(); iter2 != iter1->right.end(); ++iter2)
            {
                if (!_isNonTerminalSym(*iter2))
                {
                    continue;
                }

                std::set<std::string> first = _getProdFirstSet(std::vector<std::string>(iter2 + 1, iter1->right.end()));
                for (auto iter = first.begin(); iter != first.end(); ++iter)
                {
                    if ((*iter) != "$" && _follow_map[*iter2].find(*iter) == _follow_map[*iter2].end())
                    {
                        _follow_map[*iter2].insert(*iter);
                        changed = true;
                    }
                }
                if (first.empty() || first.find("$") != first.end())
                {
                    for (auto iter = _follow_map[iter1->left].begin(); iter != _follow_map[iter1->left].end(); ++iter)
                    {
                        if (_follow_map[*iter2].find(*iter) == _follow_map[*iter2].end())
                        {
                            _follow_map[*iter2].insert(*iter);
                            changed = true;
                        }
                    }
                }
            }
        }

        if (!changed)
            break;
    }

    return;
}

void Syntactic::_genGramSymSet()
{

    for (auto iter = _first_map.begin(); iter != _first_map.end(); ++iter)
        _gram_sym.insert(iter->first);

    _gram_sym.insert("#");
    return;
}

void Syntactic::_genLRItems()
{
    for (auto iter = _prods.begin(); iter != _prods.end(); ++iter)
    {
        if (iter->right[0] == "$")
        {
            _lr_items.insert(LRItem{iter - _prods.begin(), -1});
            continue;
        }
        int prod_len = iter->right.size();
        for (int cnt = 0; cnt <= prod_len; ++cnt)
        {

            _lr_items.insert(LRItem{iter - _prods.begin(), cnt});
        }
    }
    return;
}

std::set<LRItem> Syntactic::_genItemClosureSet(const LRItem &input_item)
{
    std::vector<LRItem> item_stack;
    item_stack.push_back(input_item);
    std::set<LRItem> item_set;

    while (!item_stack.empty())
    {

        LRItem item = item_stack[item_stack.size() - 1];
        item_stack.pop_back();

        item_set.insert(item);

        if (-1 == item.dot_pos)
        {
            continue;
        }

        if (item.dot_pos == _prods[item.prod_id].right.size())
        {
            continue;
        }

        if (!_isNonTerminalSym(_prods[item.prod_id].right[item.dot_pos]))
        {
            continue;
        }

        std::string cur_sym = _prods[item.prod_id].right[item.dot_pos];

        for (auto iter = _lr_items.begin(); iter != _lr_items.end(); ++iter)
        {

            if (_prods[iter->prod_id].left == cur_sym && (iter->dot_pos == 0 || iter->dot_pos == -1))
            {
                if (item_set.find(*iter) == item_set.end())
                {
                    item_stack.push_back(*iter);
                }
            }
        }
    }

    return item_set;
}

std::set<LRItem> Syntactic::_genItemsClosureSet(const std::set<LRItem> &items)
{
    std::vector<LRItem> item_stack;
    for (auto iter = items.begin(); iter != items.end(); ++iter)
        item_stack.push_back(*iter);

    std::set<LRItem> item_set;

    while (!item_stack.empty())
    {

        LRItem item = item_stack[item_stack.size() - 1];
        item_stack.pop_back();

        item_set.insert(item);

        if (-1 == item.dot_pos)
        {
            continue;
        }

        if (item.dot_pos == _prods[item.prod_id].right.size())
        {
            continue;
        }

        if (!_isNonTerminalSym(_prods[item.prod_id].right[item.dot_pos]))
        {
            continue;
        }

        std::string cur_sym = _prods[item.prod_id].right[item.dot_pos];

        for (auto iter = _lr_items.begin(); iter != _lr_items.end(); ++iter)
        {

            if (_prods[iter->prod_id].left == cur_sym && (iter->dot_pos == 0 || iter->dot_pos == -1))
            {
                if (item_set.find(*iter) == item_set.end())
                {
                    item_stack.push_back(*iter);
                }
            }
        }
    }

    return item_set;
}

bool Syntactic::_genNormalFamilySet()
{

    std::vector<std::set<LRItem>> item_stack;
    item_stack.push_back(_genItemClosureSet(*_lr_items.begin()));
    _norm_families.push_back(item_stack[0]);

    while (!item_stack.empty())
    {

        std::set<LRItem> item = item_stack[item_stack.size() - 1];
        item_stack.pop_back();

        int cur_state = find(_norm_families.begin(), _norm_families.end(), item) - _norm_families.begin();

        for (auto iter1 = item.begin(); iter1 != item.end(); ++iter1)
        {

            if (-1 == iter1->dot_pos || (_prods[iter1->prod_id].right.size() == iter1->dot_pos))
            {

                std::string sym = _prods[iter1->prod_id].left;

                for (auto iter2 = _follow_map[sym].begin(); iter2 != _follow_map[sym].end(); ++iter2)
                {

                    if (_action_goto_map.find({cur_state, *iter2}) == _action_goto_map.end())
                    {
                        _action_goto_map.insert({{cur_state, *iter2}, {CONCLUDE, iter1->prod_id}});
                    }
                    else
                    {

                        if (!(_action_goto_map[{cur_state, *iter2}] == SLROp{CONCLUDE, iter1->prod_id}))
                        {
                            _printSLRError(item);
                            return false;
                        }
                    }
                }
            }
            else
            {

                std::string cur_right_sym = _prods[iter1->prod_id].right[iter1->dot_pos];

                std::set<LRItem> items;
                for (auto iter2 = item.begin(); iter2 != item.end(); ++iter2)
                {

                    if (iter2->dot_pos == -1 || (iter2->dot_pos == _prods[iter2->prod_id].right.size()))
                    {
                        continue;
                    }

                    if (_prods[iter1->prod_id].right[iter1->dot_pos] ==
                        _prods[iter2->prod_id].right[iter2->dot_pos])
                    {
                        items.insert({iter2->prod_id, iter2->dot_pos + 1});
                    }
                }
                std::set<LRItem> next_normal_family = _genItemsClosureSet(items);

                if (find(_norm_families.begin(), _norm_families.end(), next_normal_family) == _norm_families.end())
                {
                    _norm_families.push_back(next_normal_family);
                    item_stack.push_back(next_normal_family);
                }
                int next_state = find(_norm_families.begin(), _norm_families.end(), next_normal_family) - _norm_families.begin();

                if (_action_goto_map.find({cur_state, cur_right_sym}) == _action_goto_map.end())
                {
                    _action_goto_map.insert({{cur_state, cur_right_sym}, {MOVE, next_state}});
                }
                else
                {
                    if (!(_action_goto_map[{cur_state, cur_right_sym}] == SLROp{MOVE, next_state}))
                    {
                        _printSLRError(item);
                        return false;
                    }
                }
            }
        }
    }
    int cur_state2 = -1;

    for (auto iter1 = _norm_families.begin(); iter1 != _norm_families.end(); ++iter1)
    {
        for (auto iter2 = iter1->begin(); iter2 != iter1->end(); ++iter2)
            if (LRItem(0, 1) == *iter2)
            {
                cur_state2 = iter1 - _norm_families.begin();
                break;
            }
        if (cur_state2 >= 0)
            break;
    }
    std::set<LRItem> item = {LRItem(0, 1)};

    _action_goto_map[{cur_state2, "#"}] = {ACCEPT, cur_state2};

    return true;
}

bool Syntactic::_buildGram()
{
    _genProd();
    _genAugGram();
    _genFirstSet();
    _genFollowSet();
    _genGramSymSet();
    _genLRItems();
    return _genNormalFamilySet();
}

void Syntactic::_printGrams()
{
    for (auto iter1 = _grams.begin(); iter1 != _grams.end(); iter1++)
    {
        std::cout << (*iter1).left << "->";
        for (auto iter2 = (iter1->right).begin(); iter2 != (iter1->right).end(); iter2++)
            std::cout << (*iter2) << "|";
        std::cout << "\b " << std::endl;
    }
    return;
}

void Syntactic::_printProds(const std::string path)
{
    std::ofstream w_hd;
    w_hd.open(path);

    if (!w_hd.is_open())
    {
        std::cerr << "语法分析过程中，" << path << "文件打开失败！" << std::endl;
        return;
    }

    for (auto iter1 = _prods.begin(); iter1 != _prods.end(); iter1++)
    {
        w_hd << (*iter1).left << "->";
        for (auto iter2 = (iter1->right).begin(); iter2 != (iter1->right).end(); iter2++)
        {
            w_hd << (*iter2) << ' ';
        }
        w_hd << std::endl;
    }

    return;
}

void Syntactic::_printFirst(const std::string path)
{
    std::ofstream w_hd;
    w_hd.open(path);

    if (!w_hd.is_open())
    {
        std::cerr << "语法分析过程中，" << path << "文件打开失败！" << std::endl;
        return;
    }
    w_hd << "First集合：" << std::endl;
    for (auto iter1 = _first_map.begin(); iter1 != _first_map.end(); ++iter1)
    {

        w_hd << (*iter1).first << ": ";
        int cnt = 0;
        int size_ = (*iter1).second.size();
        for (auto iter2 = (*iter1).second.begin(); iter2 != (*iter1).second.end(); ++iter2)
        {
            w_hd << (*iter2);
            if (size_ > cnt + 1)
                w_hd << " ";
        }
        w_hd << std::endl;
    }
    return;
}

void Syntactic::_printFollow(const std::string path)
{
    std::ofstream w_hd;
    w_hd.open(path);

    if (!w_hd.is_open())
    {
        std::cerr << "语法分析过程中，" << path << "文件打开失败！" << std::endl;
        return;
    }
    w_hd << "Follow集合：" << std::endl;
    for (auto iter1 = _follow_map.begin(); iter1 != _follow_map.end(); ++iter1)
    {

        w_hd << (*iter1).first << ": ";
        int cnt = 0;
        int size_ = (*iter1).second.size();
        for (auto iter2 = (*iter1).second.begin(); iter2 != (*iter1).second.end(); ++iter2)
        {
            w_hd << (*iter2);
            if (size_ > cnt + 1)
                w_hd << " ";
        }
        w_hd << std::endl;
    }
    return;
}

void Syntactic::_printGramSymSet(const std::string path)
{
    std::ofstream w_hd;
    w_hd.open(path);
    if (!w_hd.is_open())
    {
        std::cerr << "语法分析过程中，" << path << "文件打开失败！" << std::endl;
        return;
    }

    w_hd << "文法符号（终结符与非终结符）：" << std::endl;
    for (auto iter = _gram_sym.begin(); iter != _gram_sym.end(); ++iter)
    {
        w_hd << *iter << std::endl;
    }
    return;
}

void Syntactic::_printLRItems(const std::string path)
{
    std::ofstream w_hd;
    w_hd.open(path);
    if (!w_hd.is_open())
    {
        std::cerr << "语法分析过程中，" << path << "文件打开失败！" << std::endl;
        return;
    }
    w_hd << "LR0项目：" << std::endl;
    for (auto iter1 = _lr_items.begin(); iter1 != _lr_items.end(); ++iter1)
    {

        w_hd << _prods[iter1->prod_id].left << "->";

        if (-1 == iter1->dot_pos)
        {
            w_hd << "·" << std::endl;
            continue;
        }

        int prod_len = _prods[iter1->prod_id].right.size();

        for (auto iter2 = _prods[iter1->prod_id].right.begin();
             iter2 != _prods[iter1->prod_id].right.end(); iter2++)
        {
            if (iter1->dot_pos == (iter2 - _prods[iter1->prod_id].right.begin()))
                w_hd << "· ";
            w_hd << *iter2 << " ";

            if (iter1->dot_pos == prod_len &&
                (_prods[iter1->prod_id].right.end() - iter2 == 1))
                w_hd << "·";
        }

        w_hd << std::endl;
    }
    return;
}

void Syntactic::_printClosure()
{
    std::cout << "Closure项目：" << std::endl;
    for (auto iter = _lr_items.begin(); iter != _lr_items.end(); ++iter)
    {
        std::set<LRItem> lr = _genItemClosureSet(*iter);

        for (auto iter1 = lr.begin(); iter1 != lr.end(); ++iter1)
        {
            std::cout << _prods[iter1->prod_id].left << "->";

            if (-1 == iter1->dot_pos)
            {
                std::cout << "·" << std::endl;
                continue;
            }

            int prod_len = _prods[iter1->prod_id].right.size();

            for (auto iter2 = _prods[iter1->prod_id].right.begin();
                 iter2 != _prods[iter1->prod_id].right.end(); iter2++)
            {
                if (iter1->dot_pos == (iter2 - _prods[iter1->prod_id].right.begin()))
                    std::cout << "· ";
                std::cout << *iter2 << " ";

                if (iter1->dot_pos == prod_len &&
                    (_prods[iter1->prod_id].right.end() - iter2 == 1))
                    std::cout << "·";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }
}

void Syntactic::_printSLRError(const std::set<LRItem> &normal_family)
{
    std::cerr << "不是SLR文法！！！ 冲突项目的规范族：" << std::endl;
    std::set<LRItem> lr = normal_family;

    for (auto iter1 = lr.begin(); iter1 != lr.end(); ++iter1)
    {
        std::cerr << _prods[iter1->prod_id].left << "->";

        if (-1 == iter1->dot_pos)
        {
            std::cerr << "·" << std::endl;
            continue;
        }

        int prod_len = _prods[iter1->prod_id].right.size();

        for (auto iter2 = _prods[iter1->prod_id].right.begin();
             iter2 != _prods[iter1->prod_id].right.end(); iter2++)
        {
            if (iter1->dot_pos == (iter2 - _prods[iter1->prod_id].right.begin()))
                std::cerr << "· ";
            std::cerr << *iter2 << " ";

            if (iter1->dot_pos == prod_len &&
                (_prods[iter1->prod_id].right.end() - iter2 == 1))
                std::cout << "·";
        }
        std::cerr << std::endl;
    }
    std::cerr << std::endl;
}

void Syntactic::_printNormFamiliySet(const std::string path)
{
    std::ofstream w_hd;
    w_hd.open(path);
    if (!w_hd.is_open())
    {
        std::cerr << "语法分析过程中，" << path << "文件打开失败！" << std::endl;
        return;
    }
    w_hd << "项目集规范族：" << std::endl;
    for (auto iter1 = _norm_families.begin(); iter1 != _norm_families.end(); ++iter1)
    {
        w_hd << "规范族 " << iter1 - _norm_families.begin() << " : " << std::endl;
        for (auto iter2 = iter1->begin(); iter2 != iter1->end(); ++iter2)
        {

            w_hd << _prods[iter2->prod_id].left << "->";

            if (-1 == iter2->dot_pos)
            {
                w_hd << "·" << std::endl;
                continue;
            }

            int prod_len = _prods[iter2->prod_id].right.size();

            for (auto iter3 = _prods[iter2->prod_id].right.begin();
                 iter3 != _prods[iter2->prod_id].right.end(); iter3++)
            {
                if (iter2->dot_pos == (iter3 - _prods[iter2->prod_id].right.begin()))
                    w_hd << "· ";
                w_hd << *iter3 << " ";

                if (iter2->dot_pos == prod_len &&
                    (_prods[iter2->prod_id].right.end() - iter3 == 1))
                    w_hd << "·";
            }
            w_hd << std::endl;
        }
        w_hd << std::endl;
    }
}

void Syntactic::_printActionGoto(const std::string path)
{
    std::ofstream w_hd;
    w_hd.open(path);
    if (!w_hd.is_open())
    {
        std::cerr << "语法分析过程中，" << path << "文件打开失败！" << std::endl;
        return;
    }

    w_hd << "  ";
    for (auto iter = _gram_sym.begin(); iter != _gram_sym.end(); ++iter)
    {
        if (_isNonTerminalSym(*iter))
        {
            continue;
        }
        if ("," == (*iter))
            w_hd << ","
                 << "，";
        else
            w_hd << "," << (*iter);
    }
    for (auto iter = _gram_sym.begin(); iter != _gram_sym.end(); ++iter)
    {
        if (!_isNonTerminalSym(*iter))
        {
            continue;
        }
        w_hd << "," << (*iter);
    }
    w_hd << std::endl;

    for (unsigned int state = 0; state < _norm_families.size(); ++state)
    {
        w_hd << "state " << state;
        for (auto iter = _gram_sym.begin(); iter != _gram_sym.end(); ++iter)
        {
            if (_isNonTerminalSym(*iter))
                continue;

            if (_action_goto_map.find({state, *iter}) == _action_goto_map.end())
                w_hd << ",error";
            else
            {
                int next_state = _action_goto_map[{state, *iter}].state;
                int op = _action_goto_map[{state, (*iter)}].op;

                if (op == MOVE)
                    w_hd << ",s" << next_state;
                else if (op == CONCLUDE)
                {
                    w_hd << ",r" << next_state;
                }
                else if (op == ACCEPT)
                    w_hd << ",acc";
                else
                    w_hd << ",???";
            }
        }
        for (auto iter = _gram_sym.begin(); iter != _gram_sym.end(); ++iter)
        {
            if (!_isNonTerminalSym(*iter))
                continue;

            if (_action_goto_map.find({state, *iter}) == _action_goto_map.end())
                w_hd << ",error";
            else
            {
                int next_state = _action_goto_map[{state, *iter}].state;
                int op = _action_goto_map[{state, *iter}].op;

                if (op == MOVE)
                    w_hd << ",s" << next_state;
                else if (op == CONCLUDE)
                {
                    w_hd << ",r" << next_state;
                }
                else if (op == ACCEPT)
                    w_hd << ",acc";
                else
                    w_hd << ",???";
            }
        }

        w_hd << std::endl;
    }
    return;
}

void Syntactic::_print_buildGramDetails()
{
    if (_print)
    {
        _printProds();
        _printFirst();
        _printFollow();
        _printGramSymSet();
        _printLRItems();
        _printNormFamiliySet();
        _printActionGoto();
    }
    return;
}

void Syntactic::_printProcess(int step, const SLROp &sl_op)
{
    if (!_print)
        return;
    _syntactic_w_hd << step << ',';

    for (auto iter = _state_stack.begin(); iter != _state_stack.end(); ++iter)
    {
        _syntactic_w_hd << *iter << ' ';
    }
    _syntactic_w_hd << ',';

    for (auto iter = _move_con_stack.begin(); iter != _move_con_stack.end(); ++iter)
    {
        if ("," == *iter)
            _syntactic_w_hd << "，";
        else
            _syntactic_w_hd << *iter << ' ';
    }
    _syntactic_w_hd << ',';

    if (sl_op.op == MOVE)
        _syntactic_w_hd << "移进";
    else if (sl_op.op == ACCEPT)
        _syntactic_w_hd << "接受";
    else if (sl_op.op == CONCLUDE)
    {
        _syntactic_w_hd << "规约： " << _prods[sl_op.state].left << "->";
        for (auto iter = _prods[sl_op.state].right.begin(); iter != _prods[sl_op.state].right.end(); ++iter)
        {
            if ("," == *iter)
                _syntactic_w_hd << "，";
            else
                _syntactic_w_hd << *iter << ' ';
        }
    }

    _syntactic_w_hd << std::endl;
}

bool Syntactic::_isNonTerminalSym(const std::string &sym)
{
    if (sym.length() == 0)
        return false;
    if (sym[0] >= 'A' && sym[0] <= 'Z')
        return true;
    return false;
}

Syntactic::Syntactic(bool print, const std::string path)
{
    _print = print;
    if (_print)
    {
        _syntactic_w_hd.open(path);
        if (!_syntactic_w_hd.is_open())
        {
            std::cerr << "语法分析过程中，显示语法分析过程文件打开失败！" << std::endl;
        }
        else
            _syntactic_w_hd << "步骤, 状态栈, 符号栈, 动作说明" << std::endl;
    }
    _buildGram();
    _print_buildGramDetails();
}

Syntactic::~Syntactic()
{
    if (_syntactic_w_hd.is_open())
        _syntactic_w_hd.close();
}

bool LRItem::operator==(const LRItem &item) const
{
    return (this->prod_id == item.prod_id) && (this->dot_pos == item.dot_pos);
}

bool LRItem::operator<(const LRItem &item) const
{
    return this->prod_id < item.prod_id || this->prod_id == item.prod_id && this->dot_pos < item.dot_pos;
}

bool SLROp::operator==(const SLROp &operation) const
{
    return (this->op == operation.op) && (this->state == operation.state);
}

bool Syntactic::analyze(const std::string code_path, bool skip_parse)
{
    _state_stack.push_back(0);
    _move_con_stack.push_back("#");

    _gram_sym_stack.push_back({"Program"});

    Tokenizer tokenizer;
    if (!tokenizer.isReady(code_path, true))
        return false;

    int sytactic_step = 0;
    while (true)
    {

        Word get_word = tokenizer.getWord();
        std::string word_string = get_word.word_string;
        if (get_word.type == LUNKNOWN)
        {
            std::cerr << "词法分析器过程中，发生unknown错误！" << std::endl;
            std::cerr << get_word.val << std::endl;
        }

        if (skip_parse)
        {
            if (get_word.type == LEOF)
            {
                return true;
            }

            continue;
        }

        while (true)
        {

            int cur_state = _state_stack[_state_stack.size() - 1];

            if (_action_goto_map.find({cur_state, word_string}) == _action_goto_map.end())
            {
                std::cerr << "语法分析器过程中，发生错误！" << std::endl;
                std::cerr << get_word.val << std::endl;
                std::cerr << "state：" << cur_state << " 与 " << word_string << " 在action_goto_table 中不含对应操作!" << std::endl;
                return false;
            }

            if (MOVE == _action_goto_map[{cur_state, word_string}].op)
            {
                _state_stack.push_back(_action_goto_map[{cur_state, word_string}].state);
                _move_con_stack.push_back(word_string);
                _printProcess(sytactic_step, _action_goto_map[{cur_state, word_string}]);
                sytactic_step++;
                _gram_sym_stack.push_back({get_word.word_string, get_word.val});

                break;
            }
            else if (CONCLUDE == _action_goto_map[{cur_state, word_string}].op)
            {

                int conclude_prod_id = _action_goto_map[{cur_state, word_string}].state;
                int prod_len;
                if (_prods[conclude_prod_id].right[0] == "$")
                    prod_len = 0;
                else
                    prod_len = _prods[conclude_prod_id].right.size();

                for (int i = 0; i < prod_len; ++i)
                {

                    _state_stack.erase(_state_stack.end() - 1);
                    _move_con_stack.erase(_move_con_stack.end() - 1);
                }

                _move_con_stack.push_back(_prods[conclude_prod_id].left);
                if (_action_goto_map.find({_state_stack[_state_stack.size() - 1], _prods[conclude_prod_id].left}) == _action_goto_map.end())
                {
                    std::cerr << "语法分析器算法发生致命错误CONCLUDE中" << std::endl;
                    std::cerr << get_word.val << std::endl;
                    return false;
                }
                _state_stack.push_back(_action_goto_map[{_state_stack[_state_stack.size() - 1], _prods[conclude_prod_id].left}].state);

                if (!_sementic.doSemanticCheck(_gram_sym_stack, _prods[conclude_prod_id]))
                    return false;
            }
            else if (ACCEPT == _action_goto_map[{cur_state, word_string}].op)
            {
                _printProcess(sytactic_step, _action_goto_map[{cur_state, word_string}]);
                sytactic_step++;
                std::cout << "语法分析正确完成！" << std::endl;
                std::cout << "语义分析正确完成！" << std::endl;
                return true;
            }
            else
            {
                std::cerr << "致命错误！" << std::endl;
                std::cerr << "语法分析器算法存在错误，请检查！" << std::endl;
                return false;
            }
            _printProcess(sytactic_step, _action_goto_map[{cur_state, word_string}]);
            sytactic_step++;
        }
    }

    return true;
}