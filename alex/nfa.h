#pragma once

#include "stdafx.h"
#include <iostream>

class FAEdge;
class FANode;
class FA;
class NFA;
class DFA;
class NoSolidEdgeOutException;

#define REPEAT_0_1 0
#define REPEAT_1_N 1
#define REPEAT_0_N 2

#define SAFE_RELEASE(p) {delete p; p = 0;}

typedef FANode* PFANode;
typedef FAEdge* PFAEdge;
typedef int TransValue;
typedef int EndValue;

/* thrown when no nodes can be reached under specified input */
class NoSolidEdgeOutException : public std::exception
{
public:
    NoSolidEdgeOutException(int invalidValue) { sprintf(message, "Cannot make a state transfer under character '\\%d(%c)'.", invalidValue, invalidValue); }

    const char* what() { return message; }

private:
    char message[1024];
};

class FANode
{
public:

    FANode(FA* context);

    FANode(FA* context, EndValue);

    virtual ~FANode();

    const int nid = ++maxnid;

    /* edges that goes out of the node */
    std::vector<FAEdge *> edges;

    std::vector<FAEdge *> getEdges(TransValue transVal);

    std::vector<FANode *> getPostNodes(TransValue transVal);

    /* zero if it is a non-terminal node, else
     * a positive integer representing a specified
     * terminal status.
     */
    EndValue endType = -1;

    std::string note = "";

    void link(TransValue, FANode*);

    FA* context;
protected:
    static int maxnid;
};

class FAEdge
{
public:

    FAEdge(FA* context);

    FAEdge(FA* context, TransValue transValue);

    FAEdge(FA* context, TransValue transValue, FANode *destination);

    virtual ~FAEdge();

    /*
     * the value of the transporting character,
     * zero if it is epsilon.
     *
     * considering supporting unicode :P
     */
    TransValue value;

    /* destination of the edges */
    FANode* destination = NULL;

    FA* context;
};

class FA
{
public:
    FA();

    FA(TransValue transValue);

    std::string regex = "";

    void printCurrState();

    FANode* start() { return Start; }

    FANode* end() { return End; }

    EndValue endValue() { return End->endType; }

    void setEndValue(EndValue endValue);

    void addNode(FANode *node);

    void addEdge(FAEdge *edge);

    /* make a transfer under the given value */
    virtual void transfer(int transVal) = 0;

    virtual bool matches(const char* seq) = 0;

    virtual ~FA();
protected:
    FANode* Start = NULL;

    FANode* End = NULL;

    std::vector<FANode *> getCurrStatus();

    std::vector<EndValue> getEndValues();

    std::vector<FANode *> currStatus;

    std::vector<FANode *> nodeList;

    std::vector<FAEdge *> edgeList;
};

class NFA : public FA
{
public:
    NFA() : FA() { }
    NFA(TransValue value) : FA(value) { }

    /* use '|' to connect two NFAs in parallel */
    NFA* parallel(NFA *, EndValue = -1);

    /* use '?*+' to repeat an NFA */
    NFA* repeat(int repeatMode);

    /* directly concat two NFAs */
    NFA* concat(NFA *);

    /* compute the closure of current state */
    void computeClosure();

    /* reset the current state to the closure of Start node.
     * This will be called when the fa structure is changed
     */
    void resetState();

    void transfer(int transVal) throw(NoSolidEdgeOutException);

    bool matches(const char* seq);

protected:
    static int lastOprLevel;
};

