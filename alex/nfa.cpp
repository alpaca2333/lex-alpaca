/*
 * Created by alpaca on 16-10-21.
 */

#include "nfa.h"


bool contains(std::vector<NFANode *> &v, NFANode* value)
{
    for (NFANode* node: v)
    {
        if (node == value)
        {
            return true;
        }
    }
    return false;
}


NFANode::NFANode(NFA* context) : context(context) {
    context->addNode(this);
}

NFANode::NFANode(NFA* context, EndType endValue) : context(context)
{
    this->endType = endValue;
    context->addNode(this);
}

NFANode::~NFANode() { }

int NFANode::maxnid = 0;
int NFA::lastOprLevel = 0;

std::vector<NFAEdge *> NFANode::getEdges(TransValue transVal)
{
    std::vector<NFAEdge *> result;
    for (int i = 0; i < edges.size(); ++i)
    {
        if (this->edges[i] && this->edges[i]->value == transVal)
        {
            result.push_back(this->edges[i]);
        }
    }
    return result;
}

std::vector<NFANode *> NFANode::getPostNodes(TransValue transVal)
{
    std::vector<NFANode *> result;
    for (NFAEdge* pEdge: getEdges(transVal))
    {
        result.push_back(pEdge->destination);
        for (NFAEdge* ppEdge: pEdge->destination->getEdges(0))
        {
            result.push_back(ppEdge->destination);
        }
    }
    return result;
}

void NFANode::link(TransValue transValue, NFANode* node) {
    this->edges.push_back(new NFAEdge(this->context, transValue, node));
}

NFAEdge::NFAEdge(NFA* context) : context(context){
    context->addEdge(this);
}

NFAEdge::~NFAEdge()
{
    // TODO
}

NFAEdge::NFAEdge(NFA* context, TransValue transValue) : context(context) {
    this->value = transValue;
    context->addEdge(this);
}

NFAEdge::NFAEdge(NFA* context, TransValue transValue, NFANode *destination) : NFAEdge(context, transValue) {
    this->context = context;
    this->destination = destination;
}

std::vector<NFANode *> NFA::getCurrStatus()
{
    return this->currStatus;
}

void NFA::transfer(int transVal) throw(NoSolidEdgeOutException)
{
    std::vector<NFANode *> tmp;
    for (NFANode* pNode: this->currStatus)
    {
        tmp.push_back(pNode);
    }
    this->currStatus.clear();
    for (NFANode *pNode: tmp)
    {
        for (NFANode *tmpNode: pNode->getPostNodes(transVal))
        {
            this->currStatus.push_back(tmpNode);
        }
    }

    if (currStatus.size() == 0)
    {
        throw NoSolidEdgeOutException(transVal);
    }
    computeClosure();
}

void NFA::printCurrState()
{
    for (NFANode * node: currStatus)
    {
        printf("state: %d\ndescription: %s\n\n", node->nid, node->note.data());
    }
}

std::vector<EndType> NFA::getEndValues()
{
    std::vector<EndType> result;
    for (NFANode * node: currStatus)
    {
        if (node->endType >= 0)
        {
            result.push_back(node->endType);
        }
    }
    return result;
}

void NFA::setEndValue(EndType endValue)
{
    End->endType = endValue;
}

NFA::~NFA() {
    for (NFANode *node: nodeList)
    {
        SAFE_RELEASE(node);
    }
    for (NFAEdge *edge: edgeList)
    {
        SAFE_RELEASE(edge);
    }
}

void NFA::addNode(NFANode *node) {
    nodeList.push_back(node);
}

void NFA::addEdge(NFAEdge *edge) {
    edgeList.push_back(edge);
}

void NFA::computeClosure() {
    // compute a closure
    unsigned long lastSize;
    do
    {
        for (int i = 0; i < currStatus.size(); ++i)
        {
            NFANode* pNode = currStatus[i];
            for (NFANode *tmpNode: pNode->getPostNodes(0))
            {
                if (!contains(currStatus, tmpNode))
                    currStatus.push_back(tmpNode);
            }
        }
        lastSize = currStatus.size();
    } while (lastSize != currStatus.size());
}

void NFA::resetState() {
    this->currStatus.clear();
    this->currStatus.push_back(Start);
    computeClosure();
}

NFA* NFA::parallel(NFA *another, EndType endValue)
{
    this->regex = this->regex + "|" + another->regex;

    NFANode* start = new NFANode(this);
    start->link(0, this->Start);
    start->link(0, another->Start);
    NFANode* end = new NFANode(this, endValue);
    this->End->link(0, end);
    another->End->link(0, end);

    this->Start = start;
    this->End = end;

    resetState();
    return this;
}

NFA* NFA::repeat(int repeatMode)
{
    if (repeatMode == REPEAT_1_N)
    {
        regex = "(" + regex + ")+";
    }
    else if (repeatMode == REPEAT_0_N)
    {
        regex = "(" + regex + ")*";
    }
    else if (repeatMode == REPEAT_0_1)
    {
        regex = "(" + regex + ")?";
    }

    NFANode *start = new NFANode(this);
    NFANode *end = new NFANode(this);
    start->link(0, this->Start);
    this->End->link(0, end);
    if (repeatMode == REPEAT_0_N || repeatMode == REPEAT_0_1)
    {
        start->link(0, end);
    }
    if (repeatMode == REPEAT_1_N || repeatMode == REPEAT_0_N)
    {
        this->End->link(0, this->Start);
    }

    this->Start = start;
    this->End = end;

    resetState();
    return this;
}

NFA* NFA::concat(NFA *another) {
    this->regex = "(" + this->regex + ")" + another->regex;
    this->End->link(0, another->Start);
    this->End = another->End;

    resetState();
    return this;
}

bool NFA::matches(const char *seq) {
    resetState();
    bool result = true;
    int i;
    try
    {
        for (i = 0; i < strlen(seq); ++i)
        {
            transfer(seq[i]);
        }
    }
    catch (NoSolidEdgeOutException& e)
    {
        std::cerr << e.what() << "\n";
        std::cerr << seq << "\n";
        for (int j = 0; j < i; ++j) {
            std::cerr << ' ';
        }
        std::cerr << '^' << "\n";
        return false;
    }

    for (NFANode* node: currStatus)
    {
        if (node->endType > -1)
        {
            return true;
        }
    }
    return false;
}

NFA::NFA()
{
    End = Start = new NFANode(this, 0);
    this->currStatus.push_back(Start);
}

NFA::NFA(TransValue transValue)
{
    this->regex = (char) transValue;
    Start = new NFANode(this);
    End = new NFANode(this);
    NFAEdge* edge = new NFAEdge(this, transValue);
    Start->edges.push_back(edge);
    edge->destination = End;
    this->currStatus.clear();
    this->currStatus.push_back(Start);
}
