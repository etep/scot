#ifndef _H_Capacitor
#define _H_Capacitor

#include "Node.hpp"

class Capacitor : public Node {
public:
    Capacitor( string name, vector<Node *> connections, SymbolTable parameters );
};

#endif
