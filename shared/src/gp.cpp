#include <math.h>

#include "gp.hpp"
#include "cnvt.hpp"
#include "ggp.hpp"

using namespace std;

#define DB1 false
#define DB2 false
#define DB3 false
#define DB4 false

static const std::string Xstr( "x" );
static const std::string Cstr( "c" );
static const std::string OBJstr( "obj       " );

static const int nodeWidth = 100; // in pixels
static const int nodeHeight = 30; // in pixels
static const int linkHeight = 50; // in pixels

// MONOMIAL CLASS MEMBER FUNCTION DEFINITIONS

ostream & operator << ( ostream & os, const gposy & gp ) {
    return gp.toOstream( os );
}

monomial & monomial::multiply( int varNum, double pwr ) {

    assert( varNum >= 0 );

    if( pwr == 0.0 ) {
        return *this;
    }

    std::map<int,double>::iterator it = powers.find( varNum );

    if( it == powers.end() ) {
        powers[varNum] = pwr;
    }
    else {

        if( it->second == -pwr ) {
            powers.erase( it );
        }
        else {
            it->second += pwr;
        }
    }

    return * this;
}

/* deprecated
monomial & monomial::divide( int varNum, double pwr )
{
  assert( varNum >= 0 );
  if( pwr == 0.0 ) return *this;

  std::map<int,double>::iterator itr = powers.find(varNum);

  if( itr == powers.end() )
    powers[varNum] = -pwr;
  else
  {
    if( itr->second == pwr ) powers.erase(itr);
    else itr->second -= pwr;
  }

  return * this;
}
*/

monomial & monomial::multiply( const monomial & umn ) {

    std::map<int,double>::const_iterator it;

    for( it = umn.powers.begin(); it != umn.powers.end(); it++ ) {
        this->multiply( it->first, it->second );
    }

    return *this;
}

/* deprecated
monomial & monomial::divide( const monomial & umn )
{
  std::map<int,double>::const_iterator itr;

  for( itr = umn.powers.begin(); itr != umn.powers.end(); itr ++ )
    this->divide( itr->first, itr->second );

  return *this;
}
*/

monomial & monomial::power( double ex ) {
    std::map<int,double>::iterator itr = powers.begin();
    for( ; itr != powers.end(); itr ++ ) {
        itr->second *= ex;
    }

    return *this;
}

double monomial::evaluate( const std::vector<double> & values ) const {
    double ev = 1.0;
    double a, b;

    std::map<int,double>::const_iterator it = powers.begin();

    for( it = powers.begin(); it != powers.end(); it++ ) {
        assert( it->first < values.size() );

        a = values[ it->first ];
        b = it->second;

        assert( a >  0.0 );
        assert( b != 0.0 );

        ev *= pow( a, b );
    }

    return ev;
}

// askers
bool monomial::isSameType( const monomial & m ) const {
    std::map<int,double>:: const_iterator it1 = this->powers.begin();
    std::map<int,double>:: const_iterator it2 = m.powers.begin();

    while( it1 != this->powers.end() && it2 != m.powers.end() ) {
        assert( it1->second != 0.0 && it2->second != 0.0 );

        if( it1->first == it2->first && it1->second == it2->second ) {
            it1 ++;
            it2 ++;
        }
        else {
            return false;
        }
    }

    if( it1 != this->powers.end() || it2 != m.powers.end() ) {
        return false;
    }

    return true ;
}

// print/ostream

ostream & monomial::toOstream( ostream & os, const std::string & x ) const {

    std::map<int,double>::const_iterator it;

    for( it = powers.begin(); it != powers.end(); it++ ) {
        if( it != powers.begin() ) { os << " "; }

        if( it->second == 1.0 ) { os << x << ( it->first+1 ); }
        else { os << x << ( it->first+1 ) << "^" << it->second; }
    }

    return os;
}

ostream & monomial::toOstream( ostream & os, const symbol_table & symtab ) const {
    std::map<int,double>::const_iterator it;

    for( it = powers.begin(); it != powers.end(); it++ ) {
        if( it != powers.begin() ) { os << " "; }

        if( it->second == 1.0 ) {
            os << symtab[it->first];
        }
        else {
            os << symtab[it->first] << "^" << it->second;
        }
    }

    return os;
}

const std::string monomial::toString( const symbol_table & symtab ) const {
    std::string mm;
    std::map<int,double>::const_iterator it;

    for( it = powers.begin(); it != powers.end(); it++ ) {

        if( it != powers.begin() ) {
            mm += " ";
        }

        if( it->second == 1.0 ) {
            mm += symtab.getSymbol( it->first );
        }
        else {
            mm += symtab.getSymbol( it->first ) + "^" + cnvt::doubleToString( it->second );
        }
    }

    return mm;
}

