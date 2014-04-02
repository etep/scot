#include "Input.hpp"
#include "Subcircuit.hpp"
#include "Vdd.hpp"
#include "Gnd.hpp"
#include "Path.hpp"
#include "ModelPrefix.hpp"

//class Subcircuit;


Input::Input( Node * intNode, unsigned no ) {
    this->name = intNode->name;
    this->type = Type::inputType;
    this->connections = intNode->connections;
    this->numMaxConnections = intNode->numMaxConnections;
    this->hasVisited = intNode->hasVisited;
    numOutputs = no;
    pathsToGndVec.clear();
    pathsToGndVec.resize( no );
    pathsToVddVec.clear();
    pathsToVddVec.resize( no );
    riseToFallDioStdVec.clear();
    riseToFallDioStdVec.resize( no );
    riseToFallDioNorVec.clear();
    riseToFallDioNorVec.resize( no );
    fallToRiseDioStdVec.clear();
    fallToRiseDioStdVec.resize( no );
    fallToRiseDioNorVec.clear();
    fallToRiseDioNorVec.resize( no );


    // initialize all dio expressions to be 0
    /*
    this->riseToFallDioStd = "0";
    this->riseToFallDioNor = "0";
    this->fallToRiseDioStd = "0";
    this->fallToRiseDioNor = "0";
    */
}


void Input::findPaths( Subcircuit * parent, unsigned no ) {

    Vdd * v = new Vdd();
    Gnd * g = new Gnd();

    if( connections.size() > 2 ) {
        cout << "WARNING: Input " << name << " has more than 2 connections." << endl;
        //      exit(1);
    }

    for( int i = 0; i < connections.size(); i++ ) {
        Path currentPath_1;
        Path currentPath_2;
        vector<Path> finishedPaths_1;
        vector<Path> finishedPaths_2;

        Path currentPathToVdd;
        Path currentPathToGnd;
        Path currentPathToOutput;
        vector<Path> finishedPathsToVdd;
        vector<Path> finishedPathsToGnd;
        vector<Path> finishedPathsToOutput;
        map<string,string> AntiCorrInputMap = parent->getAntiCorrInputMap();
        map<string,int> IntPrechargeInputMap = parent->getIntPrechargeInputMap();

        // PLEASE NOTE THAT ALTHOUGH I HAVE AntiCorrInputMap AS INPUT TO ALL THE PATH FINDING ROUTINES
        // I AM NOT USING IT ANYWHERE WHILE FINDING PATHS. SO ITS JUST A DUMMY ARGUMENT.
        // I USE IT LATER TO WEED OUT WRONG PATHS FROM THE combined vector of paths.

        /*
        if (connections[i]->getType()->equals(Type::pmosType))
        connections[i]->findPathBetween(v, parent->getOutputNode(), currentPath_1, finishedPaths_1);
             else
        connections[i]->findPathBetween(g, parent->getOutputNode(), currentPath_1, finishedPaths_1);
             */
        parent->resetHasVisited();
        // find paths to vdd and gnd
        cout << "Connection's name is " << connections[i]->getName() << endl;
        connections[i]->findPathBetween( v, parent->getOutputNodeVec(),
                                         currentPathToVdd, finishedPathsToVdd, AntiCorrInputMap );
        //            cout << "  Vdd this is the problem " << finishedPathsToVdd.size() << endl;
        parent->resetHasVisited();

        connections[i]->findPathBetween( g, parent->getOutputNodeVec(),
                                         currentPathToGnd, finishedPathsToGnd, AntiCorrInputMap );
        //            cout << " Gnd this is the problem " << finishedPathsToGnd.size() << endl;
        parent->resetHasVisited();

        // find paths to output

        /*
        connections[i]->findPathBetween(parent->getOutputNode(),
                    parent->getOutputNode(),
                    currentPath_2, finishedPaths_2);
        */

        connections[i]->findPathBetween( parent->getOutputNode( no ),
                                         parent->getOutputNodeVec(),
                                         currentPathToOutput,
                                         finishedPathsToOutput, AntiCorrInputMap );
        //            cout << " Out this is the problem " << finishedPathsToOutput.size() << endl;
        parent->resetHasVisited();


        // combine the paths
        /*
        if (connections[i]->getType()->equals(Type::pmosType))
        pmosPaths = combinePaths(finishedPaths_2, finishedPaths_1);
             else nmosPaths = combinePaths(finishedPaths_2, finishedPaths_1);
             */

        vector<Path> combined = combinePaths( finishedPathsToOutput,
                                              finishedPathsToGnd );


        for ( int i = 0; i < combined.size(); i++ ) {
            const bool mapsEmpty  = AntiCorrInputMap.empty() && IntPrechargeInputMap.empty();
            const bool noAntiCorr = !( combined[i].containsAntiCorrOrIntPrechargeInputs( AntiCorrInputMap, IntPrechargeInputMap ) );
            if( mapsEmpty || noAntiCorr ) {
                pathsToGndVec[no].push_back( combined[i] );
            }
        }

        combined = combinePaths( finishedPathsToOutput,
                                 finishedPathsToVdd );

        for ( int i = 0; i < combined.size(); i++ ) {
            const bool mapsEmpty  = AntiCorrInputMap.empty() && IntPrechargeInputMap.empty();
            const bool noAntiCorr = !( combined[i].containsAntiCorrOrIntPrechargeInputs( AntiCorrInputMap,IntPrechargeInputMap ) );
            if( mapsEmpty  || noAntiCorr ) {
                pathsToVddVec[no].push_back( combined[i] );
            }
        }

    }



    cout << "Final paths to gnd" << endl;
    for( int i = 0; i < pathsToGndVec[no].size(); i++ ) {
        pathsToGndVec[no][i].print( cout );
    }

    cout << "Final paths to vdd" << endl;
    for( int i = 0; i < pathsToVddVec[no].size(); i++ ) {
        pathsToVddVec[no][i].print( cout );
    }

    cout << endl << endl;
}


