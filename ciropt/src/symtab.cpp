#include <string>
#include "symtab.hpp"
#include "cnvt.hpp"

const std::string symbol_table::AUX_SYMBOL_NAME_PREFIX( "T#" );
const std::string symbol_table::AUX_LABEL_NAME_PREFIX ( "~L" );

ostream & operator << ( ostream & os, const symbol_table & st ) {
   return st.toOstream( os );
}


void symbol_table::updateMaxSymbolLength() {
   if( currentIndex == vecString.size() ) {
      return;
   }

   for( int i = currentIndex; i < vecString.size(); i ++ ) {
      if( vecString[i].length() > maxSymbolLength ) {
         maxSymbolLength = vecString[i].length();
      }
   }

   currentIndex = vecString.size();
}

unsigned symbol_table::putWithoutLookUp( const std::string & a ) {
   mapSymbol[a] = vecString.size();
   vecString.push_back( a );
   vecNumRef.push_back( 1 );

   return vecString.size()-1;
}


unsigned symbol_table::put( const std::string & a ) {
   int ind = find( a );

   if( ind == -1 ) {
      return putWithoutLookUp( a );
   }
   vecNumRef[ind] ++;
   return ind;
}

int symbol_table::find( const std::string & a ) const {
   
   std::map<std::string,int>::const_iterator itr = mapSymbol.find( a );

   if( itr == mapSymbol.end() ) return -1;

   return itr->second;
}

int symbol_table::makeAndGetIndexOfAuxSymbol() {
   numAuxSymbols ++;

   std::string aux_name
      = AUX_SYMBOL_NAME_PREFIX + cnvt::intToString( numAuxSymbols );

   assert( find( aux_name ) == -1 );

   putWithoutLookUp( aux_name );

   return ( vecString.size()-1 );
}

int symbol_table::makeAndGetIndexOfAuxLabel() {

   numAuxLabels ++;

   std::string aux_name = AUX_LABEL_NAME_PREFIX + cnvt::intToString( numAuxLabels );

   assert( find( aux_name ) == -1 );

   putWithoutLookUp( aux_name );

   return ( vecString.size()-1 );
}

const std::string symbol_table::getSymbol( const std::string & nodename, int ind, const std::string & searchS, const std::string & replaceS ) const {
   // cout << "string " << searchS << " string " <<  replaceS << " node " << nodename << " " << ind << endl;
   assert( ind < vecString.size() );

   // std::string varname = vecString[ind];
   //
   // just vdd in ggpsolexp (why?)
   // if( nodename == "" ||  vecString[ind] == "Vdd" ||  vecString[ind] == "vdd" ) {
   if( nodename == "" ||  glbVars.find( vecString[ind] ) != glbVars.end() ) {
      return vecString[ind];
   }

   if( vecString[ind] == searchS ) return replaceS;
   else return nodename + "." + vecString[ind];
}

unsigned symbol_table::getMaxSymbolLength() const {

   unsigned ml = 0;

   for( int i = 0; i < vecString.size(); i++ ) {

      if( vecString[i].length() > ml ) {
         ml = vecString[i].length();
      }
   }


   return ml;
}

ostream & symbol_table::toOstream( ostream & os, const std::string & x ) const {
   unsigned maxSL = cnvt::max( getMaxSymbolLength(),10 );

   os << cnvt::addSpaces( "Symbols",maxSL+INTER_SYMBOL_MARGIN )
      << cnvt::addSpaces( "Variables",MAX_VARIABLE_LENGTH )
      << cnvt::addSpaces( "Variables",MAX_VARIABLE_LENGTH+INTER_SYMBOL_MARGIN )
      << cnvt::addSpaces( "Symbols",maxSL ) << endl;
   os << std::string( 2*( maxSL+INTER_SYMBOL_MARGIN ),'-' )
      << std::string( 2*( MAX_VARIABLE_LENGTH ),'-' ) << endl;

   std::map<std::string,int>::const_iterator itr;
   int i;

   for( i=0, itr = mapSymbol.begin(); itr != mapSymbol.end(); i++, itr ++ ) {
      os << cnvt::addSpaces( itr->first,maxSL );
      os << " <-> ";
      os << cnvt::addSpaces( x+cnvt::intToString( itr->second+1 ),MAX_VARIABLE_LENGTH );
      os << cnvt::addSpaces( x+cnvt::intToString( i+1 ),MAX_VARIABLE_LENGTH );
      os << " <-> ";
      os << cnvt::addSpaces( ( *this )[i],maxSL ) << endl;
   }

   return os;
}

ostream & symbol_table::matlabSymbolNamesToOstream( ostream & os ) const {
   for( int i = 0; i < getNumberOfSymbols(); i++ ) {
      
      os << "vn(" << ( i+1 ) << ").name = '" << getSymbol( i ) << "';" << endl;
   }

   return os;
}
