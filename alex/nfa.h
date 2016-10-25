#pragma once


#include "stdafx.h"

class NFAEdge;
class NFANode;
class FA;
class NFA;
class DFA;
class NoSolidEdgeOutException;
class EndValueType;

#define REPEAT_0_1 0
#define REPEAT_1_N 1
#define REPEAT_0_N 2
#define SAFE_RELEASE(p) { if (p) delete p; p = 0; }

typedef NFANode* PFANode;
typedef NFAEdge* PFAEdge;
typedef int TransValue;
typedef EndValueType EndType;

#define MIN_TRANSVALUE 32
#define MAX_TRANSVALUE 127

/* thrown when no nodes can be reached under specified input */
class NoSolidEdgeOutException : public std::exception
{
public:
    NoSolidEdgeOutException(int invalidValue) { sprintf(message, "Cannot make a state transfer under character '\\%o(%c)'.", invalidValue, invalidValue); }

    const char* what() { return message; }

private:
    char message[2048];
};

class EndValueType
{
public:
    EndValueType() { }

    EndValueType(const EndValueType&);

    EndValueType& operator=(EndValueType&);

    bool operator<(EndValueType&);

    bool operator<=(EndValueType&);

    bool operator==(EndValueType&);

    bool operator>=(EndValueType&);

    bool operator>(EndValueType&);

    int value = -1;
    int priority;
    static int maxPriority;
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

    /* create an NFANode with a single allowed value */
    NFANode(NFA* context, TransValue);

    virtual ~NFANode();

    const int nid = ++maxnid;

    /* edges that goes out of the node */
    std::vector<NFAEdge *> edges;

    std::vector<NFAEdge *> getEdges(TransValue transVal);

    /* get the set of nodes that can reach under the given character */
    std::vector<NFANode *> getPostNodes(TransValue transVal);

    /* set the node's end type */
    void setEndType(int type);

    /* zero if it is a non-terminal node, else
     * a positive integer representing a specified
     * terminal status. */
    EndType endType;

    std::string note = "";

    NFA* context;

    /* link a node with a given value */
    void link(TransValue, NFANode*);

    /* link a node with a given value set. See constructor of NFAEdge */
    void link(const char* seq, NFANode*);

protected:
    static int maxnid;
};

class NFAEdge
{
public:

    /* create an edge with context. The context is the one responsible for
     * releasing the edge */
    NFAEdge(NFA* context);

    /* create the edge with the given value */
    NFAEdge(NFA* context, TransValue transValue);

    /* create the edge with the given value and link it to the destination */
    NFAEdge(NFA* context, TransValue transValue, NFANode* destination);

    /*
     * Allows the follow format of sequence:
     * - 'a-z'
     * - '^abcdefghi'
     * - 'abcdefghi'
     */
    NFAEdge(NFA* context, const char* seq, NFANode* destination);

    /* merge another edge to this one, which means merging the
     * two edges' allowing values */
    NFAEdge* merge(NFAEdge* another);

    /* compute the complementary set of the allowing values */
    NFAEdge *not_();

    virtual ~NFAEdge();

    /* check if the given value is allowed by this edge */
    bool check(TransValue);

    /* destination of the edges */
    NFANode* destination = NULL;

    FA* context;

    std::vector<TransValue> allowedValues;
private:
    std::function<bool(TransValue)> _check;
};

class FA
{
public:
    std::string regex = "";

    /* make a transfer under the given value */
    virtual void transfer(int transVal) = 0;

    /* returns a boolean indicating whether the sequence is
     * accepted by the FA */
    virtual bool matches(const char* seq) = 0;

    virtual ~FA() { };
};



class NFA : public FA
{
public:
    NFA();

    NFA(TransValue value);

    NFA(const char* seq);

    /* create the NFA with a single edge */
    NFA(NFAEdge* edge);

    ~NFA();

    /* use '|' to connect two NFAs in parallel */
    NFA* parallel(NFA *, int = -1);

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

    void setEndType(int endValue, int priority);

    void addNode(NFANode *node);

    void addEdge(NFAEdge *edge);

    void printCurrState();

    void giveUpResource();

    /* get the preferred endtype */
    EndType getPreferredEndType();

    /* get the list of all possible end type */
    std::vector<EndType> getEndType();
protected:

    NFANode* Start = NULL;

    NFANode* End = NULL;

    std::vector<NFANode *> getCurrStatus();

    std::vector<NFANode *> currStatus;

    std::vector<NFANode *> nodeList;

    std::vector<NFAEdge *> edgeList;
};