const std::string monomial::toString( const symbol_table & symtab, const std::string & nodename, const std::string & searchS, const std::string & replaceS ) const {

    std::string mm;
    std::map<int,double>::const_iterator it;

    for( it = powers.begin(); it != powers.end(); it++ ) {

        if( it != powers.begin() ) {
            mm += " ";
        }

        if( it->second == 1.0 ) {
            mm += symtab.getSymbol( nodename,it->first,searchS,replaceS );
        }
        else {
            mm += symtab.getSymbol( nodename,it->first,searchS,replaceS ) + "^" + cnvt::doubleToString( it->second );
        }
    }

    return mm;
}

ostream & monomial::matlabAbToOstream( ostream & os, int & rowNum ) const {

    rowNum++;

    std::map<int,double>::const_iterator it;

    for( it = powers.begin(); it != powers.end(); it ++ ) {
        os << "A(" << rowNum << "," << ( it->first+1 ) << ")\t= " << it->second << ";" << endl;
    }

    return os;
}

ostream & monomial::mpsFormatToOstream( ostream & os, int & monoNum, const symbol_table & symtab, bool inverse, bool obj ) const {

    std::map<int,double>::const_iterator it;

    monoNum++;

    if( powers.size() == 0 ) {

        os << "    " << cnvt::addSpaces( Xstr+cnvt::intToString( monoNum ),10 ) << cnvt::addSpaces( Cstr+cnvt::intToString( 1 ),10 ) << 0.0 << endl;
        return os;
    }

    for( it = powers.begin(); it != powers.end(); it++ ) {

        os << "    " << cnvt::addSpaces( Xstr+cnvt::intToString( monoNum ),10 ) << cnvt::addSpaces( Cstr+cnvt::intToString( it->first+1 ),10 ) << ( inverse? -( it->second ):it->second ) << endl;
    }

    if( obj ) {
        os << "    " << cnvt::addSpaces( Xstr+cnvt::intToString( monoNum ),10 ) << cnvt::addSpaces( Cstr+cnvt::intToString( symtab.getNumberOfSymbols()+1 ),10 ) << "1" << endl;
    }

    return os;
}

/*
ostream & monomial::treeTexToOstream
( ostream & os, const symbol_table & symtab, int xcenter, int depth, int & x ) const
{
  std::map<int,double>::const_iterator itr;

  for( itr = powers.begin(); itr != powers.end(); itr ++ )
  {
    os << tex::putText
    ( std::string("$(")+symtab[itr->first]+","+doubleToString(itr->second)+")$",
      x*nodeWidth + nodeWidth/2, -(depth+1)*linkHeight, "c" ) << endl;

    os << tex::drawLine( xcenter, -depth*linkHeight,
      x*nodeWidth+nodeWidth/2, -(depth+1)*linkHeight,nodeHeight ) << endl;

    x ++;
  }
}
*/

/*
void monomial::writeMSKgpoptFormat( ostream & wrt, int rowNum ) const
{
  std::map<int,double>::const_iterator itr;

  for( itr = powers.begin(); itr != powers.end(); itr ++ )
    wrt << "A(" << rowNum << "," << (itr->first+1) << ")="
        << itr->second << ";" << endl;
}

void monomial::writeMSKgpoptFormat( ostream & wrt, int rowNum, bool inverse ) const
{
  if( !inverse ) writeMSKgpoptFormat( wrt, rowNum );

  std::map<int,double>::const_iterator itr;
  for( itr = powers.begin(); itr != powers.end(); itr ++ )
    wrt << "A(" << rowNum << "," << (itr->first+1) << ")="
        << (-itr->second) << ";" << endl;
}

void monomial::writeMPSFormat( ostream & wrt, int monoNum, bool inverse ) const
{
  std::map<int,double>::const_iterator itr;
  for( itr = powers.begin(); itr != powers.end(); itr ++ )
    wrt << "    " << cnvt::addSpaces(Xstr+cnvt::intToString(monoNum),10)
        << cnvt::addSpaces(Cstr+cnvt::intToString(itr->first+1),10)
    << (inverse? -(itr->second):itr->second ) << endl;
}
*/

// GPOSY CLASS MEMBER FUNCTION  DEFINITIONS

gposy::gposy( const std::vector<gposy *> & gposyVec ) {
    isS = isM = false;
    isP = true;
    constant = 1.0;

    gposy & maxP = *( new gposy() );
    maxP.isS = maxP.isP = false;
    maxP.isM = true;

    for( int i = 0; i < gposyVec.size(); i ++ ) {
        maxP.gps.push_back( gposyVec[i] );
    }

    gps.push_back( & maxP );
    exps.push_back( 1.0 );
}

