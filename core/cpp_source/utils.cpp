#include "utils.h"

void Tuple4::set(int num, const std::string &op, const std::string &arg1, const std::string &arg2, const std::string &result)
{
    this->num = num;
    this->op = op;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->result = result;
    return;
}

Inst::Inst() {}

Inst::Inst(const std::string &op, const std::string &arg1, const std::string &arg2, const std::string &arg3)
{
    this->op = op;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->arg3 = arg3;
}
void Inst::set(const std::string &op, const std::string &arg1, const std::string &arg2, const std::string &arg3)
{
    this->op = op;
    this->arg1 = arg1;
    this->arg2 = arg2;
    this->arg3 = arg3;
}
