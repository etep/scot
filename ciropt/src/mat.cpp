#include "mat.hpp"

ostream & Mat::arrayToOstream( ostream & os, const double * a, unsigned n, const std::string & name ) {
   
   os << name << "=[";
   for( unsigned i = 0; i < n; i ++ ) {
      os << a[i] << ";";
      if( ( i+1 )/8*8 == i+1 ) os << endl;
   }
   os << "];" << endl;

   return os;
}

ostream & Mat::vecToOstream( ostream & os, const std::vector<double> & dv, const std::string & name ) {
   os << name << "=[";
   for( unsigned i = 0; i < dv.size(); i ++ ) {
      os << dv[i] << ";";
      if( ( i+1 )/8*8 == i+1 ) os << endl;
   }
   os << "];" << endl;

   return os;
}
