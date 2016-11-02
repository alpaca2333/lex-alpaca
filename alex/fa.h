#pragma once


#include "stdafx.h"

#define interface class

interface IEdge;
interface IFA;
class NFAEdge;
class NFANode;
class NFA;
class DFAEdge;
class DFANode;
class DFA;
class NoSolidEdgeOutException;
class EndValueType;
class EdgeMap;

typedef char TransValue;
typedef EndValueType EndType;

#define REPEAT_0_1 0
#define REPEAT_1_N 1
#define REPEAT_0_N 2


#define MIN_TRANSVALUE 32
#define MAX_TRANSVALUE 126

/* thrown when no nodes can be reached under specified input */
class NoSolidEdgeOutException : public std::exception
{
public:
    NoSolidEdgeOutException(int invalidValue) { sprintf(message, "Cannot make a state transfer under character '\\%o(%c)'.", invalidValue, invalidValue); }

    const char* what() { return message; }

private:
    char message[2048];
};

class NotAcceptedStateException : public std::exception { };

class EndValueType
{
public:
    EndValueType() { }

    EndValueType(const EndValueType&);

    EndValueType& operator=(const EndValueType &);

    bool operator<(EndValueType&);

    bool operator<=(EndValueType&);

    bool operator==(EndValueType&);

    bool operator>=(EndValueType&);

    bool operator>(EndValueType&);

    int value = -1;

    /* a bigger number means a higher priority */
    int priority;

    std::string name;
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
    NFANode(NFA* context, int);

    virtual ~NFANode();

    const int nid = ++maxnid;

    /* edges that goes out of the node */
    std::vector<NFAEdge *> edges;

    std::vector<NFAEdge *> getEdges(TransValue transVal);

    /* get the set of nodes that can reach under the given character */
    std::vector<NFANode *> getPostNodes(TransValue transVal);

    /* set the node's end type */
    void setEndType(int type, int priority, std::string name);

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

interface IEdge
{
public:
    virtual bool check(TransValue) = 0;
};

class NFAEdge : public IEdge
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

    NFA* context;

    std::vector<TransValue> allowedValues;
};

class DFAEdge : IEdge
{
public:

    DFAEdge(DFA* context, std::vector<TransValue> allowedValues) : allowedValues(allowedValues), context(context) { init(); }

    DFAEdge(DFA* context, DFANode* destination) : context(context), destination(destination) { init(); }

    void init();

    void addValue(TransValue);

    void removeValue(TransValue);

    bool check(TransValue value);

    DFANode* destination;

    std::vector<TransValue> allowedValues;
private:

    DFA* context;
};


interface IFA
{
public:
    /* make a transfer under the given value */
    virtual void transfer(int transVal) = 0;

    /* returns a boolean indicating whether the sequence is
     * accepted by the FA */
    virtual bool matches(const char* seq) = 0;

    virtual std::string regex() = 0;

    virtual void setOnTokenAccepted(std::function<void(int type, const char*, const char* token)> cb) = 0;

    virtual void setOnCharacterUnaccepted(std::function<void(char c, int position)> cb) = 0;

    virtual ~IFA() { };
};


class DFANode
{
public:
    const int nid = ++maxnid;

    DFANode(DFA* context) : context(context) { init(); }

    void init();

    DFAEdge* getEdge(TransValue value);

    DFANode* getPostNode(TransValue value);

    void link(DFAEdge* edge, DFANode* destination);

    void link(TransValue value, DFANode* destination);

    std::unordered_map<int, int> stateSet;
protected:

    EdgeMap* edges;

    static int maxnid;

    DFA* context;
};


class DFA : public IFA
{
public:
    DFA(DFANode* start) { }

    void addResource(DFANode* res);

    void addResource(DFAEdge* res);

    virtual ~DFA();

    DFANode* start = NULL;

    DFANode* getNode(std::vector<NFANode *>);
private:

    std::vector<DFANode *> nodeList;

    std::vector<DFAEdge *> edgeList;
};




class NFA : public IFA
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

    void setEndType(int endValue, int priority, std::string name);

    void addResource(NFANode *node);

    void addResource(NFAEdge *edge);

    void printCurrState();

    void giveUpResource();

    void read(const char* seq);

    std::string regex() { return strRegex; }

    /* get the preferred endtype */
    EndType getPreferredEndType();

    /* get the list of all possible end type */
    std::vector<EndType> getEndType();

    void setOnTokenAccepted(std::function<void(int type, const char*, const char* token)> cb);

    void setOnCharacterUnaccepted(std::function<void(char c, int position)> cb);

    std::function<void(int type, const char* description, const char* token)> onTokenAccepted = [](int, const char*, const char*) { };

    std::function<void(char c, int position)> onCharacterUnaccepted = [](char, int) { };

    DFA* getDFA();
protected:
    void pushState();

    void popState();

    std::stack<std::vector<NFANode *>> stateStack;

    NFANode* Start = NULL;

    NFANode* End = NULL;

    std::vector<NFANode *> getCurrStatus();

    std::vector<NFANode *> currStatus;

    std::vector<NFANode *> nodeList;

    std::vector<NFAEdge *> edgeList;

    std::string strRegex = "";
};


class EdgeMap : protected std::unordered_map<TransValue, DFAEdge *>
{
public:
    EdgeMap(DFA* context) : context(context) { }

    void putEdge(TransValue value, DFANode* destination);

    DFAEdge* getEdge(TransValue value) { return (*this)[value]; }
private:
    DFA* context;
};

bool nodeSetEquals(std::unordered_map<int, int>, std::unordered_map<int, int>);