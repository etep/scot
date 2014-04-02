#ifndef _H_TransitionTable_h
#define _H_TransitionTable_h

using namespace std;

#include <string>
#include <map>
#include <ostream>
#include "Node.hpp"


class TransitionTableEntry {
public:
    int lastValue;
    int numLowToHighTransitionsFound;
    int numHighToLowTransitionsFound;
    int numTransitions;

    string name;

    TransitionTableEntry( string name, int initValue );
};


class TransitionTable {
private:
    map<string, TransitionTableEntry *> table;

public:
    void updateEntry( string name, int value );
    void createEntry( string name, int initValue );

    void calculateActivityFactors( map<string, Node *> internals );
    void printLowToHighFactors( ostream & o );

};


#endif