gposy::gposy( const gposy & gp ) {
    newlyAssign( gp );
}

const gposy & gposy::operator=( const gposy & gp ) {
    freeMemory();
    newlyAssign( gp );
    return *this;
}

gposy::~gposy() {
    freeMemory();
}

void gposy::newlyAssign( const gposy & gp ) {
    constant = gp.constant;
    umn = gp.umn;

    if( DB4 ) { cout << "haha1" << endl; }
    for( int i = 0; i < gp.gps.size(); i++ ) {
        gps.push_back( new gposy( *gp.gps[i] ) );
    }

    for( int i = 0; i < gp.exps.size(); i++ ) {
        exps.push_back( gp.exps[i] );
    }

    if( DB4 ) { cout << "haha2" << endl; }

    isP = gp.isP;
    isS = gp.isS;
    isM = gp.isM;

    if( DB4 ) { cout << "haha3" << endl; }
}

void gposy::freeMemory() {
    for( int i = 0; i < gps.size(); i++ ) { delete gps[i]; }
    gps.clear();

    exps.clear();
}

ostream & gposy::toOstream( ostream & os, const std::string & x ) const {
    if( this->isProduct() ) {
        if( this->isConstant() ) {
            os << getConstant();
            return os;
        }

        if( hasConstant() ) {
            os << getConstant() << " ";
        }

        if( DB1 ) { cout << "before" << endl; }
        umn.toOstream( os,x );
        if( DB1 ) { cout << "after" << endl; }

        if( ( hasConstant() || !umn.isEmpty() ) && gps.size() != 0 ) { os << " "; }

        if( gps.size() != 0 )
            for( int i = 0; i < gps.size(); i ++ ) {
                if( i != 0 ) {
                    os << " ";
                }

                if( gps[i]->isSum() ) {
                    gps[i]->toOstream( os << "( ",x ) << " )";
                }
                else if ( gps[i]->isMax() ) {
                    gps[i]->toOstream( os,x );
                }
                else {
                    assert( false );
                }

                if( exps[i] != 1.0 ) {
                    os << "^" << exps[i];
                }
            }

    }
    else if( this->isSum() ) {
        assert( gps.size() >= 2 );

        for( int i = 0; i < gps.size(); i ++ ) {
            gps[i]->toOstream( os,x );
            if( i < gps.size() - 1 ) {
                os << " + ";
            }
        }
    }
    else if( this->isMax() ) {
        assert( gps.size() >= 2 );

        os << "max( ";
        gps[0]->toOstream( os,x );

        if( DB1 ) { cout << "gps.size()=" << gps.size() << endl; }
        for( int i = 1; i < gps.size(); i ++ ) {
            os << ", ";
            gps[i]->toOstream( os,x );
        }

        os << " )";
    }
    else {
        assert( false );
    }

    return os;

}

ostream & gposy::toOstream( ostream & os, const symbol_table & symtab ) const {
    if( this->isProduct() ) {
        if( this->isConstant() ) {
            os << getConstant();
            return os;
        }

        if( hasConstant() ) { os << getConstant() << " "; }

        umn.toOstream( os,symtab );

        if( ( hasConstant() || !umn.isEmpty() ) && gps.size() != 0 ) {
            os << " ";
        }

        if( gps.size() > 0 )
            for( int i = 0; i < gps.size(); i ++ ) {
                if( i != 0 ) {
                    os << " ";
                }

                if( gps[i]->isSum() ) {
                    gps[i]->toOstream( os << "( ",symtab ) << " )";
                }
                else if ( gps[i]->isMax() ) {
                    gps[i]->toOstream( os,symtab );
                }
                else {
                    assert( false );
                }

                if( exps[i] != 1.0 ) {
                    os << "^" << exps[i];
                }
            }

    }
    else if( this->isSum() ) {
        assert( gps.size() >= 2 );

        for( int i = 0; i < gps.size(); i ++ ) {
            gps[i]->toOstream( os,symtab );
            if( i < gps.size() - 1 ) {
                os << " + ";
            }
        }
    }
    else if( this->isMax() ) {
        assert( gps.size() >= 2 );

        os << "max( ";
        gps[0]->toOstream( os,symtab );

        for( int i = 1; i < gps.size(); i ++ ) {
            os << ", ";
            gps[i]->toOstream( os,symtab );
        }

        os << " )";
    }
    else {
        assert( false );
    }

    return os;
}

