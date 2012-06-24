#ifndef _SH_SYMTAB_H_
#define _SH_SYMTAB_H_

#include <map>
#include <vector>
#include <string>
#include <assert.h>
#include <iostream.h>
extern	std::map<std::string, int> glbVars;

class symbol_table;
ostream & operator<< ( ostream &, const symbol_table & );

class symbol_table {
public:
   static const unsigned MAX_NUMBER_LENGTH = 20;

private:
   static const unsigned INTER_SYMBOL_MARGIN = 5;
   static const unsigned MAX_SYMBOL_LENGTH = 50;
   static const unsigned MAX_VARIABLE_LENGTH = 20;

   static const std::string AUX_SYMBOL_NAME_PREFIX;
   static const std::string AUX_LABEL_NAME_PREFIX;

   std::map<std::string,int> mapSymbol;
   std::vector<std::string> vecString;
   std::vector<int> vecNumRef;

   int numAuxSymbols;
   int numAuxLabels;

   unsigned maxSymbolLength;
   unsigned currentIndex;
   void updateMaxSymbolLength();

   //symbol_table( const symbol_table & );
   //symbol_table & operator=( const symbol_table & );

   unsigned putWithoutLookUp( const std::string & a );

public:
   // constructor
   symbol_table() {
      numAuxSymbols = numAuxLabels = maxSymbolLength = currentIndex = 0;
   }

   // public methods
   void clear() {
      mapSymbol.clear();
      vecString.clear();
      vecNumRef.clear();
      glbVars.clear();
      numAuxSymbols = numAuxLabels = maxSymbolLength = currentIndex = 0;
   }
   unsigned put( const std::string & a );
   int find( const std::string & a ) const;
   int index( const std::string & a ) const {
      return find( a );
   }
   int makeAndGetIndexOfAuxSymbol();
   int makeAndGetIndexOfAuxLabel();

   // getter
   const std::string & getSymbol( int ind ) const {
      assert( ind<vecString.size() );
      return vecString[ind];
   }
   const std::string getSymbol( const std::string &, int, const std::string &, const std::string & ) const;
   const std::string & operator[] ( int ind ) const {
      return getSymbol( ind );
   }
   unsigned getNumberOfSymbols() const {
      return mapSymbol.size();
   }
   unsigned size() const {
      return getNumberOfSymbols();
   }

   int getNumberOfAuxSymbols() const {
      return numAuxSymbols;
   }
   unsigned getMaxSymbolLength() {
      updateMaxSymbolLength();
      return maxSymbolLength;
   }
   unsigned getMaxSymbolLength() const;
   std::map<std::string,int>::const_iterator getConstMapBegin() const {
      return mapSymbol.begin();
   }
   std::map<std::string,int>::const_iterator getConstMapEnd() const {
      return mapSymbol.end();
   }

   // print/stream
   void print() const {
      print( "x" );
   }
   void print( const std::string & x ) const {
      toOstream( cout,x );
   }

   ostream & toOstream( ostream & os ) const {
      return toOstream( os,"x" );
   }
   ostream & toOstream( ostream &, const std::string & x ) const;

   ostream & matlabSymbolNamesToOstream( ostream & os ) const;
};

#endif
