// ciropt.cpp : generalized GP Parser
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include <string>
#include <fstream>
#include <iostream>
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
#include "network.hpp"
#include "opt.hpp"
#include "ggp.hpp"
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
using namespace std;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
extern FILE * ciroptin;
extern FILE * dioin;
extern map<string, int> glbVars;
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int ciroptparse();
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
string dio_filename;
string ciropt_filename;

network * nw;
opt_prob_generator * op;

string ERROR_STRING = " ERROR ";
string EMPTY_STRING = "";
string KEEPER_INPUT = "inK";
string DUM_KEEPER_INPUT = KEEPER_INPUT+"S";
double DEFAULT_QUANTILE = 0.9;
double MAX_PATH_LENGTH = 100.0;

// excerpted from ggp.cpp for linking purpose
const unsigned ggp::GP_TRANSFORM_NUMBER = 0;
const string ggp::OBJECTIVE_LABEL = " obj";
const string ggp::AUX_INEQ_LABEL_CONJ = "-";
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int errorReport( const string & em ) {

    cerr << em << endl;

    exit( -1 );
    return -1;
}
void errorBasedHalt() {
    return;
}
bool errorExist() {
    return false;
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char ** argv ) {

    //fprintf(stderr, "**Stanford Optimization Project**\n");
    if( argc != 3 ) {
        cerr << "usage: ciropt [sp filename] [dio filename]" << endl;
        return -2;
    }

    const string name        = argv[1];
    const string spFileName  = argv[1];
    const string dioFileName = argv[2];

    // check whether the number of input arguments of 'ciropt' is one.
    // ciroptin is a (file) pointer pointing to the .sp file
    // dioin is a (file) pointer pointing to the .dio file
    ciroptin = fopen( spFileName.c_str(),  "r" );
    dioin    = fopen( dioFileName.c_str(), "r" );

    if( ciroptin == NULL ) {
        errorReport( "file open error: " + spFileName );
    }
    if( dioin    == NULL ) {
        errorReport( "file open error: " + dioFileName );
    }

    errorBasedHalt();

    // create an instance of 'op' class and pass a reference to
    // a newly generated network.
    // note that 'nw' is the global pointer variable (defined before main function in this file)
    // which will be accessed in "ciropt.ypp",
    // where the circuit information in the .sp file
    // is processed and passed to the network instanced pointed by 'nw'.
    nw = new network();
    op = new opt_prob_generator( name, * nw );

    // execute the code in ciropt.tab.cpp, (one can regard this as just
    // "ciropt.ypp" since it is generated by 'bison' from "ciropt.ypp");
    // reads circuit information and optimization constraints, etc.
    // from the .sp file and store them into the network instance
    // pointed by 'nw'.
    dio_filename = dioFileName;
    ciropt_filename = spFileName;
    cout << "Recorded the file names" << endl;
    ciroptparse();
    cout << "Parsing finished" << endl;

    // close each file
    fclose( ciroptin );
    fclose( dioin );
    errorBasedHalt();

    // solve a GGP
    op->checkLegitimacyOfReferencesToOptimizations();
    op->assignTheNetList();

    errorBasedHalt();

    op->solve();
    return 0;
    errorBasedHalt();

    op->write();
    errorBasedHalt();

    op->draw();
    errorBasedHalt();

    delete op;
    delete nw;
    return 0;
}

