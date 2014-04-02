#include "Vdd.hpp"

Vdd::Vdd() {
    name = "Vdd";
    type = Type::vddType;
    numMaxConnections = notRestricted;
}

void Vdd::addConnection( Node * n ) {
    Node::addConnection( n );
}
