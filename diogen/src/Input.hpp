#ifndef _H_Input
#define _H_Input

#include "Internal.hpp"

class Subcircuit;

class Input : public Node {

private:
   //vector<Path> nmosPaths;
   //vector<Path> pmosPaths;
   vector<Path> pathsToGnd;
   vector<Path> pathsToVdd;
   vector<vector<Path> > pathsToGndVec;
   vector<vector<Path> > pathsToVddVec;
   string riseToFallDioStd;
   string riseToFallDioNor;
   string fallToRiseDioStd;
   string fallToRiseDioNor;
   vector<string> riseToFallDioStdVec;
   vector<string> riseToFallDioNorVec;
   vector<string> fallToRiseDioStdVec;
   vector<string> fallToRiseDioNorVec;


   unsigned numOutputs; //to record the number of outputs in the subckt to which this input belongs.
   vector<Path> combinePaths( vector<Path> a, vector<Path> b );
   bool containLoops( Path path1, Path path2 );

public:
   Input( Node * intNode, unsigned no );

   void findPaths( Subcircuit * parent, unsigned no );
   void generateDio( SymbolTable params, string modelPrefix, Node * output, Subcircuit * parent );
   void generateDio( SymbolTable params, string modelPrefix, Node * output, Subcircuit * parent, unsigned no );
   string getRiseToFallDioStd() {
      return riseToFallDioStd;
   }

   string getRiseToFallDioNor() {
      return riseToFallDioNor;
   }

   string getFallToRiseDioStd() {
      return fallToRiseDioStd;
   }

   string getFallToRiseDioNor() {
      return fallToRiseDioNor;
   }

   string getRiseToFallDioStd( unsigned no ) {
      return riseToFallDioStdVec[no];
   }

   string getRiseToFallDioNor( unsigned no ) {
      return riseToFallDioNorVec[no];
   }

   string getFallToRiseDioStd( unsigned no ) {
      return fallToRiseDioStdVec[no];
   }

   string getFallToRiseDioNor( unsigned no ) {
      return fallToRiseDioNorVec[no];
   }

};

#endif