const std::string gposy::toString( const symbol_table & symtab ) const {
    std::string gpStr;

    if( this->isProduct() ) {
        if( this->isConstant() ) {
            return cnvt::doubleToString( getConstant() );
        }

        if( hasConstant() ) {
            gpStr += cnvt::doubleToString( getConstant() ) + " ";
        }

        gpStr += umn.toString( symtab );

        if( ( hasConstant() || !umn.isEmpty() ) && gps.size() != 0 ) {
            gpStr += " ";
        }

        if( gps.size() > 0 )
            for( int i = 0; i < gps.size(); i ++ ) {
                if( i != 0 ) { gpStr += " "; }

                if( gps[i]->isSum() ) { gpStr+="( "+gps[i]->toString( symtab )+" )"; }
                else if ( gps[i]->isMax() ) { gpStr += gps[i]->toString( symtab ); }
                else { assert( false ); }

                if( exps[i] != 1.0 ) { gpStr +=  "^" + cnvt::doubleToString( exps[i] ); }
            }
    }
    else if( this->isSum() ) {
        assert( gps.size() >= 2 );

        for( int i = 0; i < gps.size(); i ++ ) {
            gpStr += gps[i]->toString( symtab );

            if( i < gps.size() - 1 ) { gpStr += " + "; }
        }
    }
    else if( this->isMax() ) {
        assert( gps.size() >= 2 );

        gpStr += "max( ";
        gpStr += gps[0]->toString( symtab );

        for( int i = 1; i < gps.size(); i ++ ) {
            gpStr += ", ";
            gpStr += gps[i]->toString( symtab );
        }

        gpStr += " )";
    }
    else {
        assert( false );
    }

    return gpStr;
}

const std::string gposy::toString
( const symbol_table & symtab, const std::string & nodename,
  const std::string & searchS, const std::string & replaceS ) const {
    std::string gpStr;

    if( this->isProduct() ) {
        if( this->isConstant() ) {
            return cnvt::doubleToString( getConstant() );
        }

        if( hasConstant() ) { gpStr += cnvt::doubleToString( getConstant() ) + " "; }

        gpStr += umn.toString( symtab,nodename,searchS,replaceS );

        if( ( hasConstant() || !umn.isEmpty() ) && gps.size() != 0 ) { gpStr += " "; }

        if( gps.size() > 0 )
            for( int i = 0; i < gps.size(); i ++ ) {
                if( i != 0 ) { gpStr += " "; }

                if( gps[i]->isSum() ) {
                    gpStr+="( "+gps[i]->toString( symtab,nodename,searchS,replaceS )+" )";
                }
                else if ( gps[i]->isMax() ) {
                    gpStr += gps[i]->toString( symtab,nodename,searchS,replaceS );
                }
                else {
                    assert( false );
                }

                if( exps[i] != 1.0 ) { gpStr +=  "^" + cnvt::doubleToString( exps[i] ); }
            }

    }
    else if( this->isSum() ) {
        assert( gps.size() >= 2 );

        for( int i = 0; i < gps.size(); i ++ ) {
            gpStr += gps[i]->toString( symtab,nodename,searchS,replaceS );

            if( i < gps.size() - 1 ) { gpStr += " + "; }
        }
    }
    else if( this->isMax() ) {
        assert( gps.size() >= 2 );

        gpStr += "max( ";
        gpStr += gps[0]->toString( symtab,nodename,searchS,replaceS );

        for( int i = 1; i < gps.size(); i ++ ) {
            gpStr += ", ";
            gpStr += gps[i]->toString( symtab,nodename,searchS,replaceS );
        }

        gpStr += " )";
    }
    else {
        assert( false );
    }

    return gpStr;
}

ostream & gposy::matlabAbToOstream( ostream & os, int & rowNum ) const {
    assert( isPosynomial() );

    if( isProduct() ) {
        assert( isMonomial() );
        umn.matlabAbToOstream( os, rowNum );
        os << "b(" << rowNum << ")\t= "
           << log( getConstant() ) << ";" << endl;
    }
    else {
        for( int i = 0; i < gps.size(); i ++ ) {
            gps[i]->matlabAbToOstream( os, rowNum );
        }
    }

    return os;
}

ostream & gposy::mpsFormatToOstream
( ostream & os, int & monoNum, const symbol_table & symtab, bool inverse, bool obj ) const {
    assert( isPosynomial() );
    if( inverse ) { assert( isMonomial() ); }

    if( isMonomial() ) {
        return umn.mpsFormatToOstream( os, monoNum, symtab, inverse, obj );
    }
    else {
        assert( isSum() );
        for( int i = 0; i < gps.size(); i ++ ) {
            gps[i]->mpsFormatToOstream( os, monoNum, symtab, inverse, obj );
        }

        return os;
    }
}

