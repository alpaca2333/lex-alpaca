//
// Created by alpaca on 16-10-21.
//

#include "nfa.h"

FANode::FANode()
{
    edges = new FAEdge[256];
}

FANode::~FANode()
{
    delete[] edges;
}
