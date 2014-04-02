#include <stdio.h>
#include <stdlib.h>
#include "Subcircuit.hpp"
#include "ModelPrefix.hpp"
#include <iostream>
#define MAX_NUMBER_LENGTH 10


Subcircuit::Subcircuit( string name, vector<Node *> outputs, vector<Node *> inputs, SymbolTable params, vector<Transistor *> trans, vector<Node *> internals ) {
    this->name = name;
    this->type = Type::subcircuitType;
    this->outputVec = outputs;
    this->connections = inputs;
    this->parameters = params;
    this->transistors = trans;
    this->numMaxConnections = inputs.size();
    this->internals = internals;
}

void Subcircuit::addConnectionAt( Node * n, int where ) {
    cerr << "You should not be adding a connection to a subcircuit";
    cerr << " after its creation." << endl;
    exit( 1 );
}

Node * Subcircuit::getOutputNode( unsigned i ) {
    return outputVec[i];
}


Node * Subcircuit::getInternalNode( unsigned i ) {
    return internals[i];
}

unsigned Subcircuit::getNumOutputs() {
    return outputVec.size();
}

unsigned Subcircuit::getNumInternals() {
    return internals.size();
}

vector<Node *> Subcircuit::getOutputNodeVec() {
    return outputVec;
}

vector<Node *> Subcircuit::getIntNodeVec() {
    return internals;
}



void Subcircuit::generateDio( ostream & o, SymbolTable constants, bool printToFile ) {
    vector<Node *>::iterator i = connections.begin();

    // for each input
    while ( i != connections.end() ) {
        Input * currentInput = ( Input * )( *i );
        for( unsigned n = 0; n < outputVec.size(); n++ ) {
            cout << "finding paths for input " << currentInput->getName() << " to "
                 << outputVec[n]->getName() << " of CCC " << getName() << endl;
            currentInput->findPaths( this, n );

            currentInput->generateDio( constants, fallToRiseNor, outputVec[n], this,n );
            currentInput->generateDio( constants, fallToRiseStd, outputVec[n], this,n );
            currentInput->generateDio( constants, riseToFallNor, outputVec[n], this,n );
            currentInput->generateDio( constants, riseToFallStd, outputVec[n], this,n );

            if ( printToFile ) { printDio( o, currentInput,n ); }
        }

        i++;
    }
    cout << "finished generating dios" << endl;
}

void Subcircuit::insertTxGateNodes( pair<Node *, Node *> * txGateNode ) {
    if ( !containTxGateNodes( txGateNode ) ) {
        txGateNodes.push_back( txGateNode );
    }
}

bool Subcircuit::containTxGateNodes( pair<Node *, Node *> * txGateNode ) {
    vector<pair<Node *, Node *> *>::iterator i = txGateNodes.begin();

    while ( i != txGateNodes.end() ) {
        pair<Node *, Node *> * currentPair = *i;
        if ( ( currentPair->first->getName() == txGateNode->first->getName() &&
                currentPair->second->getName() == txGateNode->second->getName() ) ||
                ( currentPair->first->getName() == txGateNode->second->getName() &&
                  currentPair->second->getName() == txGateNode->first->getName() ) ) {
            return true;
        }

        i++;
    }
    return false;
}

void Subcircuit::putAntiCorrInput( string s1, string s2 ) {
    //        cout << "Reached here for inputs "<< s1<< " and " << s2 << " of " << getName() << endl;
    //first verify that the input names are correct
    bool s1Exists = false;
    bool s2Exists = false;
    for( int i = 0; i < connections.size(); i++ ) {
        if( s1 == connections[i]->getName() ) {
            s1Exists = true;
        }
        if( s2 == connections[i]->getName() ) {
            s2Exists = true;
        }
    }
    if( !s1Exists ) {
        cout << "AntiCorr error: Input " << s1 << " does not exist in CCC " << getName() <<endl;
        exit( -1 );
    }
    if( !s2Exists ) {
        cout << "AntiCorr error: Input " << s2 << " does not exist in CCC " << getName() <<endl;
        exit( -1 );
    }
    if( AntiCorrInputs.find( s1 ) != AntiCorrInputs.end() ) {
        cout << "The input " << s1 << " is already Anti correlated to input " << AntiCorrInputs[s1] << endl;
        exit( -1 );
    }
    else {
        AntiCorrInputs[s1] = s2;
    }
}

/*
bool Subcircuit::isAntiCorr(string s1, string s2)
{
         if(AntiCorrInputs.find(s1) != AntiCorrInputs.end() && AntiCorrInputs[s1] == s2
                             || AntiCorrInputs.find(s2) != AntiCorrInputs.end() && AntiCorrInputs[s2] == s1)
                    return true;
         else
                    return false;
}
*/

map<string, string> & Subcircuit::getAntiCorrInputMap() {
    return AntiCorrInputs;
}

void Subcircuit::recIntPrechargeInputs( vector<string> ports ) {
    bool s1Exists = false;
    cout << "Recording input ports ";
    for( int p=0; p<ports.size(); p++ ) {
        for( int i = 0; i < connections.size(); i++ ) {
            if( ports[p] == connections[i]->getName() ) {
                s1Exists = true;
            }
        }
        if( !s1Exists ) {
            cout << "IntPrecharge statement error: Input " << ports[p] << " does not exist in CCC " << getName() <<endl;
            exit( -1 );
        }
        cout << ports[p] << " ";
        IntPrechargeInputs[ports[p]] = 1;
    }
    cout << "as the int precharge inputs of CCC " << getName() << endl;
}

