/*
 * Created by alpaca on 16-10-21.
 */

#include "nfa.h"

FANode::FANode() { }

FANode::~FANode() { }

int FANode::maxnid = 0;

std::vector<FAEdge *> FANode::getEdges(int transVal) {
    if (!this->edges)
    {
        return std::vector<FAEdge *>();
    }
    std::vector<FAEdge *> result;
    for (int i = 0; i < MAX_EDGE_AMOUNT; ++i)
    {
        if (this->edges[i] && this->edges[i]->value == transVal)
        {
            result.push_back(this->edges[i]);
        }
    }
    return result;
}

std::vector<FANode *> FANode::getPostNodes(int transVal) {
    std::vector<FANode *> result;
    for (FAEdge * pEdge: getEdges(transVal))
    {
        result.push_back(pEdge->destination);
        for (FAEdge * ppEdge: pEdge->destination->getEdges(0))
        {
            result.push_back(ppEdge->destination);
        }
    }
    return result;
}

FAEdge::FAEdge() { }

FAEdge::~FAEdge()
{
    // TODO
}

std::vector<FANode *> FA::getCurrStatus() {
    return this->currStatus;
}

void FA::transfer(int transVal) {
    std::vector<FANode *> tmp(this->currStatus);
    this->currStatus.clear();
    for (FANode *pNode: tmp)
    {
        this->currStatus.push_back(pNode);
    }

    // find a closure
    unsigned long lastSize = currStatus.size();
    do
    {
        for (FANode *pNode: currStatus)
        {
            for (FANode *tmpNode: pNode->getPostNodes(0))
            {
                currStatus.push_back(tmpNode);
            }
        }
    } while (lastSize != currStatus.size());
}
