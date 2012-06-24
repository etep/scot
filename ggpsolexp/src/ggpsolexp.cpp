// ggp_parser.cpp : generalized GP Parser

#include <string>
#include <stdio.h>
#include <iostream.h>
#include "ggpsolexp.hpp"
#include "ggp.hpp"
#include "cnvt.hpp"

extern FILE * ggpin;
extern int line_number;
extern symbol_table symtab, labSymTab;
extern std::string filename;

ggp * GGP;

int ggpparse( void );

int main( int argc, char* argv[] ) {
   
   argAnalyzer aa( argc,argv );

   const std::string executableName = argv[0];
   const std::string executablePrompt = executableName + ": ";

   if( aa.hasAnyErrorOccurred() ) {
      cerr << executablePrompt << aa.getErrorMessage() << endl;
      cerr << aa.getHelpMessage() << endl;
      return -1;
   }

   if( aa.isHelpOn() ) {
      cout << aa.getHelpMessage() << endl;
      return 0;
   }

   const std::vector<std::string> & vecString = aa.getInputFileNames();

   unsigned numberOfProcessedFiles = 0;

   for( int i = 0; i < aa.getNumberOfInputFiles(); i ++ ) {
      filename = vecString[i];

      ggpin = fopen( filename.c_str(), "r" );
      
      if( ggpin == ( FILE * )NULL ) {

         cerr << executablePrompt << "file open error: " << filename.c_str() << endl;
         continue;
      }

      ggpparse();
      
      ggp * GP = &GGP->toGP();

      if( aa.isGGPToStandardOutOn() || aa.isGPToStandardOutOn() ) {
         cout << "[" << filename << ":]" << endl;
      }

      if( aa.isGGPToStandardOutOn() ) {
         GGP->print( 1 );
      }
      if( aa.isGPToStandardOutOn() ) {
         GP->print( 2 );
      }

      if( aa.isGGPToFileOn() ) {
         GGP->toFile( cnvt::changeExtension( filename, "ggp" ) );
      }
      if( aa.isGPToFileOn() ) {
         GP->toFile( cnvt::changeExtension( filename, "gp" ) );
      }
      
      if( aa.isSolveDualGPOn() ) {
         GP->solveUsingMOSEKdgopt( filename, *GGP );
      }

      /*
      if( aa.isMatlabOn() )
      {
        if( !GP->toMatlabFile( cnvt::changeExtension(filename,"m") ) )
          cerr << "there are equality constraints." << endl;
      }
      */

      /*
      if( aa.isMakeMatlabFileOn() ) GGP->makeMatlabFile( filename );
      */

      fclose( ggpin );
      delete GGP;
      delete GP;

      line_number = 1;
      symtab.clear();
      labSymTab.clear();

      numberOfProcessedFiles++;
   }

   if( numberOfProcessedFiles == 0 ) {
      cerr << executablePrompt << "no valid files" << endl;
      return -1;
   }
}

// static member initialization
const int argAnalyzer::defaultFlag = 1;

const std::string argAnalyzer::flags = "hpcfgd";

const std::string argAnalyzer::helpMessage =
   " -h  show the usage of ggpsol.\n"
   " -p  print the generalized GP (GGP) to the standard output. (default flag)\n"
   " -c  print the equivalent GP (EGP) to the standard output.\n"
   " -f  write the GGP in .ggp file.\n"
   " -g  write the EGP in .gp file.\n"
   " -d  solve the problem using MOSEK dgopt and report the result to .out file.\n"
   //" -m  write CGP in matlab file with extension .m\n"
   /*
   " -a  write the problem in (A,c,map) form and add routine to solve\n"
   "     the problem and report the result in .m file with the same name as\n"
   "     input file.\n"
   "\n"
   */
   "\nplease report bugs to <sunghee.yun@stanford.edu>\n";

// class member definitions
// private members
argAnalyzer::argAnalyzer( int argc, char * argv [] ) : executableName( argv[0] ), executablePrompt( executableName + ": " ), isOn( std::vector<bool> ( flags.length() ) ) {
   
   isThereError = false;

   for( int i = 0; i < flags.length(); i ++ ) {
      isOn[i] = false;
   }

   for( int i = 1; i < argc; i ++ ) {
      if( isFlag( argv[i] ) ) {
         analyzeFlag( argv[i] );
      }
      else {
         vecInputFileNames.push_back( argv[i] );
      }
   }

   if( vecInputFileNames.size() == 0 ) {
      isThereError = true;
      errorMessage = "no files";
   }

   for( int i = 0; i < flags.length(); i++ ) {
      if( isOn[i] ) {
         return;
      }
   }

   isOn[argAnalyzer::defaultFlag] = true;
}

void argAnalyzer::analyzeFlag( const std::string & s ) {
   
   if( s.length() <= 1 ) {
      isThereError = true;
      errorMessage = "no input argument";
      return;
   }

   for( int i = 1; i < s.length(); i ++ ) {
      bool change = false;
      for( int j = 0; j < flags.length(); j ++ ) {

         if( s[i] == flags[j] ) {
            isOn[j] = true;
            change = true;
         }
      }

      if( !change ) {
         isThereError = true;
         errorMessage = "illegal option";
      }
   }
}




