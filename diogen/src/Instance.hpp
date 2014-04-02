#ifndef _H_Instance
#define _H_Instance

#include "Subcircuit.hpp"

class Instance : public Node {
private:
    vector<Node *> outputNodeVec;
    Subcircuit * subcircuit;

public:
    Instance( string name, Subcircuit * sub, vector<Node *> outputs, vector<Node *> inputs );

    Node * getOutputNode( unsigned no );
    string getOutputParasiticCap( unsigned no );
    unsigned getNumOutputs();
};

#endif
