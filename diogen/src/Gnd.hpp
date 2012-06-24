#ifndef _H_Gnd
#define _H_Gnd

#include "Node.hpp"

class Gnd : public Node {
public:

   Gnd();

   void addConnection( Node * n );
};

#endif
