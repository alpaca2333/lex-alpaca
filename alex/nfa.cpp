/*
 * Created by alpaca on 16-10-21.
 */

#include "nfa.h"


bool contains(std::vector<FANode *> &v, FANode* value)
{
    for (FANode* node: v)
    {
        if (node == value)
        {
            return true;
        }
    }
    return false;
}


FANode::FANode(FA* context) : context(context) {
    context->addNode(this);
}

FANode::~FANode() { }

int FANode::maxnid = 0;
int NFA::lastOprLevel = 0;

std::vector<FAEdge *> FANode::getEdges(TransValue transVal)
{
    std::vector<FAEdge *> result;
    for (int i = 0; i < edges.size(); ++i)
    {
        if (this->edges[i] && this->edges[i]->value == transVal)
        {
            result.push_back(this->edges[i]);
        }
    }
    return result;
}

std::vector<FANode *> FANode::getPostNodes(TransValue transVal)
{
    std::vector<FANode *> result;
    for (FAEdge* pEdge: getEdges(transVal))
    {
        result.push_back(pEdge->destination);
        for (FAEdge* ppEdge: pEdge->destination->getEdges(0))
        {
            result.push_back(ppEdge->destination);
        }
    }
    return result;
}

FANode::FANode(FA* context, EndValue endValue) : context(context)
{
    this->endType = endValue;
    context->addNode(this);
}

void FANode::link(TransValue transValue, FANode* node) {
    this->edges.push_back(new FAEdge(this->context, transValue, node));
}

FAEdge::FAEdge(FA* context) : context(context){
    context->addEdge(this);
}

FAEdge::~FAEdge()
{
    // TODO
}

FAEdge::FAEdge(FA* context, TransValue transValue) : context(context) {
    this->value = transValue;
    context->addEdge(this);
}

FAEdge::FAEdge(FA* context, TransValue transValue, FANode *destination) : FAEdge(context, transValue) {
    this->context = context;
    this->destination = destination;
}

std::vector<FANode *> FA::getCurrStatus()
{
    return this->currStatus;
}

void NFA::transfer(int transVal) throw(NoSolidEdgeOutException)
{
    std::vector<FANode *> tmp;
    for (FANode* pNode: this->currStatus)
    {
        tmp.push_back(pNode);
    }
    this->currStatus.clear();
    for (FANode *pNode: tmp)
    {
        for (FANode *tmpNode: pNode->getPostNodes(transVal))
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

void FA::printCurrState()
{
    for (FANode * node: currStatus)
    {
        printf("state: %d\ndescription: %s\n\n", node->nid, node->note.data());
    }
}

std::vector<EndValue> FA::getEndValues()
{
    std::vector<EndValue> result;
    for (FANode * node: currStatus)
    {
        if (node->endType >= 0)
        {
            result.push_back(node->endType);
        }
    }
    return result;
}

FA::FA()
{
    End = Start = new FANode(this, 0);
    this->currStatus.push_back(Start);
}

FA::FA(TransValue transValue)
{
    this->regex = (char) transValue;
    Start = new FANode(this);
    End = new FANode(this);
    FAEdge* edge = new FAEdge(this, transValue);
    Start->edges.push_back(edge);
    edge->destination = End;
    this->currStatus.clear();
    this->currStatus.push_back(Start);
}

void FA::setEndValue(EndValue endValue)
{
    End->endType = endValue;
}

FA::~FA() {
    for (FANode *node: nodeList)
    {
        SAFE_RELEASE(node);
    }
    for (FAEdge *edge: edgeList)
    {
        SAFE_RELEASE(edge);
    }
}

void FA::addNode(FANode *node) {
    nodeList.push_back(node);
}

void FA::addEdge(FAEdge *edge) {
    edgeList.push_back(edge);
}

void NFA::computeClosure() {
    // compute a closure
    unsigned long lastSize;
    do
    {
        for (int i = 0; i < currStatus.size(); ++i)
        {
            FANode* pNode = currStatus[i];
            for (FANode *tmpNode: pNode->getPostNodes(0))
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

NFA* NFA::parallel(NFA *another, EndValue endValue)
{
    this->regex = this->regex + "|" + another->regex;

    FANode* start = new FANode(this);
    start->link(0, this->Start);
    start->link(0, another->Start);
    FANode* end = new FANode(this, endValue);
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

    FANode *start = new FANode(this);
    FANode *end = new FANode(this);
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

    for (FANode* node: currStatus)
    {
        if (node->endType > -1)
        {
            return true;
        }
    }
    return false;
}