ostream & gposy::coefficientsToOstream( ostream & os, bool inverse ) const {
    assert( isPosynomial() );
    if( inverse ) { assert( isMonomial() ); }

    if( isMonomial() ) {
        if( inverse ) {
            os << ( 1.0/getConstant() ) << endl;
        }
        else {
            os << getConstant() << endl;
        }
    }
    else {
        assert( isSum() );
        for( int i = 0; i < gps.size(); i ++ ) {
            gps[i]->coefficientsToOstream( os, inverse );
        }
    }

    return os;
}

/*
int gposy::writeMSKgpoptFormat
( ostream & wrt, int rowNum, int ineqNum ) const
{
  assert( posys.size() == 0 && maxposys.size() == 0 );

  umn.writeMSKgpoptFormat( wrt, rowNum );
  wrt << "c(" << rowNum << ",1)=" << constant << ";" << endl;
  wrt << "map(" << rowNum << ",1)=" << ineqNum << ";" << endl;

  return (rowNum+1);
}

int gposy::writeMSKgpoptFormat
( ostream & wrt, int rowNum, int ineqNum, bool inverse ) const
{
  assert( posys.size() == 0 && maxposys.size() == 0 );

  if( !inverse ) return writeMSKgpoptFormat(wrt,rowNum,ineqNum);

  umn.writeMSKgpoptFormat( wrt, rowNum, true );
  wrt << "c(" << rowNum << ",1)=" << (1.0/constant) << ";" << endl;
  wrt << "map(" << rowNum << ",1)=" << ineqNum << ";" << endl;

  return (rowNum+1);
}
*/

gposy & gposy::add( gposy & gp ) {
    if( isProduct() ) {
        if( gp.isProduct() ) {
            if( this->isSameType( gp ) ) {
                constant += gp.constant;
                delete &gp;
                return *this;
            }

            gposy & gpSum = *( new gposy );
            gpSum.isS = true;
            gpSum.gps.push_back( this );
            gpSum.gps.push_back( & gp );

            if( DB3 ) { cout << "gpSum=" << gpSum << endl; }

            return gpSum;
        }
        else if( gp.isSum() ) {
            std::vector<gposy *>::iterator itr = gp.gps.begin();

            for( ; itr != gp.gps.end(); itr ++ )
                if( this->isSameType( *( *itr ) ) ) {
                    this->constant += ( *itr )->constant;

                    delete ( *itr );
                    gp.gps.erase( itr );

                    break;
                }

            gp.gps.insert( gp.gps.begin(),this );

            return gp;
        }
        else {
            assert( false );
        }
    }
    else if( isSum() ) {
        assert( gps.size() >= 2 );

        if( gp.isProduct() ) {
            for( int i = 0; i < gps.size(); i ++ )
                if( gps[i]->isSameType( gp ) ) {
                    gps[i]->constant += gp.constant;

                    delete ( & gp );
                    return *this;
                }

            gps.push_back( & gp );

            return *this;
        }
        else if( gp.isSum() ) {
            for( int i = 0; i < gp.gps.size(); i ++ ) {
                int j;
                for( j = 0; j < gps.size(); j ++ )
                    if( gps[j]->isSameType( * gp.gps[i] ) ) {
                        gps[j]->constant += gp.gps[i]->constant;
                        break;
                    }

                if( j == gps.size() ) {
                    gps.push_back( new gposy( *gp.gps[i] ) );
                }
            }

            delete &gp;

            return *this;
        }
        else {
            assert( false );
        }
    }
    else {
        assert( false );
    }
}

