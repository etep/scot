#include <string.h>
#include <string>
#include "utility.hpp"
#include "scanner.h"
#include "parser.h"
#include "CircuitParser.hpp"

#include <ostream>
#include <fstream>

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_NUMBER_LENGTH 10
//#define DEBUG

extern FILE * yyin; // parse input file

CircuitParser * parser;
extern CircuitParser * parser;

void runDio   ( const char * modifiedSpiceFilename, const char * dioDataFilename, const char * outputDioFilename, bool printToFile );
void runPower ( const char * originalSpiceFilename, const char * modifiedSpiceFilename, const char * irsimTechFilename, int numRuns, const char * irsimCmdFilename, double defActFact, int maxSignalsOnALineInIrsim );

void displayHelp();
static inline const std::string intToString( int d );
FILE * openFile( const char * name, const char * flag );


int main( int argc, char ** argv ) {

   bool generateDio = false;
   bool generatePower = false;

   const char * originalSpiceFilename = "";
   const char * modifiedSpiceFilename = "";
   //char * modifiedSpiceForPowerFilename = "";
   const char * irsimTechFilename = "";
   const char * dioDataFilename = "";
   const char * outputDioFilename = "";
   const char * irsimCmdFilename = "";

   int numRuns = 100;
   double defActFact = 0.25;
   int maxSignalsOnALineInIrsim = 64;

   parser = new CircuitParser();

   // parse dio base file
   #ifdef DEBUG


   originalSpiceFilename = "test_adder_32.sp";
   modifiedSpiceFilename = "adder32.sp";
   //  modifiedSpiceForPowerFilename = "test_adder_32.sp";
   irsimTechFilename = "scmos30.prm";
   dioDataFilename = "base_dio_var.txt";
   outputDioFilename = "adder32.dio";
   irsimCmdFilename = "test.cmd";

   /*
     originalSpiceFilename = "inv_chain4.spi";
     modifiedSpiceFilename = "inv_chain4.sp";
     modifiedSpiceForPowerFilename = "inv_chain4.sp";
     irsimTechFilename = "scmos30.prm";
     dioDataFilename = "base_dio_var.txt";
     outputDioFilename = "inv_chain4.dio";
    */
   generateDio = true;
   generatePower = true;

   #else

   if( argc < 2 ) {
      displayHelp();
   }

   if( strchr( argv[1], 'd' ) ) {
      generateDio = true;
   }
   if( strchr( argv[1], 'p' ) ) {
      generatePower = true;
   }

   // the possible options for diogen are: generate just dio, or generate
   // both dio and power
   if( !generateDio && !generatePower ) {
      displayHelp();
   }

   if( generateDio && !generatePower ) {
      if( argc != 5 ) {
         displayHelp();
      }
      modifiedSpiceFilename = argv[2];
      dioDataFilename = argv[3];
      outputDioFilename = argv[4];

   }
   else if ( !generateDio && generatePower ) {
      
      if( argc != 6 && argc != 7 ) {
         displayHelp();
      }
      
      originalSpiceFilename = argv[2];
      modifiedSpiceFilename = argv[3];
      irsimTechFilename = argv[4];
      numRuns = atoi( argv[5] );

      if( argc == 7 ) {
         irsimCmdFilename = argv[6];
      }
   }
   else if( generateDio && generatePower ) {

      if( argc != 8 && argc != 9 ) {
         displayHelp();
      }
      
      originalSpiceFilename = argv[2];
      modifiedSpiceFilename = argv[3];
      irsimTechFilename = argv[4];
      numRuns = atoi( argv[5] );

      if( argc == 9 ) {
         irsimCmdFilename = argv[8];
      }
   }
   else {
      displayHelp();
   }

   #endif

   srand( time( 0 ) );
   InitScanner();
   InitParser();


   // parse spice input file
   yyin = openFile( modifiedSpiceFilename, "r" );
   yyparse();
   fclose( yyin );

   if( generateDio ) {
      // parse dio data file
      yyin = openFile( dioDataFilename, "r" ); //fopen(dioDataFilename, "r");
      yyparse();
      fclose( yyin );
      runDio( modifiedSpiceFilename, dioDataFilename, outputDioFilename, generateDio );
      ofstream dioFile( outputDioFilename, ios_base::app );
      parser->generatePowerExpressionsInDioFile( dioFile );
      parser->generateLeakageExpressionsInDioFile( dioFile );
      dioFile.close();
   }

   if( generatePower ) {
      
      runPower( originalSpiceFilename, modifiedSpiceFilename, irsimTechFilename, numRuns, irsimCmdFilename, defActFact,maxSignalsOnALineInIrsim );
   }
   return 0;
}

