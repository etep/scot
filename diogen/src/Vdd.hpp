#ifndef _H_Vdd
#define _H_Vdd

#include "Node.hpp"

class Vdd : public Node {
public:
    Vdd();

    void addConnection( Node * n );
};

#endif