gposy & gposy::multiply( gposy & gp ) {
    if( isProduct() ) {
        if( gp.isProduct() ) {
            constant *= gp.constant;
            umn.multiply( gp.umn );

            for( int i = 0; i < gp.gps.size(); i ++ ) {
                int j;
                for( j = 0; j < gps.size(); j ++ )
                    if( gps[j]->isSameType( * gp.gps[i] ) ) {
                        exps[j] += 1.0;
                        break;
                    }

                if( j == gps.size() ) {
                    gps.push_back( new gposy( *gp.gps[i] ) );
                    exps.push_back( gp.exps[i] );
                }
            }

            delete &gp;
            return *this;
        }
        else if( gp.isSum() ) {
            assert( gp.gps.size()>=2 );
            if( isMonomial() ) {
                for( int i = 0; i < gp.gps.size(); i ++ ) {
                    gp.gps[i]->constant *= constant;
                    gp.gps[i]->umn.multiply( umn );
                }

                delete this;
                return gp;
            }
            else {
                int j;
                for( j = 0; j < gps.size(); j ++ )
                    if( gps[j]->isSameType( gp ) ) {
                        exps[j] += 1.0;

                        delete &gp;
                        return *this;
                    }

                gps.push_back( & gp );
                exps.push_back( 1.0 );

                return *this;
            }
        }
        else {
            assert( false );
        }
    }
    else if( isSum() ) {
        assert( gps.size() >= 2 );

        if( gp.isProduct() ) {
            if( gp.isMonomial() ) {
                for( int i = 0; i < gps.size(); i ++ ) {
                    gps[i]->umn.multiply( gp.umn );
                    gps[i]->constant *= gp.constant;
                }

                delete &gp;
                return *this;
            }
            else {
                double ex = 1.0;
                std::vector<gposy *>::iterator itr = gp.gps.begin();

                for( ; itr != gp.gps.end(); itr ++ )
                    if( this->isSameType( *( *itr ) ) ) {
                        ex += 1.0;

                        delete ( *itr );
                        gp.gps.erase( itr );

                        break;
                    }

                gp.gps.insert( gp.gps.begin(),this );
                gp.exps.insert( gp.exps.begin(),ex );

                return gp;
            }
        }
        else if( gp.isSum() ) {
            if( this->isSameType( gp ) ) {
                delete &gp;

                return *( new gposy( *this,2.0 ) );
            }

            gposy & gpP = *( new gposy() );
            gpP.isP = true;
            gpP.gps.push_back( this );
            gpP.exps.push_back( 1.0 );
            gpP.gps.push_back( &gp );
            gpP.exps.push_back( 1.0 );

            return gpP;
        }
        else {
            assert( false );
        }
    }
    else {
        assert( false );
    }

    return * this;
}

gposy & gposy::divide( gposy & mono ) {

    assert( mono.isMonomial() );
    return this->multiply( mono.power( -1 ) );
}

gposy & gposy::power( double ex ) {

    if( ex == 0.0 ) {
        delete this;
        return *( new gposy( 1.0 ) );
    }

    if( ex == 1.0 ) { return *this; }

    if( ex < 0.0 ) { assert( this->isMonomial() ); }

    if( isProduct() ) {
        constant = pow( constant,ex );
        umn.power( ex );

        for( int i = 0; i < gps.size(); i ++ ) { exps[i] *= ex; }

        return *this;
    }
    else if ( isSum() ) {
        assert( gps.size() >= 2 );

        return *( new gposy( *this,ex ) );
    }
    else {
        assert( false );
    }
}

bool gposy::isPosynomial() const {

    if( isMonomial() ) {
        return true;
    }
    if( isProduct() ) {
        return false;
    }

    for( int i = 0; i < gps.size(); i++ ) {

        if( ! gps[i]->isMonomial() ) {
            return false;
        }
    }

    return true;
}

// tree drawing methods

