#ifndef SYNTACTIC_H
#define SYNTACTIC_H

#include <map>

#include "config.h"
#include "token.h"
#include "semantic.h"

enum SLR_OP_TYPE { CONCLUDE, MOVE, ACCEPT, NONTERMINAL, ERROR };
struct SLROp
{
    SLR_OP_TYPE op;
    int state;
    bool operator ==(const SLROp &operation) const;
};

struct LRItem {
    int prod_id;
    int dot_pos;
    LRItem(int item_num, int point_num) { this->prod_id = item_num, this->dot_pos = point_num; };
    bool operator ==(const LRItem &item) const;
    bool operator <(const LRItem &item) const;
};

struct LRforward {
    const LRItem* LRpointer;
    std::set<std::string> forward;
    //LRforward(const LRItem* LRpointer,std::set<std::string> forward) { this->LRpointer = LRpointer, this->forward = forward; };
    bool operator ==(const LRforward& item) const;
    bool operator <(const LRforward& item) const;
};

class Syntactic {
private:
    bool _print;
    std::vector<Gram> _grams;
    std::vector<Prod> _prods;
    std::set<std::string> _gram_sym;
    std::map<std::string, std::set<std::string>> _first_map, _follow_map;
    std::vector<std::set<LRforward>> _norm_families;
    std::set<LRItem> _lr_items;
    std::map<std::pair<int, std::string>, SLROp> _action_goto_map;
    std::ofstream _syntactic_w_hd;
    std::vector<std::string> _move_con_stack;
    std::vector<int> _state_stack;
    Semantic _sementic;
    std::vector<GramSym> _gram_sym_stack;

    bool _isNonTerminalSym(const std::string &sym);

    bool _genProd(const std::string path = path::GRAM_PATH);
    void _genAugGram();
    void _genFirstSet();
    void _genFollowSet();
    void _genGramSymSet();
    bool _genNormalFamilySet();
    void _genLRItems();
    bool _buildGram();
    std::set<LRforward> _genItemClosureSet(LRforward &);
    std::set<LRforward> _genItemsClosureSet(std::set<LRforward> &);
    std::set<std::string> _getProdFirstSet(const std::vector<std::string> &);

    void _print_buildGramDetails();
    void _printProds(const std::string path = path::syntatic::PRODS_PATH);
    void _printGrams();
    void _printFirst(const std::string path = path::syntatic::FIRST_PATH);
    void _printFollow(const std::string path = path::syntatic::FOLLOW_PATH);
    void _printGramSymSet(const std::string path = path::syntatic::GRAMSYM_PATH);
    void _printLRItems(const std::string path = path::syntatic::LRITEM_PATH);
    void _printClosure();
    void _printSLRError(const std::set<LRforward> &);
    void _printNormFamiliySet(const std::string path = path::syntatic::NORMFAMILY_PATH);
    void _printActionGoto(const std::string path = path::syntatic::TABLE_PATH);

    void _printProcess(int step, const SLROp &sl_op);

public:
    Syntactic(bool show_detail = true, const std::string path = path::syntatic::PROCESS_PATH);
    ~Syntactic();
    bool analyze(const std::string code_path, bool skip_parse = false);
};


#endif