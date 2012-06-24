#ifndef _H_InternalCap
#define _H_InternalCap

#include "Transistor.hpp"
#include "utility.hpp" // for typedef

class InternalCap : public Transistor {
private:

public:

   InternalCap( string name, Type * type, Node * drainNode, Node * otherNodes, SymbolTable parameters );

   void addConnectionAt( Node * n, int location );
   Node * getConnectionAt( int location );

   vector<Node *> getChildren( Node * goal );
   string getParameter( string key );

};

#endif
