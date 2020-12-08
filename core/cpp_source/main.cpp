#include <iostream>
#include <string>
#include "token.h"
#include "syntactic.h"

#include "utils.h"

int main(int argc, char** argv)
{
    bool skip_parse = false;
    if (argc > 1 && std::string(argv[1]) == "-l")
    {
        skip_parse = true;
    }

    std::string file_name = "./core/in/tmp.txt";
    Syntactic sa;
    sa.analyze(file_name, skip_parse = skip_parse);
    return 0;
}