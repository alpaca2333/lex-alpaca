

class FAEdge;
class FANode;

class FANode
{
public:
    FANode();

    ~FANode();

    const int nid = ++maxnid;

    /* edges that goes out of the node */
    FAEdge* edges;
private:
    static int maxnid;
};

class FAEdge
{
public:

    /* the value of the transporting character,
     * zero if it is epsilon
     *
     * considering supporting unicode :P
     */
    int value;

    /* destination of the edges */
    FANode* destination;
private:
};

int FANode::maxnid = 0;