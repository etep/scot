#include "Capacitor.hpp"

Capacitor::Capacitor( string name, vector<Node *>connections, SymbolTable parameters ) {
    this->name = name;
    this->connections = connections;
    this->parameters = parameters;
    this->type = Type::capType;
}
