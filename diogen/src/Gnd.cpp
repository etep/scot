#include "Gnd.hpp"

Gnd::Gnd() {
   name = "Gnd";
   type = Type::gndType;
   numMaxConnections = notRestricted;
}

void Gnd::addConnection( Node * n ) {
   Node::addConnection( n );
}

