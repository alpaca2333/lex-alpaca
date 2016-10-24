#pragma once

#include "stdafx.h"

class NFAEdge;
class NFANode;
class FA;
class NFA;
class DFA;
class NoSolidEdgeOutException;

#define REPEAT_0_1 0
#define REPEAT_1_N 1
#define REPEAT_0_N 2
#define SAFE_RELEASE(p) { if (p) delete p; p = 0; }

typedef NFANode* PFANode;
typedef NFAEdge* PFAEdge;
typedef int TransValue;
typedef int EndType;

/* thrown when no nodes can be reached under specified input */
class NoSolidEdgeOutException : public std::exception
{
public:
    NoSolidEdgeOutException(int invalidValue) { sprintf(message, "Cannot make a state transfer under character '\\%o(%c)'.", invalidValue, invalidValue); }

    const char* what() { return message; }

private:
    char message[2048];
};



class InvalidNodeSeqException : public std::exception
{
public:
    InvalidNodeSeqException(const char* invalidSeq) { strcpy(invalid, invalidSeq); }

    const char* what() { return invalid; }
private:
    char invalid[12];
};




class NFANode
{
public:

    NFANode(NFA* context);

    NFANode(NFA* context, EndType);

    virtual ~NFANode();

    const int nid = ++maxnid;

    /* edges that goes out of the node */
    std::vector<NFAEdge *> edges;

    std::vector<NFAEdge *> getEdges(TransValue transVal);

    std::vector<NFANode *> getPostNodes(TransValue transVal);

    /* zero if it is a non-terminal node, else
     * a positive integer representing a specified
     * terminal status.
     */
    EndType endType = -1;

    std::string note = "";

    NFA* context;

    void link(TransValue, NFANode*);

protected:
    static int maxnid;
};

class NFAEdge
{
public:

    NFAEdge(NFA* context);

    NFAEdge(NFA* context, TransValue transValue);

    NFAEdge(NFA* context, TransValue transValue, NFANode* destination);

    NFAEdge(NFA* context, const char* seq, NFANode* destination);

    virtual ~NFAEdge();

    bool check(TransValue);

    /* destination of the edges */
    NFANode* destination = NULL;

    FA* context;

private:
    std::function<bool(TransValue)> _check;

    bool _dot(TransValue);

    bool _range(TransValue);

    bool _not(TransValue);

    bool _single(TransValue);

    TransValue range[2];

    TransValue notList[128];

    /*
     * the value of the transporting character,
     * zero if it is epsilon.
     */
    TransValue value;
};

class FA
{
public:
    std::string regex = "";

    /* make a transfer under the given value */
    virtual void transfer(int transVal) = 0;

    virtual bool matches(const char* seq) = 0;

    virtual ~FA() { };
};



class NFA : public FA
{
public:
    NFA();

    NFA(TransValue value);

    ~NFA();

    /* use '|' to connect two NFAs in parallel */
    NFA* parallel(NFA *, EndType = -1);

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

    NFANode* start() { return Start; }

    NFANode* end() { return End; }

    EndType endValue() { return End->endType; }

    void transfer(int transVal) throw(NoSolidEdgeOutException);

    bool matches(const char* seq);

    void setEndType(EndType endValue);

    void addNode(NFANode *node);

    void addEdge(NFAEdge *edge);

    void printCurrState();

    void giveUpResource();
protected:
    static int lastOprLevel;

    NFANode* Start = NULL;

    NFANode* End = NULL;

    std::vector<NFANode *> getCurrStatus();

    std::vector<EndType> getEndValues();

    std::vector<NFANode *> currStatus;

    std::vector<NFANode *> nodeList;

    std::vector<NFAEdge *> edgeList;
};



