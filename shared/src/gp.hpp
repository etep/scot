#ifndef _SH_GP_H_
#define _SH_GP_H_

#include <string>
#include <vector>
#include <map>
#include <iostream.h>
#include <assert.h>
#include "symtab.hpp"

extern std::map<std::string, int> glbVars;


/**
 * rule : if an operation is not const member,
 * properly delete/new objects,
 * and return itself/or a new object of same type by pass-by-reference,
 * and if an operation is const member,
 * then return a new one as pass-by-value.
 * (does this always make sense??? yes, I think so.. (8/28/03)
**/

/**
 * XXX : We may (not probably) eventaully want to maintain the GLOBAL MONOMIAL
 * and POSYNOMIAL bank, which takes care of all the jobs, such as, check
 * whether a certain monomial/posynomial has been already checked in,
 * if so, only returns the reference to it to the client,
 * if not, makes a new one and
 * returns the reference to it to the client, etc.
 * Then the client must request a new monomial/posynomial only through
 * this big bank.
**/

class monomial;
class gposy;

ostream & operator<< ( ostream &, const gposy & );

// this monomial class stands for monomial with coefficient 1


class monomial {
private:
   std::map<int,double> powers;

   //monomial( const monomial & );
   //const monomial & operator=( const monomial & );

public:
   // constructors
   monomial() {}
   
   monomial( int varNum, double pwr ) {
      multiply( varNum,pwr );
   }

   // operators
   monomial & multiply( int varNum, double pwr );
   //monomial & divide( int varNum, double pwr ); // deprecated

   monomial & multiply( const monomial & umn );
   //monomial & divide( const monomial & umn ); // deprecated

   monomial & power( double ex );

   // getters
   int getNumVariables() const {
      return powers.size();
   }
   double evaluate( const std::vector<double> & values ) const;

   // askers
   bool isEmpty() const {
      return ( powers.size() == 0 );
   }
   bool isSameType( const monomial & ) const;

   // print/ostream
   void print() const {
      print( "x" );
   }
   void print( const std::string & x ) const {
      toOstream( cout,x );
   }
   void print( const symbol_table & st ) const {
      toOstream( cout,st );
   }

   ostream & toOstream( ostream & os ) const {
      return toOstream( os,"x" );
   }
   ostream & toOstream( ostream &, const std::string & ) const;
   ostream & toOstream( ostream &, const symbol_table & ) const;

   const std::string toString( const symbol_table & ) const;
   const std::string toString( const symbol_table &, const std::string &, const std::string &, const std::string & ) const;

   ostream & matlabAbToOstream( ostream & os, int & rowNum ) const;
   ostream & mpsFormatToOstream( ostream & os, int & monoNum, const symbol_table & symtab, bool inverse, bool obj ) const;

   /*
   // tree drawing method
   ostream & treeTexToOstream( ostream & os, const symbol_table & symtab,
       int xcenter, int depth, int & x ) const;
       */

   // mosek related methods
   // deprecated
   /*
   void writeMSKgpoptFormat( ostream &, int, bool ) const;

   void writeMPSFormat( ostream &, int, bool ) const;
   */
};

class gposy {
private:

   double constant;
   monomial umn;
   std::vector<gposy *> gps;
   std::vector<double> exps;
   //std::vector<bool> hasExp;

   bool isP, isS, isM;

   void newlyAssign( const gposy & );
   void freeMemory();

private:
   gposy() {
      isP = isS = isM = false;   // private constructor
      constant = 1.0;
   }
   gposy( gposy & gp, double ex ) {
      isS = isM = false;
      isP = true;

      constant = 1.0;
      gps.push_back( & gp );
      exps.push_back( ex );
   }
   gposy( double cnst, const monomial & mn ) {
      isS = isM = false;
      isP = true;

      constant = cnst;
      umn = mn;
   }

   const gposy & operator=( const gposy & ); // must not be used

public:
   // constructor

   gposy( double cnst ) {
      assert( cnst > 0.0 );
      constant = cnst;
      isS = isM = false;
      isP = true;
   }

   gposy( int varNum ) {
      isS = isM = false;
      isP = true;
      constant = 1.0;
      umn.multiply( varNum, 1.0 );
   }

   gposy( const std::vector<gposy *> & gposyVec );

   gposy( const gposy & );
   ~gposy();

   // print/ostream
   void print() const {
      print( "x" );
   }
   void print( const std::string & x ) const {
      toOstream( cout, x );
   }
   void print( const std::string & x, const std::string & name ) const {
      cout << name;
      print( x );
   }

   void print( const symbol_table & symtab ) const {
      toOstream( cout,symtab );
   }
   void print( const symbol_table & symtab, const std::string & name ) const {
      cout << name;
      print( symtab );
   }

   ostream & toOstream( ostream & os ) const {
      return toOstream( os, "x" );
   }
   ostream & toOstream( ostream &, const std::string & ) const;
   ostream & toOstream( ostream &, const symbol_table & ) const;

   const std::string toString( const symbol_table & st ) const;
   const std::string toString( const symbol_table &, const std::string &, const std::string &, const std::string & ) const;

   // gp.m related methods
   ostream & matlabAbToOstream( ostream & os, int & rowNum ) const;

   // mosek related methods
   ostream & mpsFormatToOstream( ostream & os, int & monoNum, const symbol_table & symtab ) const {
      return mpsFormatToOstream( os, monoNum, symtab, false );
   }