/*
ostream & gposy::treeTexToOstream
( ostream & os, const symbol_table & symtab, int xcor, int depth ) const
{
  if( isSum() )
  {
    int xcenter = xcor*nodeWidth + getNumberOfLeafNodes()*nodeWidth/2;
    os << tex::putText( "{\\tt +}", xcenter, -depth*linkHeight, "c" ) << endl;

    for( int x = xcor, i = 0; i < gps.size(); i ++ )
    {
      os << tex::drawLine( xcenter, -depth*linkHeight,
        x*nodeWidth + getNumberOfLeafNodes()*nodeWidth/2,
    -(depth+1)*linkHeight, nodeHeight ) << endl;

      gps[i]->treeTexToOstream( os, x, depth+1 );

      x += gps[i]->getNumberOfLeafNodes();
    }
  }
  else if ( isProduct() )
  {
    int xcenter = xcor*nodeWidth + getNumberOfLeafNodes()*nodeWidth/2;
    os << tex::putText( "{\\tt *}", xcenter, -depth*linkHeight, "c" ) << endl;

    int x = xcor;

    if( constant != 1.0 )
    {
      os << tex::drawLine( xcenter, -depth*linkHeight,
        x*nodeWidth + nodeWidth/2,
    -(depth+1)*linkHeight, nodeHeight ) << endl;

      os << tex::putText
            ( std::string("$") + cnvt::doubleToString(constant) + "$",
           x*nodeWidth + nodeWidth/2, -(depth+1)*linkHeight, nodeHeight )
     << endl;

      x ++;
    }

    umn.treeTexToOstream( os, xcenter, depth, x );

    for( int i = 0; i < gps.size(); i ++ )
    {
      os << tex::drawLine( xcenter, -depth*linkHeight,
        x*nodeWidth + gps[i]->getNumberOfLeafNodes()*nodeWidth/2,
    -(depth+1)*linkHeight, nodeHeight ) << endl;

      gps[i]->treeTexToOstream( os, x, depth+1, exps[i] );

      x += gps[i]->getNumberOfLeafNodes();
    }
  }
  else if ( isMax() )
  {
    int xcenter = xcor*nodeWidth + getNumberOfLeafNodes()*nodeWidth/2;
    os << tex::putText( "{\\tt max}", xcenter, -depth*linkHeight, "c" ) << endl;

    for( int x = xcor, i = 0; i < gps.size(); i ++ )
    {
      os << tex::drawLine( xcenter, -depth*linkHeight,
        x*nodeWidth + gps[i]->getNumberOfLeafNodes()*nodeWidth/2,
    -(depth+1)*linkHeight, nodeHeight ) << endl;

      gps[i]->treeTexToOstream( os, x, depth+1, exps[i] );

      x += gps[i]->getNumberOfLeafNodes();
    }
  }
  else
    assert( false );

  return os;
}

ostream & gposy::
treeTexToOstream( ostream & os, const symbol_table & symtab, int xcor, int depth, double power ) const
{
  int xcenter = xcor*nodeWidth + getNumberOfLeafNodes()*nodeWidth/2;

   os << tex::putText
         ( std::string("{\\tt (ppower}, $") + cnvt::doubleToString(power)
       +  "${\\tt )}", xcenter, -depth*linkHeight, "c" ) << endl;

   os << tex::drawLine( xcenter, -depth*linkHeight,
                        xcenter, -(depth+1)*linkHeight, nodeHeight ) << endl;

   return treeTexToOstream( os, xcor, depth+1 );
}
*/

// conversion into a GP

const gposy & gposy::toGP( symbol_table & symtab, std::vector<std::string> & labels, std::vector<const gposy *> & lefts, std::vector<const gposy *> & rights ) const {

    return this->toNormalPosy( symtab, labels, lefts, rights, ggp::OBJECTIVE_LABEL );
}

void gposy::toGP( symbol_table & symtab, std::vector<std::string> & labels, std::vector<const gposy *> & lefts, std::vector<const gposy *> & rights, const gposy & RHS, const std::string & label, int & num ) const {

    if( isProduct() || isSum() ) {
        int previousSize = lefts.size();

        const gposy & ps = this->toNormalPosy( symtab,labels,lefts,rights,label,num );

        lefts.insert ( lefts.begin()  + previousSize, & ps );
        rights.insert( rights.begin() + previousSize, new gposy( RHS ) );
        labels.insert( labels.begin() + previousSize, label );
    }
    else if( isMax() ) {
        assert( gps.size() >=2 );
        assert( label.length() != 0 );

        if( DB4 ) { cout << "max start" << endl; }

        for( int i = 0; i < gps.size(); i ++ )
            gps[i]->toGP( symtab, labels, lefts, rights, RHS,
                          label + ggp::AUX_INEQ_LABEL_CONJ + cnvt::intToString( ++num ) );

        if( DB4 ) { cout << "max end" << endl; }
    }
    else {
        assert( false );
    }
}

gposy & gposy::toNormalPosy( symbol_table & symtab, std::vector<std::string> & labels, std::vector<const gposy *> & lefts, std::vector<const gposy *> & rights, const std::string & label, int & num ) const {

    if( DB1 ) { cout << "hey" << endl; }
    assert( label.length() != 0 );

    if( isProduct() ) {
        gposy & ps = *( new gposy( getConstant(),getMonomial() ) );

        for( int i = 0; i < gps.size(); i ++ ) {
            int newVar = symtab.makeAndGetIndexOfAuxSymbol();

            ps.umn.multiply( newVar, exps[i] );
            if( gps[i]->isMax() ) {
                gps[i]->toGP( symtab,labels,lefts,rights,gposy( newVar ),label,num );
            }
            else
                gps[i]->toGP( symtab, labels, lefts, rights, gposy( newVar ),
                              label + ggp::AUX_INEQ_LABEL_CONJ + cnvt::intToString( ++num ) );
        }

        return ps;
    }
    else if( isSum() ) {
        assert( gps.size() >= 2 );

        if( DB2 ) { cout << "hi=" << gps.size() << endl; }

        gposy * ps, * tps;

        ps = & gps[0]->toNormalPosy( symtab,labels,lefts,rights,label,num );
        /*
        if( label.length() == 0 )
          ps = & gps[0]->toNormalPosy(symtab,labels,lefts,rights, "" );
        else
          ps = & gps[0]->toNormalPosy(symtab,labels,lefts,rights,
            label + ggp::AUX_INEQ_LABEL_CONJ + cnvt::intToString(1) );
        */

        for( int i = 1; i < gps.size(); i ++ ) {
            tps = & gps[i]->toNormalPosy( symtab,labels,lefts,rights,label,num );
            /*
            if( label.length() == 0 )
              tps = & gps[i]->toNormalPosy( symtab, labels, lefts, rights, "" );
            else
              tps = & gps[i]->toNormalPosy( symtab, labels, lefts, rights,
            label + ggp::AUX_INEQ_LABEL_CONJ + cnvt::intToString(i+1) );
            */

            ps = & ps->add( * tps );
        }

        return * ps;
    }
    else {
        cout << *this << endl;
        assert( false );
    }
}

