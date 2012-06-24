#ifndef _H_Internal
#define _H_Internal

#include "Node.hpp"

class Internal : public Node {
   //friend class Input;
private:

   void calculateCapExpr( SymbolTable constants, string modelPrefix, Node * outputNode, bool rise );

public:
   Internal( string name );

   void addConnection( Node * n );

   vector<Node *> getChildren( Node * goal ) {
      return connections;
   }
   
   string getFallCapExpr ();
   string getAvgCapExpr  ();
   string getWireCapExpr ();
   string getRiseCapExpr ();
   
   string getFallCapExpr ( SymbolTable constants, string modelPrefix, Node * outputNode );
   string getAvgCapExpr  ( SymbolTable constants, string modelPrefix, Node * outputNode );
   string getRiseCapExpr ( SymbolTable constants, string modelPrefix, Node * outputNode );
};

#endif
