//
// Created by alpaca on 16-10-23.
//

#pragma once

#include "stdafx.h"
#include "nfa.h"

class LackOperandException : public std::exception
{
public:
    LackOperandException(char opr) {
        sprintf(message, "operand '%c' lack operand", opr);
    }

    const char* what()
    {
        return message;
    }

private:
    char message[1024];
};
class ExcessiveOperandException : public std::exception { };
class LackOperatorException : public std::exception { };
class BracketNotInPairException : public std::exception { };

class Regex
{
public:
    Regex(const char*);

    void pushOperand(NFA*);

    void loadRegex(const char*);

    /*
     * push and calculate
     */
    void pushOperator(char);

    /* push a char, automatically decide what the char means */
    void pushChar(char);

    NFA* getNFA() { return stkOpn.top(); }
protected:

    NFA* popOperand();

    char popOperator();

    /* merely push an operator into the stack */
    void _push(char opr);

    /* pop an opr and do the calculation */
    void _pop();

    std::vector<NFA *> nfaList;
    std::stack<NFA *> stkOpn;
    std::stack<char> stkOpr;
};