void runDio( const char * modifiedSpiceFilename, const char * dioDataFilename, const char * outputDioFilename, bool generateDio ) {

   ofstream dioFile( outputDioFilename );
   string modifiedSpiceFilenameStr( modifiedSpiceFilename );
   modifiedSpiceFilenameStr += ".txgate";
   ofstream txgateFile( modifiedSpiceFilenameStr.c_str() );

   if( generateDio && ( !dioFile || !txgateFile ) ) {
      cerr << "cannot open output file " << outputDioFilename << " or ";
      cerr << modifiedSpiceFilename<< endl;
      exit( 1 );
   }

   parser->generateDio( dioFile, generateDio );
   parser->generateTxGateConstraints( txgateFile );

   dioFile.close();
   txgateFile.close();

   return;
}

void runPower( const char * originalSpiceFilename, const char * modifiedSpiceFilename, const char * irsimTechFilename, int numRuns, const char * irsimInputCmdFilename, double defActFact, int maxSignalsOnALineInIrsim ) {
   
   const char * scotHomePtr = getenv( "SCOT_HOME_DIR" );
   const char * tempHomePtr = getenv( "SCOT_TEMP_DIR_OPTIM_DOT_PY" );
   const string scotHome    = string( scotHomePtr );
   const string tempHome    = string( tempHomePtr );
   const string perlHome    = scotHome + "/lrep";
   const string pyHome      = scotHome + "/pys";
   const string irsmHome    = scotHome + "/irsim/bin";
   
   const string modifiedSpiceFilenameStr = string( modifiedSpiceFilename );
   const string originalSpiceFilenameStr = string( originalSpiceFilename );
   const string irsimTechFilenameStr     = string( irsimTechFilename );

   const bool cmdFileSpecified = string( irsimInputCmdFilename ) != "";
   
   const string irsimCmdFilename = cmdFileSpecified ? string( irsimInputCmdFilename ) : tempHome + "/" + originalSpiceFilenameStr + ".cmd";

   ofstream irsimCmdFile;
   if ( string( irsimInputCmdFilename ) == "" ) {
      irsimCmdFile.open( irsimCmdFilename.c_str() );
      if ( !irsimCmdFile ) {
         cerr << "cannot open output file " << irsimCmdFilename << endl;
         exit( 1 );
      }

      // generate IRSIM commadn file
      parser->generateIrsimInput( irsimCmdFile, originalSpiceFilenameStr, numRuns, defActFact );
   }

   const string irsimInputFileName = tempHome + "/" + originalSpiceFilenameStr + ".irsim";
   const string irsimOutput        = tempHome + "/" + originalSpiceFilenameStr + ".out";
   const string powerFilenameStr   = modifiedSpiceFilenameStr + ".power";
   const string dutyFilenameStr    = modifiedSpiceFilenameStr + ".duty";
      
   const string adjustIRSIMcmd = scotHome + "/lrep/limVectForDiogen.pl " + irsimCmdFilename +  " " + intToString( maxSignalsOnALineInIrsim );
   // -- TODO -- remove -- shifted impl to python -- const string flattenerCmd   = perlHome + "/fasths2flat.pl "           + originalSpiceFilenameStr + " > " + irsimInputFileName;
   const string flattenerCmd   = pyHome   + "/fasths2flat.py -i " + originalSpiceFilenameStr + " -o " + irsimInputFileName;
   const string irsimCmd       = irsmHome + "/irsim "                    + irsimTechFilenameStr + " " + irsimInputFileName + " " + "-" + irsimCmdFilename;
   const string perlCmd        = perlHome + "/cal_act_fact.pl "          + irsimOutput + " " +  irsimInputFileName + " " + powerFilenameStr + " " + dutyFilenameStr + " " +  intToString( numRuns );
   
   int sysresult;
   
   sysresult = system( adjustIRSIMcmd.c_str() );
   assert( sysresult == 0 );
   
   cout << "diogen: flattening netlist for irsim" << endl;
   cout << "..." << flattenerCmd << endl;
   sysresult = system( flattenerCmd.c_str() );
   assert( sysresult == 0 );
   
   
   // if (irsimInputCmdFilename == "") {
   //    irsimCmd += originalSpiceFilenameStr + ".cmd";
   // }
   // else irsimCmd += irsimInputCmdFilename;
   
   // run irsim
   // need to chdir so that the irsim .out file lands in the correct place
   assert( chdir( tempHome.c_str() ) == 0 );
   cout << "diogen: running irsim" << endl;
   cout << "..." << flattenerCmd << endl;
   cout << irsimCmd << endl;
   system( irsimCmd.c_str() );
   
   // will use a perl file to record the switching activity factors.
   // yyin = openFile(irsimOutput.c_str(), "r");

   // yyparse();

   // Dinesh wants the power outputs to another file and he will write a
   // perl script that combines that with the modifiled spice file
   //
   // ofstream spiceFile(modifiedSpiceFilename, ios_base::app);

   ofstream powerFile( powerFilenameStr.c_str() );
   // ofstream dioFile(outputDioFilename, ios_base::app);
   // parser->generateActivityFactors(powerFile);
   // cout << "number of iterations  are " << intToString(numRuns) << endl;
   system( perlCmd.c_str() );

   powerFile.close();
}

