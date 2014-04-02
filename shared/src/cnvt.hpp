#ifndef _SH_CNVT_H_
#define _SH_CNVT_H_

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include <map>
#include <vector>
#include <string>
#include <ostream>

// -- TODO -- remove -- not necessary -- static  size_t MAX_NUMBER_LENGTH = 20;
// -- TODO -- remove -- not necessary -- static  size_t MAX_STRING_LENGTH = 20;
// -- TODO -- remove -- not necessary -- static  unsigned PRECISION_NUMBER = 7;


class cnvt {
private:
    static const size_t MAX_NUMBER_LENGTH = 20;
    static const size_t MAX_STRING_LENGTH = 20;
    static const unsigned PRECISION_NUMBER = 7;

    static bool initDone;
    static std::vector<std::string> colors, styles, mrkers;

    static void initializeColorsStyles();

public:

    static inline int max( int a, int b ) {
        return ( a>b )?a:b;
    }
    static inline int min( int a, int b ) {
        return ( a<b )?a:b;
    }

    static inline double max( double a, double b ) {
        return ( ( a>b )?a:b );
    }
    static inline double min( double a, double b ) {
        return ( ( a<b )?a:b );
    }

    static inline std::string toString( int i ) {
        return intToString( i );
    }

    static inline std::string toString( unsigned u ) {
        return intToString( ( int )u );
    }

    static inline std::string toString( double d ) {
        return doubleToString( d );
    }

    static inline std::string intToString( int i ) {
        if( i == 0 ) { return std::string( "0" ); }

        char * s = ( char * )malloc( MAX_NUMBER_LENGTH );
        snprintf( s, MAX_NUMBER_LENGTH, "%d", i );

        return std::string( s );
    }

    static inline std::string  doubleToString( double d ) {
        if( d == 0.0 ) { return std::string( "0" ); }

        char * const s = ( char * )malloc( MAX_NUMBER_LENGTH );
        snprintf( s, MAX_NUMBER_LENGTH, "%.4g", d );

        return std::string( s );
    }

    static inline std::string doubleToNormalString( double d ) {
        if( d == 0.0 ) { return std::string( "0" ); }

        char * s = ( char * )malloc( MAX_NUMBER_LENGTH );
        snprintf( s, MAX_NUMBER_LENGTH, "%.4f", d );

        return std::string( s );
    }

    static inline std::string doubleToString( const double d, const unsigned prec, const int & numS ) {
        std::string tmp = cnvt::doubleToString( d,prec );
        return cnvt::addSpaces( tmp, numS );
    }


    static inline std::string doubleToString( double d, unsigned prec ) {
        char s[100];

        switch( prec ) {
        case 1:
            sprintf( s, "%.1e", d );
            return std::string( s );
        case 2:
            sprintf( s, "%.2e", d );
            return std::string( s );
        case 3:
            sprintf( s, "%.3e", d );
            return std::string( s );
        case 4:
            sprintf( s, "%.4e", d );
            return std::string( s );
        case 5:
            sprintf( s, "%.5e", d );
            return std::string( s );
        case 6:
            sprintf( s, "%.6e", d );
            return std::string( s );
        case 7:
            sprintf( s, "%.7e", d );
            return std::string( s );
        case 8:
            sprintf( s, "%.8e", d );
            return std::string( s );
        case 9:
            sprintf( s, "%.9e", d );
            return std::string( s );
        case 10:
            sprintf( s, "%.10e", d );
            return std::string( s );
        case 11:
            sprintf( s, "%.11e", d );
            return std::string( s );
        case 12:
            sprintf( s, "%.12e", d );
            return std::string( s );
        case 13:
            sprintf( s, "%.13e", d );
            return std::string( s );
        case 14:
            sprintf( s, "%.14e", d );
            return std::string( s );
        case 15:
            sprintf( s, "%.15e", d );
            return std::string( s );
        case 16:
            sprintf( s, "%.16e", d );
            return std::string( s );
        case 17:
            sprintf( s, "%.17e", d );
            return std::string( s );
        case 18:
            sprintf( s, "%.18e", d );
            return std::string( s );
        case 19:
            sprintf( s, "%.19e", d );
            return std::string( s );
        case 20:
            sprintf( s, "%.20e", d );
            return std::string( s );
        case 21:
            sprintf( s, "%.21e", d );
            return std::string( s );
        case 22:
            sprintf( s, "%.22e", d );
            return std::string( s );
        case 23:
            sprintf( s, "%.23e", d );
            return std::string( s );
        }

        assert( false );
        return "";
    }

    static inline std::string addSpaces( const std::string & s,  int tl ) {
        size_t sz = ( s.length()>tl )? 0:tl-s.length();

        return s + std::string( sz,' ' );
    }

    static inline const std::string changeExtension( const std::string & name, const std::string & extension ) {

        std::string basename;

        std::string::size_type dotIdx   = name.rfind( '.' );
        std::string::size_type slashIdx = name.rfind( '/' );

        const bool dotFound   = dotIdx   < name.length();
        const bool slashFound = slashIdx < name.length();
        const bool bothFound  = dotFound && slashFound;

        if( bothFound ) {
            if( dotIdx < slashIdx ) {
                // no file extension
                basename = name;
            }
            else {
                basename = name.substr( 0, dotIdx );
            }
        }
        else if( dotFound ) {
            basename = name.substr( 0, dotIdx );
        }

        return basename + "." + extension;
    }


