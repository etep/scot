#ifndef _H_CircuitParser
#define _H_CircuitParser

#include <vector>
#include <map>
#include <ostream>
#include "Subcircuit.hpp"
#include "TransitionTable.hpp"
#include "utility.hpp"


using namespace std;


class CircuitParser {
private:
    //int cmp_nocase(const string& s, const string& s2);

    SymbolTable constants;
    map<string, Subcircuit *> subcircuits;
    map<string, Node *> elements; // includes instances and capacitors, etc
    map<string, Node *> internals;
    map<string, vector<Node *> > capNetNodeMap;
    vector<pair<string,double> > primaryInputs;

    vector<SymbolTable> irsimData;

    string convertToLower( string in );

    string generateRandomVector( int length, std::vector<std::string> prevStimulus,vector<double> inputAf,  double defActFact );

    void processOneTransitionEntry( TransitionTable * t,
                                    SymbolTable s );

    void createTransitionTableEntries( TransitionTable * t,
                                       SymbolTable s );

public:
    CircuitParser() {
        capNetNodeMap.clear();
    }

    // functions used during parsing
    void setConstants( SymbolTable constants );
    void setSubcircuits( map<string, Subcircuit *> sub );
    Subcircuit * getSubcircuit( string name );

    void setElements( map<string, Node *> elements );
    void insertInternal( string name, Node * internal );
    void insertCapNetNode( string CCCname, Node * net );
    vector<Node *> getCapNetNodes( string CCCname );
    void setAntiCorrInfo( string name, string in1, string in2 );
    void setIntPrechargeInputInfo( string name, vector<string> preIns );
    Node * getInternal( string name );

    void setPrimaryInputs( vector<pair<string,double> > inputs );

    void setIrsimData( vector<SymbolTable> irsimData );

    // functions used after parsing
    void generateDio( ostream & o, bool printToFile );
    void generateTxGateConstraints( ostream & o );

    void generateIrsimInput( ostream & o, string filename, int numRuns, double actFact );

    void generateActivityFactors( ostream & o );

    void generatePowerExpressionsInDioFile( ostream & o );
    void generateLeakageExpressionsInDioFile( ostream & o );
    void printConstants();


};

#endif
