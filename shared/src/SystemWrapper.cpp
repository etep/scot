#include <stdlib.h>
#include <assert.h>
#include <iostream>
#include <SystemWrapper.hpp>

using namespace std;

void SystemWrapper( const std::string & cmd, const bool verbose, const bool trial ) {

    if( verbose ) {
        cout << "... system( " << cmd << " );" << endl;
    }
    if( !trial ) {

        int status = system( cmd.c_str() );
        assert( status == 0 );
    }
    else {
        cout << "... trial/dry run" << endl;
    }
    if( verbose ) {
        cout << "... finished: " << cmd << endl;
    }
}

