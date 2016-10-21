#pragma once

#include <vector>
#include "stdafx.h"

class FAEdge;
class FANode;

#define MAX_NODE_AMOUNT 1024
#define MAX_EDGE_AMOUNT 256

typedef FANode* PFANode;
typedef FAEdge* PFAEdge;

class FANode
{
public:

    FANode();

    ~FANode();

    const int nid = ++maxnid;

    /* edges that goes out of the node */
    FAEdge* edges[MAX_EDGE_AMOUNT];

    std::vector<FAEdge *> getEdges(int transVal);

    std::vector<FANode *> getPostNodes(int transVal);
private:
    static int maxnid;

    /* zero if this is non-terminal node, else
     * a positive integer representing a specified
     * terminal status.
     */
    int endValue;
};

class FAEdge
{
public:

    FAEdge();

    ~FAEdge();

    /* the value of the transporting character,
     * zero if it is epsilon
     *
     * considering supporting unicode :P
     */
    int value;

    /* destination of the edges */
    FANode* destination = NULL;
};

class FA {
public:

protected:
    FANode* S = new FANode();

    std::vector<FANode *> getCurrStatus();

    void transfer(int transVal);

private:
    std::vector<FANode *> currStatus;
};