    static inline const std::string removeLastExtension( const std::string & name ) {

        std::string::size_type idx = name.rfind( '.' );
        return ( ( idx < name.length() )? name.substr( 0,idx ) : name );
    }

    static inline const std::string removeAllExtensions( const std::string & name ) {

        std::string::size_type idx = name.find( '.' );
        return ( ( idx < name.length() )? name.substr( 0,idx ) : name );
    }

    static inline double max( const std::vector<double> & xs ) {
        unsigned i;
        return max( i, xs );
    }

    static inline double max( unsigned & s, const std::vector<double> & xs ) {
        assert( xs.size() > 0 );

        double m = xs[0];
        s = 0;

        for( unsigned i = 1; i < xs.size(); i ++ )
            if( xs[i] > m ) {
                m = xs[i];
                s = i;
            }

        return m;
    }

    static inline unsigned addToStringVecFromDelimitedString( std::vector<std::string> & strVec, const std::string & dstr, const std::string & delimiter ) {
        unsigned num = 0, startP = 0, endP;

        do {
            num ++;
            endP = dstr.find( delimiter,startP );
            strVec.push_back( dstr.substr( startP,endP-startP ) );
            startP = endP + delimiter.length();
        }
        while( endP < dstr.length() );

        return num;
    }

    static inline unsigned addToStringVecFromDelimitedString( std::vector<std::string> & strVec, const std::string & dstr ) {
        return addToStringVecFromDelimitedString( strVec, dstr, "," );
    }

    static inline std::ostream & doubleVectorToOstream( std::ostream & os, const std::vector<double> & dvec ) {
        for( unsigned i = 0; i < dvec.size(); i ++ ) {
            if( i == 0 ) { os << dvec[i]; }
            else { os << " " << dvec[i]; }
        }

        return os;
    }

    static inline std::ostream & doubleVectorToMatlabOstream( std::ostream & os, const std::vector<double> & dv ) {
        return doubleVectorToMatlabOstream( os, dv, "" );
    }

    static inline std::ostream & doubleVectorToMatlabOstream( std::ostream & os, const std::vector<double> & dv, const std::string & name ) {
        if( name.length() != 0 ) { os << name << " = "; }

        os << "[ ..." << std::endl;

        for( std::vector<double>::size_type i = 0; i < dv.size(); i++ ) {
            os << dv[i] << ";" << std::endl;
        }

        os << "];" << std::endl;

        return os;
    }

    static inline std::ostream & toOstream( std::ostream & os, const std::map<std::string,double> & strDblMap ) {

        std::map<std::string,double>::const_iterator it;

        for( it = strDblMap.begin(); it != strDblMap.end(); it++ ) {

            os << addSpaces( it->first,MAX_STRING_LENGTH ) << " " << doubleToString( it->second,PRECISION_NUMBER ) << std::endl;
        }

        return os;
    }

    static inline std::ostream & toOstream( std::ostream & os, const std::map<std::string,double> & strDblMap, const std::string & fS, const std::string & sS ) {

        std::map<std::string,double>::const_iterator it;

        for( it = strDblMap.begin(); it != strDblMap.end(); it++ ) {

            if( it->first.find( fS ) >= it->first.length() && it->first.find( sS ) >= it->first.length() ) {

                os << addSpaces( it->first,MAX_STRING_LENGTH ) << " " << doubleToString( it->second,PRECISION_NUMBER ) << std::endl;
            }
        }

        return os;
    }

    static inline const std::string addTH( unsigned num ) {
        const std::string NUM = intToString( num );

        if( num == 1 ) { return NUM + "st"; }
        if( num == 2 ) { return NUM + "nd"; }
        if( num == 3 ) { return NUM + "rd"; }

        return NUM + "th";
    }

    static inline const std::string addS( const std::string & s, int n ) {
        assert( n >= 0 );

        if( n == 0 || n == 1 ) { return s; }

        return s + "s";
    }

    static inline const std::string addS( const std::string & s, double d ) {
        assert( d >= 0.0 );

        if( d <= 1.0 ) { return s; }

        return s + "s";
    }

    static inline const std::string getFileExtension( const std::string & s ) {
        std::string::size_type lo = s.rfind( "." );

        if( lo < s.length() ) { return s.substr( lo+1,s.length()-lo-1 ); }

        return "";
    }

    static std::vector<double> mapToDoubleVector( const std::map<std::string,double> & doubleMap );

    static std::ostream & matlabPlotToOstream( std::ostream & os, const std::vector<std::string> & xnames, const std::vector<std::string> & ynames ) {
        return matlabPlotToOstream( os, xnames, ynames, false );
    }

    static std::ostream & matlabPlotToOstream( std::ostream & os, const std::vector<std::string> & xnames, const std::vector<std::string> & ynames, bool marker );

    static std::ostream & matlabLegendToOstream( std::ostream & os, const std::vector<std::string> & );

    static double interpolate( double xx, double x1, double y1, double x2, double y2 );

    static std::vector<double> linSpaceToVec( double s, double e, size_t num );

    static double getMean( const std::vector<double> & vec );

    static double getSTD( const std::vector<double> & vec ) {
        return getSTD( vec, getMean( vec ) );
    }

    static double getSTD( const std::vector<double> & vec, double mean );

};
#endif
