#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>

enum CAT_TYPE
{
    FUNCTION,
    VARIABLE,
    CONST,
    TEMP,
    RETURN_VAR
};
enum VAR_TYPE
{
    INT,
    VOID
};

enum TOKEN_TYPE
{
    LCINT,
    LKEYWORD,
    LIDENTIFIER,
    LTYPE,
    LBORDER,
    LUNKNOWN,
    LEOF,
    LOPERATOR,
    LBEGIN,
    LCFLOAT,
    LCDOUBLE,
    LCCHAR,
    LCSTRING,
    LCERROR,
    LOERROR
};

struct Word
{
    TOKEN_TYPE type;
    std::string val;
    std::string word_string;

    int func_idx;
};

struct SymPos
{
    int table_pos;
    int sym_pos;
};
struct GramSym
{
    std::string sym_name;
    std::string txt_val;
    SymPos pos;
    std::string op;
};

struct Gram
{
    std::string left;
    std::vector<std::string> right;
};

struct Prod
{
    std::string left;
    std::vector<std::string> right;
};

struct Tuple4
{
    int num;
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string result;

    void set(int num, const std::string &op, const std::string &arg1, const std::string &arg2, const std::string &result);
};

struct Inst
{
    std::string op;
    std::string arg1;
    std::string arg2;
    std::string arg3;
    Inst();
    Inst(const std::string &op, const std::string &arg1, const std::string &arg2, const std::string &arg3);
    void set(const std::string &op, const std::string &arg1, const std::string &arg2, const std::string &arg3);
};

struct Register
{
    const std::string name;
    std::string content;
    SymPos content_info;
    int miss_time = 0;
    bool used = false;
};

#endif