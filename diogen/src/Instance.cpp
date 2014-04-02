#include "Instance.hpp"
#include <assert.h>

Instance::Instance( string name, Subcircuit * sub, vector<Node *> outputs, vector<Node *> inputs ) {
    this->name = name;
    this->subcircuit = sub;
    this->type = Type::instanceType;

    this->outputNodeVec = outputs;
    this->connections = inputs;


}

Node * Instance::getOutputNode( unsigned no ) {
    assert( no < outputNodeVec.size() );
    return outputNodeVec[no];
}

unsigned Instance::getNumOutputs() {
    return outputNodeVec.size();
}

string Instance::getOutputParasiticCap( unsigned no ) {
    string combinedCap = "0.5 * ( ";
    combinedCap += subcircuit->getOutputNode( no )->getFallCapExpr();
    combinedCap += " + ";
    combinedCap += subcircuit->getOutputNode( no )->getRiseCapExpr();
    combinedCap += " )";

    return combinedCap;
}