/*
double gposy::evaluate( const double * values ) const
{
  double ev;

  if( isSum() )
  {
    assert( gps.size() >= 2 );
    ev = 0.0;
    for( int i = 0; i < gps.size(); i ++ ) ev += gps[i]->evaluate(values);
  }
  else if ( isProduct() )
  {
    ev = constant;
    ev *= umn.evaluate( values );

    for( int i = 0; i < gps.size(); i ++ )
      ev *= pow(gps[i]->evaluate(values),exps[i]);
  }
  else if ( isMax() )
  {
    assert( gps.size() >= 2 );
    ev = gps[0]->evaluate(values);

    double val;
    for( int i = 1; i < gps.size(); i ++ )
    {
      if( (val=gps[i]->evaluate(values)) > ev ) ev = val;
    }
  }
  else
    assert(false);

  return ev;
}
*/

double gposy::evaluate( const std::vector<double> & values ) const {
    //   cout << "called" << endl;
    double ev;

    if( isSum() ) {
        assert( gps.size() >= 2 );
        ev = 0.0;
        for( int i = 0; i < gps.size(); i ++ ) { ev += gps[i]->evaluate( values ); }
    }
    else if ( isProduct() ) {
        ev = constant;
        ev *= umn.evaluate( values );

        for( int i = 0; i < gps.size(); i ++ ) {
            ev *= pow( gps[i]->evaluate( values ),exps[i] );
        }
    }
    else if ( isMax() ) {
        assert( gps.size() >= 2 );
        ev = gps[0]->evaluate( values );

        double val;
        for( int i = 1; i < gps.size(); i ++ ) {
            if( ( val=gps[i]->evaluate( values ) ) > ev ) { ev = val; }
        }
    }
    else {
        assert( false );
    }

    return ev;
}

double gposy::evaluate( const symbol_table & symtab, const std::string & nodename, const std::map<std::string,double> & optVs, const std::string & str, const double replacedValue ) const {

    std::map<std::string,double>::const_iterator it;
    std::vector<double> vals( symtab.size() );
    bool hasSTRappeared = false;

    std::string key;
    // Here the algorithm is to first find out the values of all the possible
    // symbols that can occur in the gposy. It might lead to something like
    // gatename._LOAD_F_ etc symbols that do not make sense, but they will be
    // removed by the second if statement.
    // Note that this is used for evaluating any kind of gpsoy and so some of
    // the symbols wont be used, just as sometimes, there will be no str variable,
    // as at that time we are only finding the capacitive load.
    for( unsigned i = 0; i < symtab.size(); i ++ ) {

        // -- OLD version of ggpsolexp -- not sure why it was
        // -- different from the one living in the ciropt source,
        // -- but this is the only diff that I found.  Keeping
        // -- the ciropt version as it looks more sensible
        // -- and from what I can see, ggpsolexp does not need symbolic
        // -- evaluation.
        // if( symtab[i] == "Vdd" || symtab[i] == "vdd" )
        if( nodename == "" || glbVars.find( symtab[i] ) != glbVars.end() ) {
            key = symtab[i];
        }
        else {
            key = nodename + "." + symtab[i];
        }

        //cout << key << " is the variable and " << str << " is the string" << endl;
        it = optVs.find( key );

        if( it != optVs.end() ) {
            vals[i] = it->second;
        }
        else if( symtab[i] == str ) {
            // str is typically _LOAD_F_ or _LOAD_R_ to be replaced.
            assert( !hasSTRappeared );
            vals[i] = replacedValue;
            hasSTRappeared = true;
        }
        else {
            //cerr << "Warning : The value of " << symtab[i] << " is not found" << endl;
            //toOstream( cerr, symtab ) << ": " << symtab[i] << endl;
            vals[i] = 0.0;
        }
    }

    return evaluate( vals );
}