   ostream & mpsFormatToOstream( ostream & os, int & monoNum, const symbol_table & symtab, bool inverse ) const {
      return mpsFormatToOstream( os, monoNum, symtab, inverse, false );
   }

   ostream & mpsFormatToOstream( ostream & os, int & monoNum, const symbol_table & symtab, bool inverse, bool obj ) const;

   ostream & coefficientsToOstream( ostream & os ) const {
      return coefficientsToOstream( os, false );
   }
   ostream & coefficientsToOstream( ostream & os, bool inverse ) const;

   // mosek related methods
   // deprecated; see mpsFormatToOstream
   /*
   int writeMSKgpoptFormat( ostream &, int rowNum, int ineqNum ) const;
   int writeMSKgpoptFormat
     ( ostream &, int rowNum, int ineqNum, bool inverse ) const;
   void writeMPSFormat
     ( ostream &, int &, bool obj, bool inverse, const symbol_table &) const;
   */

   // operators
   gposy & add( gposy & );
   gposy & multiply( gposy & );
   gposy & divide( gposy & );
   gposy & power( double expn );

   gposy & operator+( const gposy & gp ) const {
      return ( ( new gposy( *this ) )->add( *( new gposy( gp ) ) ) );
   }

   gposy & operator*( const gposy & gp ) const {
      return ( ( new gposy( *this ) )->multiply( *( new gposy( gp ) ) ) );
   }

   gposy & operator/( const gposy & gp ) const {
      return ( ( new gposy( *this ) )->divide( *( new gposy( gp ) ) ) );
   }

   gposy & operator^( double expn ) const {
      return ( ( new gposy( *this ) )->power( expn ) );
   }

   //gposy & multiply( double cnst ); // deprecated
   //gposy & multiply( int varNum, double pwr ); // deprecated
   //gposy & multiply( const monomial & mmn ); // deprecated

   //gposy & divide( double cnst ); // deprecated
   //gposy & divide( int varNum, double pwr ); // deprecated

   // getters
   double getConstant() const {
      assert( isProduct() );
      return constant;
   }
   const monomial & getMonomial() const {
      assert( isProduct() );
      return umn;
   }
   int getNumMonomials() const {
      return getNumberOfMonomials();
   }
   int getNumberOfMonomials() const {
      assert( isPosynomial() );

      if( isProduct() ) {
         return 1;
      }
      else {
         return gps.size();
      }
   }
   //double evaluate( const double * ) const;
   double evaluate( const std::vector<double> & ) const;
   double evaluate( const symbol_table & symtab, const std::string & nodename, const std::map<std::string,double> & optVs, const std::string & str, const double replacedValue ) const;

   // askers
   bool isProduct() const {
      return isP;
   }
   bool isSum() const {
      return isS;
   }
   bool isMax() const {
      return isM;
   }

   bool hasConstant() const {
      return ( constant != 1.0 );
   }

   //bool isSimpleMonomial() const { return isMonomial(); } // wrapper function
   bool isMonomial() const {
      return ( isP && gps.size()==0 );
   }
   bool isPosynomial() const;

   bool isConstant() const {
      return ( isMonomial() && umn.isEmpty() );
   }

   bool isSameType( const gposy & gp ) const {
   // XXX: lots of improvement possibility here
   // this is a mere starting version of this method
      return
         this->isProduct()
         && this->gps.size() == 0
         && gp.isProduct()
         && gp.gps.size() == 0
         && this->umn.isSameType( gp.umn );
   }

   /*
   // tree drawing methods
   ostream & treeTexToOstream( ostream & os, const symbol_table & symtab,
       int xcor, int depth ) const;
   ostream & treeTexToOstream( ostream & os, const symbol_table & symtab,
       int xcor, int depth, double power ) const;
   int getNumberOfChildNodes() const;
   int getNumberOfLeafNodes() const;
   */

   // conversion into a GP

   const gposy & toGP
   ( symbol_table & symtab, std::vector<std::string> & labels,
     std::vector<const gposy *> & lefts, std::vector<const gposy *> & rights )
   const;

   void toGP ( symbol_table & symtab, std::vector<std::string> & labels,
               std::vector<const gposy *> & lefts, std::vector<const gposy *> & rights,
               const gposy & RHS, const std::string & label ) const {
      int num=0;
      toGP( symtab,labels,lefts,rights,RHS,label,num );
   }

   void toGP ( symbol_table & symtab, std::vector<std::string> & labels,
               std::vector<const gposy *> & lefts, std::vector<const gposy *> & rights,
               const gposy & RHS, const std::string & label, int & num ) const;

private:
   gposy & toNormalPosy
   ( symbol_table & symtab, std::vector<std::string> & labels,
     std::vector<const gposy *> & lefts, std::vector<const gposy *> & rights,
     const std::string & label ) const {
      int num=0;
      return toNormalPosy( symtab,labels,lefts,rights,label,num );
   }

   gposy & toNormalPosy
   ( symbol_table & symtab, std::vector<std::string> & labels,
     std::vector<const gposy *> & lefts, std::vector<const gposy *> & rights,
     const std::string & label, int & num ) const;

   // max elimination
   //const gposy & maxElimination( int, constraints &, symbol_table & ) const;
};

#endif
