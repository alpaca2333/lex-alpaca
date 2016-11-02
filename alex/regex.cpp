//
// Created by alpaca on 16-10-23.
//

#include "fa.h"
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
    if (!inBracket)
    {
        try
        {
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
        catch (LackOperandException &e)
        {
            throw e;
        }
        catch (LackOperatorException &e)
        {
            throw e;
        }
        catch (BracketNotInPairException &e)
        {
            throw e;
        }
    }
    else
    {
        switch (opr)
        {
            case '-':
                _push(opr);
                break;
            case '^':
                _push(opr);
                break;
            case '|':
                if (!stkEOpr.empty() && stkEOpr.top() == '-')
                {
                    _pop();
                }
                _push(opr);
                break;
            default:
                break;
        }
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

char Regex::popOperator()
{
    if (!inBracket)
    {
        if (stkOpr.empty())
        {
            throw LackOperatorException();
        }
        char opr = stkOpr.top();
        stkOpr.pop();
        return opr;
    }
    else
    {
        if (stkEOpr.empty())
        {
            throw LackOperatorException();
        }
        char opr = stkEOpr.top();
        stkEOpr.pop();
        return opr;
    }
}

void Regex::_push(char opr)
{
    if (!inBracket)
    {
        stkOpr.push(opr);
    }
    else
    {
        stkEOpr.push(opr);
    }
}

void Regex::judgeInsert(char lastChar, char c)
{
    if (!inBracket)
    {
        if (c != '+' && c != '*' && c != '?' && c != '|' && c != '$' && c != ']' && c != ')' && c != '&')
        {
            if (lastChar != '|' && lastChar != '&' && lastChar != '(' && lastChar != 0)
            {
                pushOperator('&');
            }
        }
    }
    else
    {
        if (c != ']' && c != '-' && c != '^')
        {
            if (lastChar != '-' && lastChar != '|' && lastChar != '^' && lastChar != '[' || stkEOpr.size() == 0 && stkEdge.size() == 1)
            {
                pushOperator('|');
            }
        }
    }
}

void Regex::pushChar(char c)
{
    judgeInsert(lastChar, c);
    if (!inBracket && c == '[')
    {
        while (!stkEdge.empty()) stkEdge.pop();
        while (!stkEOpr.empty()) stkEOpr.pop();
        inBracket = true;
        lastChar = c;
        return;
    }
    if (!inBracket)
    {

        if (c == '+' || c == '*' || c == '?' || c == '|' || c == '$' || c == '(' || c == ')' || c == '&')
        {
            pushOperator(c);
        }
        else
        {
            // handle special charactors
            if (c == '.')
            {
                pushOperand(new NFA("."));
            }
            else
            {
                pushOperand(new NFA(c));
            }
        }
    }
    else
    {
        if (c == ']')
        {
            while (!stkEOpr.empty())
            {
                _pop();
            }
            stkOpn.push(new NFA(stkEdge.top()));
            inBracket = false;
        }
        else if (c == '-')
        {
            if (stkEdge.empty())
            {
                pushEdge(new NFAEdge(NULL, '-'));
            }
            else
            {
                pushOperator(c);
            }
        }
        else if (c == '^')
        {
            pushOperator(c);
        }
        else
        {
            pushEdge(new NFAEdge(NULL, c));
        }
    }
    lastChar = c;
}

void Regex::_pop()
{
    if (!inBracket)
    {
        char opr = stkOpr.top();
        stkOpr.pop();
        NFA *op1 = popOperand();
        NFA *op2 = NULL;
        if (opr == '&' || opr == '|')
        {
            op2 = op1;
            op1 = popOperand();
        }
        pushOperand(calculate(opr, op1, op2));
        //    SAFE_RELEASE(op2)
    }
    else
    {
        char opr = popOperator();
        switch (opr)
        {
            case '-':
                if (stkEdge.empty())
                {
                    stkEdge.push(new NFAEdge(NULL, '-'));
                }
                else if (stkEdge.size() == 1)
                {
                    stkEdge.push(new NFAEdge(NULL, '-'));
                }
                else
                {
                    NFAEdge *e1 = popEdge(), *e2 = popEdge();
                    char tmp[4];
                    sprintf(tmp, "%c-%c", e2->allowedValues[0], e1->allowedValues[0]);
                    NFAEdge *e3 = new NFAEdge(NULL, tmp, NULL);
                    SAFE_RELEASE(e2)
                    SAFE_RELEASE(e1)
                    pushEdge(e3);
                }
                break;
            case '^':
                if (stkEdge.empty())
                {
                    pushEdge(new NFAEdge(NULL, '^'));
                }
                else
                {
                    NFAEdge* edge = popEdge();
                    edge->not_();
                    pushEdge(edge);
                }
                break;
            case '|':
                if (stkEdge.size() == 1)
                {
                    break;
                }
                else if (stkEdge.size() == 0)
                {
                    pushEdge(new NFAEdge(NULL, '|'));
                }
                else
                {
                    NFAEdge *e1 = popEdge(), *e2 = popEdge();
                    pushEdge(e1->merge(e2));
                    SAFE_RELEASE(e2);
                }
                break;
            default:
                break;
        }
    }
}

Regex::Regex(const char* re)
{
    loadRegex(re);
}

void Regex::loadRegex(const char* re)
{
    while (!stkOpr.empty()) stkOpr.pop();
    while (!stkOpn.empty()) stkOpn.pop();

    int i;
    for (i = 0; i < strlen(re); ++i)
    {
        pushChar(re[i]);
        if (re[i] == '$') break;
    }
    if (re[i] != '$')
    {
        // cleaning code should be here

        lastChar = 0;
        pushChar('$');
    }
}

void Regex::pushEdge(NFAEdge* edge)
{
    this->stkEdge.push(edge);
}

NFAEdge *Regex::popEdge()
{
    if (stkEdge.empty())
    {
        throw LackOperandException(' ');
    }
    NFAEdge* result = stkEdge.top();
    stkEdge.pop();
    return result;
}
