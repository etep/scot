// ggp.h : generalized GP

#ifndef _SH_GGP_H_
#define _SH_GGP_H_

/*
 * XXX: add the destructor of the below two classes.
 */

#include <vector>
#include "gp.hpp"
#include "symtab.h"

class ggp;

ostream & operator<<( ostream & os, const ggp & );

class ggp {
private:
   const gposy & objFunc;
   symbol_table & symtab;

   std::map<std::string,int> inCnstMap, eqCnstMap;
   std::vector<std::string> inCnstLabels, eqCnstLabels;
   std::vector<const gposy *> inCnstLeft, inCnstRight;
   std::vector<const gposy *> eqCnstLeft, eqCnstRight;

   std::vector<int> corrInCnstNum;

   bool hasInMaxLabelLengthUpToDate, hasEqMaxLabelLengthUpToDate;
   int numberOfInLabelsChecked, numberOfEqLabelsChecked;
   int maxInLabelLength, maxEqLabelLength;

   bool hasCnstPosysUpToDate;
   std::vector<const gposy *> inCnstPosys;
   std::vector<const gposy *> eqCnstPosys;
   void updateCnstPosys();

   std::vector<double> optValues;
   double objValue;
   std::vector<double> inDuals, inLefts, inRights, inMargins;
   std::vector<double> eqDuals, eqLefts, eqRights, eqMargins;

   bool isgp;

   bool hasInSizeVecBeenMade;
   std::vector<int> inSizeVec;
   int sumNumMono;

   void makeSizeVec() {
      assert( isGP() );
      assert( getNumberOfEqConstraints() == 0 );

      if( hasInSizeVecBeenMade ) return;

      sumNumMono = 0;
      for( int i = 0; i < inCnstLeft.size(); i ++ ) {
         int sz = inCnstLeft[i]->getNumMonomials();
         inSizeVec.push_back( sz );
         sumNumMono += sz;
      }

      hasInSizeVecBeenMade = true;
   }

   // must-not-be-called-methods
   ggp( const ggp & );
   ggp & operator=( const ggp & );

   // private methods
   /*
   void maxElimination() {;}
   void writeMPSFormat( const std::string & );
   */

public:
   static const unsigned GP_TRANSFORM_NUMBER;
   static const std::string OBJECTIVE_LABEL;
   static const std::string AUX_INEQ_LABEL_CONJ;

   ggp( const gposy & obj, symbol_table & st ) : objFunc( obj ), symtab( st ) {
      isgp = objFunc.isPosynomial();
      hasInSizeVecBeenMade = false;
      hasCnstPosysUpToDate = false;

      hasInMaxLabelLengthUpToDate = hasEqMaxLabelLengthUpToDate = false;
      maxInLabelLength = maxEqLabelLength = 0;
      numberOfInLabelsChecked = numberOfEqLabelsChecked = 0;
   }

   // public methods
   bool addInConstraint( const gposy & left, const gposy & right ) {
      return addInConstraint( left,right,"" );
   }

   bool addInConstraint( const gposy & left, const gposy & right,
                         const std::string & );

   bool addEqConstraint( const gposy & left, const gposy & right ) {
      return addEqConstraint( left,right,"" );
   }

   bool addEqConstraint( const gposy & left, const gposy & right,
                         const std::string & );

   void addInConstraints
   ( const std::vector<const gposy *> & lefts,
     const std::vector<const gposy *> & rights,
     const std::vector<std::string> & labels );

   void addEqConstraints
   ( const std::vector<const gposy *> & lefts,
     const std::vector<const gposy *> & rights,
     const std::vector<std::string> & labels );

   // getters
   int getNumberOfInConstraints() const {
      return inCnstLeft.size();
   }
   int getNumberOfEqConstraints() const {
      return eqCnstLeft.size();
   }
   int getNumberOfMonomials() const {
      assert( hasCnstPosysUpToDate );
      int num = objFunc.getNumberOfMonomials();
      for( int i = 0; i < inCnstPosys.size(); i ++ )
         num += inCnstPosys[i]->getNumberOfMonomials();

      num += 2*eqCnstPosys.size();

      return num;
   }

   int getInMaxLabelLength() {
      if( hasInMaxLabelLengthUpToDate ) return maxInLabelLength;

      for( int i = numberOfInLabelsChecked; i < inCnstLabels.size(); i ++ )
         if( inCnstLabels[i].length() > maxInLabelLength )
            maxInLabelLength = inCnstLabels[i].length();

      hasInMaxLabelLengthUpToDate = true;
      numberOfInLabelsChecked = inCnstLabels.size();

      return maxInLabelLength;
   }

   int getEqMaxLabelLength() {
      if( hasEqMaxLabelLengthUpToDate ) return maxEqLabelLength;

      for( int i = numberOfEqLabelsChecked; i < eqCnstLabels.size(); i ++ )
         if( eqCnstLabels[i].length() > maxEqLabelLength )
            maxEqLabelLength = eqCnstLabels[i].length();

      hasEqMaxLabelLengthUpToDate = true;
      numberOfEqLabelsChecked = eqCnstLabels.size();

      return maxEqLabelLength;
   }

   void updateMaxLabelLengths() {
      getInMaxLabelLength();
      getEqMaxLabelLength();
   }

   // print/ostream methods
   void print() {
      print( 0 );
   }
   void print( int num ) {
      updateMaxLabelLengths();
      toOstream( cout, num );
   }

   void print( const std::string & x ) {
      print( x,0 );
   }
   void print( const std::string & x, int num ) {
      updateMaxLabelLengths();
      toOstream( cout, x, num );
   }

   bool toFile( const std::string & );

   ostream & toOstream( ostream & os ) {
      updateMaxLabelLengths();
      return toOstream( os,0 );
   }

   ostream & toOstream( ostream & os, int num ) const {
      return toOstream( os, num, true );
   }

   ostream & toOstream( ostream & os, int num, bool sorted ) const;

   // deprecated
   ostream & toOstream( ostream & os, const std::string & x, int num ) const;

   // askers
   bool isGP() const {
      return isgp;
   }

   // convert to gp
   ggp & toGP();

   // simeple matlab file
   bool toMatlabFile( const std::string & fileName );

   // mosek related methods
   void solveUsingMOSEKdgopt( const std::string &, ggp & );
   void writeDgoptFormat( const std::string & ) const;
   ostream & dgoptOptimalOutputToOstream
   ( ostream & os, double p_obj, double d_obj, const ggp & GGP ) const {
      return dgoptOptimalOutputToOstream( os,p_obj,d_obj,GGP,true );
   }
   ostream & dgoptOptimalOutputToOstream
   ( ostream & os, double p_obj, double d_obj, const ggp & GGP, bool sorted )
   const;
   ostream & dgoptOptimalVarMatlabToOstream
   ( ostream & os, double p_obj, double d_obj, const ggp & GGP ) const;

   void assignPrimalValues( const double * );
   void assignDualValues( const double * );
   void evaluateCnstValues( const double * values, unsigned size );

   /*
   void writeMSKgpoptFormat( const std::string & );
   void makeMatlabFile( const std::string & );
   */

};

#endif
