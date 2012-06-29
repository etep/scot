#include "cnvt.hpp"
using namespace std;

vector< string > cnvt::colors, cnvt::styles, cnvt::mrkers;

bool cnvt::initDone = false;

void cnvt::initializeColorsStyles() {

   if( initDone ) {
      return;
   }

   colors.push_back( "b" );
   colors.push_back( "r" );
   colors.push_back( "k" );
   colors.push_back( "m" );
   colors.push_back( "y" );
   colors.push_back( "c" );
   colors.push_back( "g" );

   styles.push_back( "-" );
   styles.push_back( "--" );
   styles.push_back( "-." );
   styles.push_back( ":" );

   mrkers.push_back( "+" );
   mrkers.push_back( "o" );
   mrkers.push_back( "*" );
   mrkers.push_back( "." );
   mrkers.push_back( "x" );
   mrkers.push_back( "s" );
   mrkers.push_back( "d" );
   mrkers.push_back( "^" );
   mrkers.push_back( "v" );
   mrkers.push_back( ">" );
   mrkers.push_back( "<" );
   mrkers.push_back( "p" );
   mrkers.push_back( "h" );

   initDone = true;

   return;
}

vector< double > cnvt::mapToDoubleVector( const map<string,double> & doubleMap ) {
   vector<double> vec;

   map<string,double>::const_iterator it;

   for( it = doubleMap.begin(); it != doubleMap.end(); it++ ) {
      vec.push_back( it->second );
   }

   assert( vec.size() == doubleMap.size() );

   return vec;
}

ostream & cnvt::matlabPlotToOstream( ostream & os, const vector<string> & xnames, const vector<string> & ynames, bool marker ) {
   assert( xnames.size() == ynames.size() );
   initializeColorsStyles();

   os << "plot( ";

   for( vector<string>::size_type i = 0; i < xnames.size(); i++ ) {
      size_t colorNum = i - ( ( size_t )( i/colors.size() ) )*colors.size();
      size_t styleNum = i - ( ( size_t )( i/styles.size() ) )*styles.size();
      size_t mrkerNum = i - ( ( size_t )( i/mrkers.size() ) )*mrkers.size();

      os << xnames[i] << ", " << ynames[i] << ", '";
      os << colors[colorNum] << styles[styleNum];
      if( marker ) os << mrkers[mrkerNum];

      if( i != xnames.size() - 1 ) os << "', ..." << endl;
      else os << "');" << endl << endl;
   }

   return os;
}

ostream & cnvt::matlabLegendToOstream( ostream & os, const vector<string> & vec ) {

   os << "legend( ";

   for( vector<string>::size_type i = 0; i < vec.size(); i ++ ) {
      if( i != 0 ) os << ", ";
      os << "'" << vec[i] << "'";
   }

   os << " );" << endl;

   return os;
}

double cnvt::interpolate( double xx, double x1, double y1, double x2, double y2 ) {

   assert( xx > x1 );
   assert( x2 > xx );

   double ic = ( x2-x1 );
   double w1 = ( x2-xx )/ic;
   double w2 = ( xx-x1 )/ic;

   return ( w1 * y1 + w2 * y2 );
}

vector<double> cnvt::linSpaceToVec( double s, double e, size_t num ) {

   assert( e > s );

   vector<double> vec;
   double inc = ( e-s )/( double )num;

   for( size_t i = 0; i < num; i ++ ) vec.push_back( s + inc * ( double )i );

   return vec;
}

double cnvt::getMean( const vector<double> & vec ) {

   double s = 0.0;
   vector<double>::size_type num = vec.size();

   for( vector<double>::size_type i = 0; i < num; i++ ) {
      s += vec[i];
   }

   return s/( double )num;
}

double cnvt::getSTD( const  vector<double> & vec, double mean ) {
   double s = 0.0;
   vector<double>::size_type num = vec.size();

   for( vector<double>::size_type i = 0; i < num; i ++ ) {
      s += vec[i]*vec[i];
   }

   return sqrt( s/( double )num-mean*mean );
}
