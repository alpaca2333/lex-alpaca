//
// Created by alpaca on 16-10-23.
//

#include "nfa.h"
#include "regex.h"

using namespace std;

NFA* calculate(char opr, NFA* op1, NFA* op2 = NULL)
{
    switch (opr)
    {
        case '|':
            return op1->parallel(op2);
        case '*':
            return op1->repeat(REPEAT_0_N);
        case '+':
            return op1->repeat(REPEAT_1_N);
        case '?':
            return op1->repeat(REPEAT_0_1);
        case '&':
            return op1->concat(op2);
        case '(':
            throw BracketNotInPairException();
        default:
            return op1;
    }
}

void Regex::pushOperand(NFA* opn)
{
    stkOpn.push(opn);
}

void Regex::pushOperator(char opr)
{
    try {
        switch (opr)
        {
            case '(':
                _push(opr);
                break;
            case ')':
                while (stkOpr.top() != '(')
                {
                    _pop();
                    if (stkOpr.empty())
                    {
                        throw BracketNotInPairException();
                    }
                }
                popOperator();
                break;
            case '*':
            case '+':
            case '?':
            {
                NFA *result = calculate(opr, popOperand());
                pushOperand(result);
                break;
            }
            case '|':
                while (!stkOpr.empty() && stkOpr.top() == '&')
                {
                    _pop();
                }
                _push(opr);
                break;
            case '&':
                _push(opr);
                break;
            case '$':
                while (!stkOpr.empty())
                {
                    _pop();
                }
                if (stkOpn.size() > 1)
                {
                    throw ExcessiveOperandException();
                }
                for (NFA *p: nfaList)
                {
                    if (p != getNFA())
                        SAFE_RELEASE(p)
                }
                break;
            default:
                break;
        }
    }
    catch (LackOperandException& e)
    {
        throw e;
    }
    catch (LackOperatorException& e)
    {
        throw e;
    }
    catch (BracketNotInPairException& e)
    {
        throw e;
    }
}

NFA *Regex::popOperand()
{
    if (stkOpn.empty())
    {
        throw LackOperandException(stkOpr.top());
    }
    NFA* opn = stkOpn.top();
    stkOpn.pop();
    return opn;
}

char Regex::popOperator() {
    if (stkOpr.empty())
    {
        throw LackOperatorException();
    }
    char opr = stkOpr.top();
    stkOpr.pop();
    return opr;
}

void Regex::_push(char opr) {
    stkOpr.push(opr);
}

void Regex::pushChar(char c) {
    static char lastChar = 0;

    if (c == '+' || c == '*' || c == '?' || c == '|' || c == '$' || c == '(' || c == ')' || c == '&')
    {
        pushOperator(c);
    }
    else
    {
        if (lastChar != '|' && lastChar != '&' && lastChar != '(' && lastChar != 0)
        {
            pushOperator('&');
        }
        pushOperand(new NFA(c));
    }
    lastChar = c;
}

void Regex::_pop() {
    char opr = stkOpr.top();
    stkOpr.pop();
    NFA* op1 = popOperand();
    NFA* op2 = NULL;
    if (opr == '&' || opr == '|')
    {
        op2 = op1;
        op1 = popOperand();
    }
    pushOperand(calculate(opr, op1, op2));
//    SAFE_RELEASE(op2)
}

Regex::Regex(const char* re)
{
    loadRegex(re);
}

void Regex::loadRegex(const char* re)
{
    while (!stkOpr.empty()) stkOpr.pop();
    while (!stkOpn.empty()) stkOpn.pop();

    for (int i = 0; i < strlen(re); ++i)
    {
        pushChar(re[i]);
    }
}

std::string Regex::preCompile(const char* s)
{
    string a = s;
    unsigned long dotPos;
    while ((dotPos = a.find('.')) != string::npos)
    {

    }
    return std::__cxx11::string();
}