void Input::generateDio( SymbolTable params, string modelPrefix, Node * output, Subcircuit * parent ) {

    const bool pfxIsFTRNor = modelPrefix == fallToRiseNor;
    const bool pfxIsFTRStd = modelPrefix == fallToRiseStd;

    // prefix is fall-to-rise
    const bool pfxIsFTR = pfxIsFTRNor || pfxIsFTRStd;

    vector<Path> paths = pfxIsFTR ? pathsToVdd : pathsToGnd;

    for( int j = 0; j < connections.size(); j++ ) {
        Node * switchNode = connections[j];
        for( int i = 0; i < paths.size(); i++ ) {
            if( paths[i].contains( switchNode ) ) {
                paths[i].separateElements();
                paths[i].checkForTxGates();
                if( paths[i].getTxGateNode() != NULL ) {
                    parent->insertTxGateNodes( paths[i].getTxGateNode() );
                }
                paths[i].generateDio( params, modelPrefix, output, switchNode );
            }
        }
    }

    string completedDio = ( paths.size() > 1 ) ? "max( " : "";

    for( int i = 0; i < paths.size(); i++ ) {
        completedDio += paths[i].getDio();
        if( paths.size() > 1 && i != paths.size() - 1 ) {
            completedDio += ", ";
        }
    }
    completedDio += ( paths.size() > 1 ) ? " )" : "";
    if( completedDio != "" && completedDio.find( modelNotFound ) == string::npos ) {

        if      ( modelPrefix == fallToRiseNor ) {
            fallToRiseDioNor = completedDio;
        }
        else if ( modelPrefix == fallToRiseStd ) {
            fallToRiseDioStd = completedDio;
        }
        else if ( modelPrefix == riseToFallNor ) {
            riseToFallDioNor = completedDio;
        }
        else if ( modelPrefix == riseToFallStd ) {
            riseToFallDioStd = completedDio;
        }
        else {
            cerr << "unknown dio type " << modelPrefix << endl;
        }
    }
}

void Input::generateDio( SymbolTable params, string modelPrefix, Node * output, Subcircuit * parent, unsigned no ) {

    const bool pfxIsFTRNor = modelPrefix == fallToRiseNor;
    const bool pfxIsFTRStd = modelPrefix == fallToRiseStd;

    // prefix is fall-to-rise
    const bool pfxIsFTR = pfxIsFTRNor || pfxIsFTRStd;

    vector<Path> paths = pfxIsFTR ? pathsToVddVec[no] : pathsToGndVec[no];

    for( int j = 0; j < connections.size(); j++ ) {

        Node * switchNode = connections[j];

        for( int i = 0; i < paths.size(); i++ ) {
            if( paths[i].contains( switchNode ) ) {

                paths[i].separateElements();
                paths[i].checkForTxGates();

                if( paths[i].getTxGateNode() != NULL ) {
                    parent->insertTxGateNodes( paths[i].getTxGateNode() );
                }
                paths[i].generateDio( params, modelPrefix, output, switchNode );
            }
        }
    }

    string completedDio = ( paths.size() > 1 ) ? "max( " : "";
    for( int i = 0; i < paths.size(); i++ ) {
        completedDio += paths[i].getDio();
        if( paths.size() > 1 && i != paths.size() - 1 ) {
            completedDio += ", ";
        }
    }

    completedDio += ( paths.size() > 1 ) ? " )" : "";
    if( completedDio != "" && completedDio.find( modelNotFound ) == string::npos ) {
        if      ( modelPrefix == fallToRiseNor ) {
            fallToRiseDioNorVec[no] = completedDio;
        }
        else if ( modelPrefix == fallToRiseStd ) {
            fallToRiseDioStdVec[no] = completedDio;
        }
        else if ( modelPrefix == riseToFallNor ) {
            riseToFallDioNorVec[no] = completedDio;
        }
        else if ( modelPrefix == riseToFallStd ) {
            riseToFallDioStdVec[no] = completedDio;
        }
        else {
            cerr << "unknown dio type " << modelPrefix << endl;
        }
    }
}

// reverses front as well
vector<Path> Input::combinePaths( vector<Path> front, vector<Path> end ) {

    vector<Path> combinedPaths;

    for( int i = 0; i < front.size(); i++ ) {
        for( int j = 0; j < end.size(); j++ ) {

            Path combined( front[i].rbegin(), front[i].rend() - 1 );

            if( !containLoops( combined, end[j] ) ) {
                combined.append( end[j] );
                combinedPaths.push_back( combined );
            }
        }
    }
    return combinedPaths;
}

bool Input::containLoops( Path path1, Path path2 ) {

    Path::iterator it;

    for( it = path2.begin(); it != path2.end(); it++ ) {

        if( path1.contains( *it ) ) {
            return true;
        }
    }

    return false;
}

