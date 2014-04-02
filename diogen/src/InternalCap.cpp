#include "InternalCap.hpp"
#include <iostream>
#include "Vdd.hpp"
#include "Gnd.hpp"
#include "ModelPrefix.hpp"

InternalCap::InternalCap( string name, Type * type, Node * drainNode, Node * sourceNode, Node * gateNode, Node * bodyNode, SymbolTable parameters ) {
    this->name = name;
    this->type = type;
    this->numMaxConnections = 2;
    this->parameters = parameters;

    addConnectionAt( drainNode,   DrainConnection  );
    addConnectionAt( sourceNodes, SourceConnection );
    addConnectionAt( gateNodes,   GateConnection   );
    addConnectionAt( bodyNodes,   BodyConnection   );
}

Node * InternalCap::getConnectionAt( int location ) {
    if( location < 2 ) {
        return connections[location];
    }
    cerr << "you are trying to get an invalid connection on a internal capacitor of a ccc" << endl;
    exit( 1 );
}

string InternalCap::getParameter( string key ) {
    if( key == capValue ) {
        return parameters[capValue];
    }
    else {
        cerr << " trying to ask for non cap attributes from a capacitor node " << endl;
        exit( 1 );
    }
    return "";
}

void InternalCap::addConnectionAt( Node * n, int where ) {
    Node::addConnectionAt( n, where );
}

vector<Node *> InternalCap::getChildren( Node * goal ) {
    vector<Node *> v;
    return v;
}