/*
bool isIntPrechargeInput(string s1)
{
         if(IntPrechargeInputs.find(s1) == IntPrechargeInputs.end())
                    return false;
         else
                    return true;
}
*/

map<string, int> & Subcircuit::getIntPrechargeInputMap() {
    return IntPrechargeInputs;
}


void Subcircuit::printDio( ostream & o, Input * input, unsigned no ) {

    string header = "";
    //  o << name << "." << input->getName() << "." << outputVec[no]->getName() << endl;
    if ( input->getRiseToFallDioNor( no ) != "" ) {
        if( header == "" ) {
            header = name+"."+input->getName()+"."+outputVec[no]->getName();
            o << header << endl;
        }
        o << "rf : " << input->getRiseToFallDioNor( no ) << " ;\n" << endl;
    }
    if ( input->getRiseToFallDioStd( no ) != "" ) {
        if( header == "" ) {
            header = name+"."+input->getName()+"."+outputVec[no]->getName();
            o << header << endl;
        }
        o << "std : " << input->getRiseToFallDioStd( no ) << " ;\n" << endl;
    }
    if ( input->getFallToRiseDioNor( no ) != "" ) {
        if( header == "" ) {
            header = name+"."+input->getName()+"."+outputVec[no]->getName();
            o << header << endl;
        }
        o << "fr : " << input->getFallToRiseDioNor( no ) << " ;\n" << endl;
    }
    if ( input->getFallToRiseDioStd( no ) != "" ) {
        if( header == "" ) {
            header = name+"."+input->getName()+"."+outputVec[no]->getName();
            o << header << endl;
        }
        o << "std : " << input->getFallToRiseDioStd( no ) << " ;\n" << endl;
    }
    o << "\n" << endl;
}




int Subcircuit::generateTxGateConstraints( ostream & o ) {
    if ( txGateNodes.size() == 0 ) { return 0; } // contains no tramission gate

    o << name << " : ";
    vector<pair<Node *, Node *> *>::iterator i = txGateNodes.begin();
    while ( i != txGateNodes.end() ) {
        pair<Node *, Node *> * currentPair = *i;
        o << currentPair->first->getName() << " = "
          << currentPair->second->getName();

        i++;
        if ( i == txGateNodes.end() ) { o << ";"; }
        else { o << ", "; }
    }
    cout << endl;
    return 1;
}


void Subcircuit::resetHasVisited() {
    vector<Node *>::iterator i = internals.begin();

    while ( i != internals.end() ) {
        //            cout << "Reset the visiting of " << (*i)->getName() << endl;
        ( *i )->resetHasVisited();
        i++;
    }

    vector<Transistor *>::iterator j = transistors.begin();
    while ( j != transistors.end() ) {
        ( *j )->resetHasVisited();
        j++;
    }
}


string Subcircuit::getLeakCurrExpr( string name,SymbolTable params, unsigned no ) {
    string LeakCurr = "";
    if ( params.find( name ) == params.end() ) {
        cout << "model " + name + " was not found!" << endl;
        return LeakCurr;
    }
    string model;
    int count=0;
    double fraction= 0.5;
    for ( int j=0; j< transistors.size(); j++ ) {
        model = params[name];
        if(  ( ( transistors[j]->getType()->equals( Type::nmosType ) ) &&
                ( name == nLeakNom || name == nLeakStat ) ) ||
                ( ( transistors[j]->getType()->equals( Type::pmosType ) ) &&
                  ( name == pLeakNom || name == pLeakStat ) ) ) {
            count ++;
            if( LeakCurr == "" ) {
                LeakCurr = searchReplaceString( model, width,
                                                transistors[j]->getParameter( effectiveWidth ) );
            }
            else {
                LeakCurr += " + " + searchReplaceString( model, width,
                            transistors[j]->getParameter( effectiveWidth ) );
            }
        }
    }
    fraction = 1.0/count;
    char st[MAX_NUMBER_LENGTH];
    sprintf( st, "%.4f",fraction );
    if( LeakCurr == "" ) {
        return LeakCurr;
    }
    else {
        if( fraction < 0.9 ) { //this is just to check if count > 1?
            LeakCurr = "(" + string( st ) + "( "+ LeakCurr + "))";
        }
        else {
            LeakCurr = "("+ LeakCurr + ")";
        }
        return LeakCurr;
    }
}



string Subcircuit::searchReplaceString( string model, string toReplace, string replaceWith ) {
    int foundPos = 0;
    int currentPos = 0;
    string newstr = model;
    for ( currentPos = 0; currentPos < newstr.length(); currentPos++ ) {
        foundPos = newstr.find( toReplace, currentPos );
        if ( foundPos != string::npos ) {
            newstr.replace( foundPos, toReplace.size(), replaceWith );
        }
        else {
            return newstr;
        }
        currentPos = foundPos + replaceWith.size();
    }

    return newstr;
}

