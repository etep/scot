#ifndef _H_Transistor
#define _H_Transistor

#include "Node.hpp"
#include "utility.hpp" // for typedef

class Transistor : public Node {
private:

public:

   static const int DrainConnection = 0;
   static const int SourceConnection = 1;
   static const int GateConnection = 2;
   static const int BodyConnection = 3;

   Transistor( string name, Type * type, Node * drainNode, Node * sourceNode, Node * gateNode, Node * bodyNode, SymbolTable parameters );

   void addConnectionAt( Node * n, int location );
   Node * getConnectionAt( int location );

   vector<Node *> getChildren( Node * goal );
   string getParameter( string key );

};

#endif
