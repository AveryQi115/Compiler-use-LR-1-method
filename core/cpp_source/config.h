#ifndef CONFIG_H
#define CONFIG_H

#include <string>

namespace path
{
    namespace token
    {
        const std::string TOKENIZER_PATH = "./out/token/tokenizer_result.txt";
    };

    namespace syntatic
    {
        const std::string PRODS_PATH = "./out/syntactic/prods.txt";
        const std::string FIRST_PATH = "./out/syntactic/fisrts.txt";
        const std::string FOLLOW_PATH = "./out/syntactic/follows.txt";
        const std::string GRAMSYM_PATH = "./out/syntactic/symbols.txt";
        const std::string LRITEM_PATH = "./out/syntactic/items.txt";
        const std::string NORMFAMILY_PATH = "./out/syntactic/normal_families.txt";
        const std::string TABLE_PATH = "./out/syntactic/action_goto_tables.csv";
        const std::string PROCESS_PATH = "./out/syntactic/syntactic_analyser_process.csv";
    };

    namespace semantic
    {
        const std::string IR_TMP_PATH = "./out/ir/IR_tmp.txt";
        const std::string IR_PATH = "./out/ir/IR.txt";
    };

    const std::string GRAM_PATH = "./in/grammar.txt";
}

#endif