static inline const string intToString( int d ) {
   if( d == 0 ) return std::string( "0" );

   char * const s = ( char * )malloc( MAX_NUMBER_LENGTH );
   snprintf( s, MAX_NUMBER_LENGTH, "%d", d );

   return std::string( s );
}


FILE * openFile( const char * name, const char * flag ) {
   FILE * f = fopen( name, flag );
   if ( !f ) {
      cerr << "Cannot open file " << name << endl;
      exit( 1 );
   }
   return f;
}

void displayHelp() {

   cerr << "Usage: diogen <options> <filenames>" << endl;
   cerr << "where the options are:\n" << endl;

   cerr << "    -d     generate delay and power models " << endl;
   cerr << "and <filenames> should be ";
   cerr << "<modified spice file> <dio data file> <output dio file>\n" << endl;


   cerr << "    -p     generate activity/duty factors " << endl;
   cerr << "<filenames> should be ";
   cerr << "<original spice file (with correct sizes)> <modified spice file> <irsim tech file> ";
   cerr << "<number of irsim runs>  [optional]<.cmd file for IRSIM runs> "                 << endl;
   cerr << "If the .cmd file is not given, a random input file will be generated"          << endl;
   cerr << "If the .cmd file is given the user must input correct value of number of runs" << endl;
   cerr << "The activity(duty) factors are recored in <modified spice file>.power(duty) and also included in";
   cerr << "<modified spice file> itself";
   cerr << endl << endl;


   cerr << "    -dp    generate both delay/power models and activity/duty factors " << endl;
   cerr << "<filenames> should be ";
   cerr << "<original spice file (with correct sizes)> <modified spice file> <irsim tech file> ";
   cerr << "<dio data file> <output dio file> <number of irsim runs>  [optional]<.cmd file for IRSIM runs> " << endl;
   cerr << "If the .cmd file is not given, a random input file will be generated" << endl;

   exit( 1 );
}
