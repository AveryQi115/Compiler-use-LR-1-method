#ifndef LEXICAL_H
#define LEXICAL_H

#include <iostream>
#include <fstream>
#include <set>
#include <string>

#include "utils.h"
#include "config.h"

class Tokenizer
{
private:
    std::set<std::string> _OPERATORS = {"+", "-", "*", "/", "=", "==", ">", ">=", "<", "<=", "!="};

    std::set<std::string> _KEYWORDS = {"if", "else", "return", "while"};

    std::set<std::string> _TYPES = {"int", "void"};

    std::set<std::string> _PRE_OPERATORS = {"+", "-"};

    std::set<char> _BORDERS = {'(', ')', '{', '}', ',', ';'};

    std::ifstream _code_r_hd;
    std::ofstream _tokenizer_w_hd;

    unsigned int _line_cnt;
    bool _ifprint;
    unsigned int step_cnt;

    bool _isLetter(const unsigned char ch);
    bool _isDig(const unsigned char ch);
    bool _isSingleCharOp(const unsigned char ch);
    bool _isDoubleCharOpPre(const unsigned char ch);
    bool _isBlank(const unsigned char ch);
    unsigned char _getNextChar();
    void _print(Word word);
    Word _getBasicWord();

public:
    Tokenizer();
    ~Tokenizer();
    bool isReady(const std::string code_path, bool print = true);
    Word getWord();
};

#endif