#ifndef _H_Subcircuit
#define _H_Subcircuit

#include "Node.hpp"
#include "Transistor.hpp"
#include "Input.hpp"
#include <ostream>
#include "utility.hpp"
#include <string>

class Subcircuit : public Node {
private:
    vector<Node *> outputVec;
    SymbolTable parameters;
    vector<Transistor *> transistors;
    vector<Node *> internals;
    vector<pair<Node *, Node *> *> txGateNodes;
    //storing the names of inputs that are necessarily opposite of each other..like dual rail inputs.
    map<string, string> AntiCorrInputs;
    map<string, int> IntPrechargeInputs; //its just a vector but map is easy to search in.


    void printDio( ostream & o, Input * input, unsigned no );
    bool containTxGateNodes( pair<Node *, Node *> * txGateNode );

public:
    Subcircuit( string name, vector<Node *> outputs, vector<Node *> inputs, SymbolTable params, vector<Transistor *> trans, vector<Node *> internals );

    void addConnectionAt( Node * n, int where );
    Node * getOutputNode( unsigned i );
    Node * getInternalNode( unsigned i );
    unsigned getNumOutputs();
    unsigned getNumInternals();
    vector<Node *> getOutputNodeVec();
    vector<Node *> getIntNodeVec();

    void generateDio( ostream & o, SymbolTable constants, bool printToFile );
    int generateTxGateConstraints( ostream & o );
    void resetHasVisited();
    string getLeakCurrExpr( string name, SymbolTable params, unsigned no );
    string searchReplaceString( string model, string toReplace, string replaceWith );
    void insertTxGateNodes( pair<Node *, Node *> * txGateNode );
    void putAntiCorrInput( string s1, string s2 );
    //   bool isAntiCorr(string s1, string s2);
    map<string, string> & getAntiCorrInputMap();
    void recIntPrechargeInputs( vector<string> ports );
    map<string,int> & getIntPrechargeInputMap();
};

#endif
