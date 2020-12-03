#include "token.h"

const char *TypeTranslation[] = {"NUM", "KEYWARD", "IDENTIFIER", "TYPE", "BOARDER", "UNKNOWN", "EOF", "OPERATOR"};

bool Tokenizer::isReady(const std::string code_path, bool print)
{
    if (print)
    {
        this->_ifprint = true;
        _tokenizer_w_hd.open(path::token::TOKENIZER_PATH);
        if (!_tokenizer_w_hd.is_open())
        {
            std::cerr << "词法分析过程中，显示输出结果文件打开失败！" << std::endl;
        }
    }
    else
    {
        this->_ifprint = false;
    }

    _code_r_hd.open(code_path);

    if (!_code_r_hd.is_open())
    {
        std::cerr << "词法分析过程中，源代码文件打开失败！" << std::endl;
        return false;
    }
    return true;
}

bool Tokenizer::_isLetter(const unsigned char ch)
{
    if (ch >= 'a' && ch <= 'z' || ch >= 'A' && ch <= 'Z' || '_' == ch)
        return true;
    else
        return false;
}

bool Tokenizer::_isDig(const unsigned char ch)
{
    if (ch >= '0' && ch <= '9')
        return true;
    else
        return false;
}

bool Tokenizer::_isSingleCharOp(const unsigned char ch)
{
    if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
        return true;
    else
        return false;
}

bool Tokenizer::_isDoubleCharOpPre(const unsigned char ch)
{
    if (ch == '=' || ch == '<' || ch == '>' || ch == '!')
        return true;
    else
        return false;
}

bool Tokenizer::_isBlank(const unsigned char ch)
{
    if (' ' == ch || '\n' == ch || '\t' == ch || 255 == ch)
        return true;
    else
        return false;
}

unsigned char Tokenizer::_getNextChar()
{
    unsigned char ch = _code_r_hd.get();
    if ('\n' == ch)
        _line_cnt++;
    return ch;
}

Word Tokenizer::_getBasicWord()
{
    unsigned char ch;
    std::string str_token;
    Word word;

    while (!_code_r_hd.eof())
    {
        ch = _getNextChar();
        if ('/' == ch)
        {
            unsigned char next = _getNextChar();
            if ('/' == next)
            {
                while (_getNextChar() != '\n' && !_code_r_hd.eof())
                    ;
            }
            else if ('*' == next)
            {
                while (!_code_r_hd.eof())
                {
                    while (_getNextChar() != '*' && !_code_r_hd.eof())
                        ;

                    if (_code_r_hd.eof())
                    {
                        word.type = LUNKNOWN;
                        word.val = "commitment error! line: " + std::to_string(_line_cnt);
                        return word;
                    }

                    if (_getNextChar() == '/')
                        break;
                }
            }
            else
            {
                _code_r_hd.seekg(-1, std::ios::cur);
                word.type = LOPERATOR;
                word.val = ch;
                return word;
            }
        }
        else if (_isDig(ch))
        {
            str_token += ch;
            while (!_code_r_hd.eof())
            {
                unsigned char next = _getNextChar();
                if (!_isDig(next))
                {
                    _code_r_hd.seekg(-1, std::ios::cur);
                    word.type = LCINT;
                    word.val = str_token;
                    return word;
                }
                else
                    str_token += next;
            }
        }
        else if (_isLetter(ch))
        {
            str_token += ch;
            while (!_code_r_hd.eof())
            {
                unsigned char next = _getNextChar();
                if ((!_isDig(next)) && (!_isLetter(next)))
                {
                    _code_r_hd.seekg(-1, std::ios::cur);

                    if (_KEYWORDS.find(str_token) != _KEYWORDS.end())
                        word.type = LKEYWORD;
                    else if (_TYPES.find(str_token) != _TYPES.end())
                        word.type = LTYPE;
                    else
                        word.type = LIDENTIFIER;

                    word.val = str_token;
                    return word;
                }
                else
                    str_token += next;
            }
        }
        else if (_BORDERS.find(ch) != _BORDERS.end())
        {
            word.type = LBORDER;
            word.val = ch;
            return word;
        }
        else if (_isSingleCharOp(ch))
        {
            word.type = LOPERATOR;
            word.val = ch;
            return word;
        }
        else if (_isDoubleCharOpPre(ch))
        {
            str_token += ch;
            unsigned char next = _getNextChar();

            if ('=' == next)
            {
                str_token += next;
                word.type = LOPERATOR;
                word.val = str_token;
                return word;
            }
            else
            {
                _code_r_hd.seekg(-1, std::ios::cur);
                if (ch != '!')
                {
                    word.type = LOPERATOR;
                    word.val = str_token;
                    return word;
                }
                else
                {
                    word.type = LUNKNOWN;
                    word.val = str_token + ", line: " + std::to_string(_line_cnt);
                    return word;
                }
            }
        }
        else if (_isBlank(ch))
        {
            continue;
        }
        else
        {
            word.type = LUNKNOWN;
            word.val = ch;
            word.val += ", line: " + std::to_string(_line_cnt);
            return word;
        }
    }

    word.type = LEOF;
    word.val = "#";
    return word;
}

Word Tokenizer::getWord()
{
    Word get_word = _getBasicWord();
    std::string word_string;
    if (get_word.type == LEOF)
        word_string = "#";
    else if (get_word.type == LCINT)
        word_string = "num";
    else if (get_word.type == LKEYWORD || get_word.type == LTYPE ||
             get_word.type == LBORDER || get_word.type == LOPERATOR)

        word_string = get_word.val;
    else if (get_word.type == LTYPE)
        word_string = get_word.val;
    else if (get_word.type == LIDENTIFIER)
        word_string = "identifier";
    else
    {
        ;
    }

    get_word.word_string = word_string;

    if (_ifprint)
        _print(get_word);
    return get_word;
}

void Tokenizer::_print(Word word)
{
    if (word.type == LEOF)
        return;
    _tokenizer_w_hd << "step " << step_cnt << " , type: " << TypeTranslation[word.type] << " , value: " << word.val;
    if (word.type == LUNKNOWN)
        _tokenizer_w_hd << "    ****** warning! ******";
    _tokenizer_w_hd << std::endl;
    step_cnt++;
    return;
}

Tokenizer::Tokenizer()
{
    _line_cnt = 1;
    step_cnt = 1;
    _ifprint = false;
}

Tokenizer::~Tokenizer()
{
    if (_code_r_hd.is_open())
        _code_r_hd.close();

    if (_tokenizer_w_hd.is_open())
        _tokenizer_w_hd.close();
}
