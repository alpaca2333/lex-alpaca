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


NFANode::NFANode(NFA* context) : context(context)
{
    context->addNode(this);
}

NFANode::NFANode(NFA* context, int endValue) : context(context)
{
    this->setEndType(endValue, 0);
    context->addNode(this);
}

NFANode::~NFANode() { }

int NFANode::maxnid = 0;

EndValueType::EndValueType(const EndValueType& another)
{
    this->value = another.value;
    this->priority = another.priority;
}

EndValueType &EndValueType::operator=(const EndValueType &another)
{
    this->priority = another.priority;
    this->value = another.value;
    return *this;
}

bool EndValueType::operator<(EndValueType& another)
{
    return this->priority > another.priority;
}

bool EndValueType::operator>(EndValueType& another)
{
    return this->priority < another.priority;
}

bool EndValueType::operator==(EndValueType& another)
{
    return this->priority == another.priority;
}

bool EndValueType::operator>=(EndValueType& another)
{
    return this->priority <= another.priority;
}

bool EndValueType::operator<=(EndValueType& another)
{
    return this->priority <= another.priority;
}


std::vector<NFAEdge *> NFANode::getEdges(TransValue transVal)
{
    std::vector<NFAEdge *> result;
    for (int i = 0; i < edges.size(); ++i)
    {
        if (this->edges[i] && this->edges[i]->check(transVal))
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

void NFANode::link(TransValue transValue, NFANode* node)
{
    this->edges.push_back(new NFAEdge(this->context, transValue, node));
}

void NFANode::link(const char *seq, NFANode* node)
{
    this->edges.push_back(new NFAEdge(this->context, seq, node));
}

void NFANode::setEndType(int type, int priority)
{
    this->endType.value = type;
    this->endType.priority = priority;
}



NFAEdge::NFAEdge(NFA* context) : context(context)
{
    if (context)
        context->addEdge(this);
    this->allowedValues.push_back(0);
}

NFAEdge::~NFAEdge()
{
    // TODO
}

NFAEdge::NFAEdge(NFA* context, TransValue transValue) : context(context)
{
    if (context)
        context->addEdge(this);
    this->allowedValues.push_back(transValue);
}

NFAEdge::NFAEdge(NFA* context, TransValue transValue, NFANode *destination) : NFAEdge(context, transValue) {
    this->destination = destination;
}

NFAEdge::NFAEdge(NFA *context, const char *seq, NFANode* destination)
{
    this->context = context;
    this->destination = destination;
    if (!strcmp(seq, "."))
    {
        for (TransValue i = MIN_TRANSVALUE; i <= MAX_TRANSVALUE; ++i) {
            if (i != '\n')
            {
                allowedValues.push_back(i);
            }
        }
    }
    else if (strlen(seq) == 3 && seq[1] == '-')
    {
        for (int i = seq[0]; i <= seq[2]; ++i)
        {
            allowedValues.push_back(i);
        }
    }
    else if (strlen(seq) > 1 && seq[0] == '^')
    {
        for (int i = MIN_TRANSVALUE; i <= MAX_TRANSVALUE; ++i)
        {
            bool banned = false;
            for (int j = 1; j < strlen(seq); ++j)
            {
                if (i == seq[j])
                {
                    banned = true;
                    break;
                }
            }
            if (!banned)
            {
                allowedValues.push_back(i);
            }
        }
    }
    else
    {
        throw InvalidNodeSeqException(seq);
    }
    if (context)
        context->addEdge(this);
}

bool NFAEdge::check(TransValue c)
{
    for (TransValue value: allowedValues)
    {
        if (c == value)
        {
            return true;
        }
    }
    return false;
}

NFAEdge *NFAEdge::merge(NFAEdge *another)
{
    for (TransValue value: another->allowedValues)
    {
        this->allowedValues.push_back(value);
    }
    return this;
}

NFAEdge *NFAEdge::not_()
{
    std::vector<TransValue> tmp = this->allowedValues;
    this->allowedValues.clear();
    for (TransValue i = MIN_TRANSVALUE; i <= MAX_TRANSVALUE; ++i)
    {
        this->allowedValues.push_back(i);
        for (TransValue v: tmp)
        {
            if (v == i)
            {
                this->allowedValues.pop_back();
                break;
            }
        }
    }
    return this;
}


std::vector<NFANode *> NFA::getCurrStatus()
{
    return this->currStatus;
}

void NFA::transfer(int transVal) throw(NoSolidEdgeOutException)
{
    std::vector<NFANode *> tmp;
    tmp = currStatus;
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
        currStatus = tmp;
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

std::vector<EndType> NFA::getEndType()
{
    std::vector<EndType> result;
    for (NFANode * node: currStatus)
    {
        if (node->endType.value >= 0)
        {
            result.push_back(node->endType);
        }
    }
    return result;
}

void NFA::setEndType(int endValue, int priority)
{
    End->setEndType(endValue, priority);
}

NFA::~NFA()
{
    for (NFANode *node: nodeList)
    {
        SAFE_RELEASE(node);
    }
    for (NFAEdge *edge: edgeList)
    {
        SAFE_RELEASE(edge);
    }
}

void NFA::addNode(NFANode *node)
{
    nodeList.push_back(node);
}

void NFA::addEdge(NFAEdge *edge)
{
    edgeList.push_back(edge);
}

void NFA::computeClosure()
{
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

NFA* NFA::parallel(NFA *another, int endValue)
{
    this->strRegex = this->strRegex + "|" + another->strRegex;

    NFANode* start = new NFANode(this);
    start->link(0, this->Start);
    start->link(0, another->Start);
    NFANode* end = new NFANode(this, endValue);
    this->End->link(0, end);
    another->End->link(0, end);

    this->Start = start;
    this->End = end;

    another->giveUpResource();
    resetState();
    return this;
}

NFA* NFA::repeat(int repeatMode)
{
    if (repeatMode == REPEAT_1_N)
    {
        strRegex = "(" + strRegex + ")+";
    }
    else if (repeatMode == REPEAT_0_N)
    {
        strRegex = "(" + strRegex + ")*";
    }
    else if (repeatMode == REPEAT_0_1)
    {
        strRegex = "(" + strRegex + ")?";
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

NFA* NFA::concat(NFA *another)
{
    this->strRegex = "(" + this->strRegex + ")" + another->strRegex;
    this->End->link(0, another->Start);
    this->End = another->End;

    another->giveUpResource();
    resetState();
    return this;
}

bool NFA::matches(const char *seq)
{
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
        if (node->endType.value > -1)
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
    this->strRegex = (char) transValue;
    Start = new NFANode(this);
    End = new NFANode(this);
    Start->link(transValue, End);
    this->currStatus.clear();
    this->currStatus.push_back(Start);
}

NFA::NFA(const char *seq) {
    this->strRegex = seq;
    this->strRegex = "[" + strRegex + "]";
    Start = new NFANode(this);
    End = new NFANode(this);
    Start->link(seq, End);
    this->currStatus.clear();
    this->currStatus.push_back(Start);
}

void NFA::giveUpResource() {
    nodeList.clear();
    edgeList.clear();
}

NFA::NFA(NFAEdge *edge)
{
    strRegex = "";
    for (TransValue value: edge->allowedValues)
    {
        strRegex += (strRegex == "" ? "" : "|") + (char) value;
    }
    Start = new NFANode(this);
    End = new NFANode(this);
    Start->edges.push_back(edge);
    edge->destination = End;
    this->edgeList.push_back(edge);
    resetState();
}

EndType NFA::getPreferredEndType()
{
    std::vector<EndType> ends = getEndType();
    if (!ends.size())
    {
        throw NotAcceptedStateException();
    }
    EndType curr = ends[0];
    for (int i = 1; i < ends.size(); ++i)
    {
        if (curr < ends[i])
        {
            curr = ends[i];
        }
    }
    return curr;
}

void NFA::setOnTokenAccepted(std::function<void(int type, const char *token)> cb)
{
    this->onTokenAccepted = cb;
}

void NFA::setOnCharacterUnaccepted(std::function<void(char c, int position)> cb)
{
    this->onCharacterUnaccepted = cb;
}

void NFA::read(const char *seq)
{
    resetState();

    std::string buf = "";
    for (int i = 0; i < strlen(seq); ++i)
    {
        try
        {
            transfer(seq[i]);
            buf += seq[i];
        }
        catch (NoSolidEdgeOutException&)
        {
            EndType endType;
            try
            {
                endType = getPreferredEndType();
            }
            catch (NotAcceptedStateException&)
            {
                onCharacterUnaccepted(seq[i], i);
                resetState();
                continue;
            }
            --i;
            onTokenAccepted(endType.value, buf.data());
            buf = "";
            resetState();
        }
    }

    EndType endType;
    try
    {
        endType = getPreferredEndType();
    }
    catch (NotAcceptedStateException&)
    {
        resetState();
    }
    onTokenAccepted(endType.value, buf.data());
}
