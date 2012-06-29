// ggp.h : generalized GP

//#define DB

#include <fstream>

#include <iomanip.h>

#include "ggp.hpp"
#include "cnvt.hpp"
//#include "dgopt.h"
#include "d2p.h"

using namespace std;

#define MAX(x,y) (((x)>(y))? x:y)

#define WRITETOM false

const unsigned ggp::GP_TRANSFORM_NUMBER = 0;
const string ggp::OBJECTIVE_LABEL = " obj";
const string ggp::AUX_INEQ_LABEL_CONJ = "-";

ostream & operator<< ( ostream & os, const ggp & g ) {
   return g.toOstream( os,0 );
}


void ggp::updateCnstPosys() {
   if( hasCnstPosysUpToDate ) return;

   for( int i = inCnstPosys.size(); i < inCnstLeft.size(); i ++ ) {
      gposy & gp = *inCnstLeft[i] / *inCnstRight[i];
      //inCnstLeft[i]->print(); cout << endl;
      //inCnstRight[i]->print(); cout << endl;
      //gp.print(); cout << endl;
      inCnstPosys.push_back( & gp );
   }

   for( int i = eqCnstPosys.size(); i < eqCnstLeft.size(); i ++ )
      eqCnstPosys.push_back( & ( *eqCnstLeft[i] / *eqCnstRight[i] ) );

   hasCnstPosysUpToDate = true;
}

bool ggp::addInConstraint( const gposy & left, const gposy & right, const string & label ) {
   if( !right.isMonomial() ) return false;

   hasInSizeVecBeenMade = false;
   hasCnstPosysUpToDate = false;
   hasInMaxLabelLengthUpToDate = false;

   if( !left.isPosynomial() ) isgp = false;

   inCnstLeft.push_back( &left );
   inCnstRight.push_back( &right );

   assert( inCnstMap.find( label ) == inCnstMap.end() );
   inCnstMap[label] = inCnstLabels.size();
   inCnstLabels.push_back( label );

   return true;
}

bool ggp::addEqConstraint( const gposy & left, const gposy & right, const string & label ) {
   if( !( left.isMonomial() && right.isMonomial() ) ) return false;

   hasInSizeVecBeenMade = false;
   hasCnstPosysUpToDate = false;
   hasEqMaxLabelLengthUpToDate = false;

   eqCnstLeft.push_back( &left );
   eqCnstRight.push_back( &right );

   assert( eqCnstMap.find( label ) == eqCnstMap.end() );
   eqCnstMap[label] = eqCnstLabels.size();
   eqCnstLabels.push_back( label );

   return true;
}

void ggp::addInConstraints( const vector<const gposy *> & lefts, const vector<const gposy *> & rights, const vector<string> & labels ) {
   assert( lefts.size() == labels.size() && rights.size() == labels.size() );

   for( int i = 0; i < lefts.size(); i++ )
      addInConstraint( *lefts[i], *rights[i], labels[i] );
}

void ggp::addEqConstraints( const vector<const gposy *> & lefts, const vector<const gposy *> & rights, const vector<string> & labels ) {
   assert( lefts.size() == labels.size() && rights.size() == labels.size() );

   for( int i = 0; i < lefts.size(); i ++ )
      addEqConstraint( *lefts[i], *rights[i], labels[i] );
}

bool ggp::toFile( const string & filename ) {
   ofstream fout( filename.c_str(), ios::out );

   if( !fout.is_open() ) {
      cerr << "file open error: " << filename << endl;
      return false;
   }

   updateMaxLabelLengths();
   fout << *this;
   fout.close();

   return true;
}

ostream & ggp::toOstream( ostream & os, int num, bool sorted ) const {
   assert( hasInMaxLabelLengthUpToDate );
   assert( hasEqMaxLabelLengthUpToDate );

   switch( num ) {
   case 0:
      if( isGP() )
         os << "Geometric Program:" << endl;
      else
         os << "Generalized Geometric Program:" << endl;
      break;
   case 1:
      os << "Generalized Geometric Program:" << endl;
      break;
   case 2:
      assert( isGP() );
      os << "Converted Geometric Program:" << endl;
      break;
   default:
      assert( false );
   }

   os << " Number of Variables : " << symtab.getNumberOfSymbols() << endl;

   switch( num ) {
   case 0:
      if( isGP() )
         os << " Number of Auxiliary Variables : "
            << symtab.getNumberOfAuxSymbols() << endl;
      break;
   case 1:
      assert( symtab.getNumberOfAuxSymbols() == 0 );
      break;
   case 2:
      os << " Number of Auxiliary Variables : "
         << symtab.getNumberOfAuxSymbols() << endl;
      break;
   default:
      assert( false );
   }

   os << " Number of Inequality Constraints : " << inCnstLeft.size() << endl;
   os << " Number of Equality Constraints : " << eqCnstLeft.size() << endl;
   os << endl;

   os << "Objective Statement:" << endl << endl;
   objFunc.toOstream( os << "  Minimize ",symtab ) << endl << endl;

   if( inCnstLeft.size() != 0 ) {
      os << "Inequality Constraints:" << endl << endl;

      map<string,int>::const_iterator itr;
      int ind, i;
      for( itr = inCnstMap.begin(), ind = 0; itr != inCnstMap.end(); itr ++, ind ++ ) {
         i = sorted? itr->second:ind;

         if( inCnstLabels[i].length() != 0 )
            os <<cnvt::addSpaces( inCnstLabels[i]+":",maxInLabelLength+2 );
         else
            os << cnvt::addSpaces( " ",maxInLabelLength+2 );

         inCnstRight[i]->toOstream
         ( inCnstLeft[i]->toOstream( os,symtab ) << " < ",symtab ) << endl;
      }
      os << endl;
   }

   if( eqCnstLeft.size() != 0 ) {
      os << "Equality Constraints:" << endl << endl;

      map<string,int>::const_iterator itr;
      int i, ind;
      for( itr = eqCnstMap.begin(), ind = 0; itr != eqCnstMap.end(); itr ++, ind ++ ) {
         i = sorted? itr->second:ind;

         if( eqCnstLabels[i].length() != 0 )
            os <<cnvt::addSpaces( eqCnstLabels[i]+":",maxEqLabelLength+2 );
         else
            os << cnvt::addSpaces( " ",maxEqLabelLength+2 );

         eqCnstRight[i]->toOstream
         ( eqCnstLeft[i]->toOstream( os,symtab ) << " = ",symtab ) << endl;
      }
      os << endl;
   }

   return os;
}

/*
ostream & ggp::toOstream( ostream & os, const string & x, int num ) const
{
  assert( hasInMaxLabelLengthUpToDate );
  assert( hasEqMaxLabelLengthUpToDate );

  switch( num )
  {
    case 0:
      if( isGP() )
	os << "Geometric Program:" << endl;
      else
	os << "Generalized Geometric Program:" << endl;
      break;
    case 1:
      os << "Generalized Geometric Program:" << endl;
      break;
    case 2:
      assert( isGP() );
      os << "Converted Geometric Program:" << endl;
      break;
    default:
      assert( false );
  }

  os << " Number of Variables : " << symtab.getNumberOfSymbols() << endl;

  switch( num )
  {
    case 0:
      if( isGP() )
	os << " Number of Auxiliary Variables : "
	  << symtab.getNumberOfAuxSymbols() << endl;
      break;
    case 1:
      assert( symtab.getNumberOfAuxSymbols() == 0 );
      break;
    case 2:
      os << " Number of Auxiliary Variables : "
	<< symtab.getNumberOfAuxSymbols() << endl;
      break;
    default:
      assert( false );
  }

  os << " Number of Inequality Constraints : " << inCnstLeft.size() << endl;
  os << " Number of Equality Constraints : " << eqCnstLeft.size() << endl;
  os << endl;

  os << "Objective Statement:" << endl << endl;
  objFunc.toOstream(os << "  Minimize ",x) << endl << endl;

  if( inCnstLeft.size() != 0 )
  {
    os << "Inequality Constraints:" << endl << endl;

    for( int i = 0; i < inCnstLeft.size(); i ++ )
    {
      if( inCnstLabels[i].length() != 0 )
        os << " " << inCnstLabels[i].c_str() << ":" << endl;

      inCnstRight[i]->toOstream
      ( inCnstLeft[i]->toOstream( os << "  ", x ) << " < ", x ) << endl;
    }
    os << endl;
  }

  if( eqCnstLeft.size() != 0 )
  {
    os << "Equality Constraints:" << endl << endl;

    for( int i = 0; i < eqCnstLeft.size(); i ++ )
    {
      if( eqCnstLabels[i].length() != 0 )
        os << " " << eqCnstLabels[i].c_str() << ":" << endl;

      eqCnstRight[i]->toOstream
      ( eqCnstLeft[i]->toOstream( os << "  ", x ) << " = ", x ) << endl;
    }
    os << endl;
  }

  //symtab.toOstream( os ) << endl;

  return os;
}
*/

ggp & ggp::toGP() {
   vector<const gposy *> lefts, rights;
   vector<string> newLabels;
   symbol_table & newSymtab = * ( new symbol_table( symtab ) );

   const gposy & newObj = objFunc.toGP( newSymtab, newLabels, lefts, rights );
   ggp & gp = *( new ggp( newObj,newSymtab ) );

   gp.addInConstraints( lefts, rights, newLabels );
   lefts.clear();
   rights.clear();
   newLabels.clear();

   corrInCnstNum.clear();
   for( int i = 0; i < inCnstLeft.size(); i ++ ) {
      inCnstLeft[i]->toGP( newSymtab, newLabels, lefts, rights,
                           * inCnstRight[i], inCnstLabels[i] );

      corrInCnstNum.push_back( gp.getNumberOfInConstraints() );
      gp.addInConstraints( lefts, rights, newLabels );

      lefts.clear();
      rights.clear();
      newLabels.clear();
   }

   gp.addEqConstraints( eqCnstLeft, eqCnstRight, eqCnstLabels );

   return gp;
}

bool ggp::toMatlabFile( const string & fileName ) {
   
   assert( isGP() );

   if( getNumberOfEqConstraints() != 0 ) {
      return false;
   }

   makeSizeVec();
   updateCnstPosys();

   ofstream wrt( fileName.c_str(), ios::out );
   if( !wrt.is_open() ) {
      assert( false );
   }

   wrt << "% this is a matlab file generated by ggpsol " << endl;

   wrt << "szs = [ " << objFunc.getNumMonomials() << "; "<< endl;
   for( int i = 0; i < inSizeVec.size(); i ++ ) {
      wrt << "        " << inSizeVec[i] << ";" << endl;
   }
   wrt << "      ];" << endl << endl;

   wrt << "A = zeros( " << ( objFunc.getNumMonomials() + sumNumMono ) << ", " << symtab.getNumberOfSymbols() << " );" << endl;
   wrt << "b = zeros( " << ( objFunc.getNumMonomials() + sumNumMono ) << ", 1);" << endl;

   int rowNum = 0;
   objFunc.matlabAbToOstream( wrt, rowNum );

   for( int i = 0; i < inCnstPosys.size(); i ++ ) {
      inCnstPosys[i]->matlabAbToOstream( wrt, rowNum );
   }

   assert( rowNum == ( objFunc.getNumMonomials() + sumNumMono ) );
   wrt << endl;

   symtab.matlabSymbolNamesToOstream( wrt ) << endl;

   wrt << "%[x,nu,lambda] = gp(A,b,szs,x0);" << endl << endl;
   wrt.close();

   return true;
}

void ggp::solveUsingMOSEKdgopt( const string & fileName, ggp & GGP ) {
   assert( isGP() );
   updateCnstPosys();

   const string mpsFileName  = cnvt::changeExtension( fileName, "mps" );
   const string fFileName    = cnvt::changeExtension( fileName, "f"   );
   const string eoFileName   = cnvt::changeExtension( fileName, "eo"  );
   const string solFileName  = cnvt::changeExtension( fileName, "sol" );
   const string outFile      = cnvt::changeExtension( fileName, "out" );

   writeDgoptFormat( fileName );
   
   // const string TOLERANCE = "1e-4";
   // system( ("dgopt " + mpsFileName + " " + fFileName + " " + TOLERANCE).c_str() );
   
   const char * scotHomePtr    = getenv( "SCOT_HOME_DIR" );
   const string scotHome  = string( scotHomePtr );
   const string mpsToEo   = scotHome + "/lrep/mpsToEo.pl";
   const string priToDual = scotHome + "/lrep/fakePriToDual.pl";
   const string paramFile = scotHome + "/mosekparams/paramFile";
   
   const string mpsToEoCmd = mpsToEo + " " + mpsFileName + " " + fFileName + " " + eoFileName;
   const string rmMpsCmd   = "rm " + mpsFileName + " " + fFileName;
   const string mosekCmd   = "mskexpopt " + eoFileName + " -primal" + " -p " + paramFile + " -sol " + solFileName;
   
   system( mpsToEoCmd.c_str() );
   system( rmMpsCmd.c_str()   );
   system( mosekCmd.c_str()   );
   
   //system( ("mskexpopt " + eoFileName + " -primal").c_str());
   //system( ("mskexpopt " + eoFileName + " -primal" + " -p ~/research/dco/ggpsolexp/paramFile").c_str());
   //system( ("mskdgopt " + mpsFileName + " " + fFileName + " ~/research/dco/ggpsolexp/paramFile").c_str() );

   int numVar = symtab.getNumberOfSymbols();
   int numCon = getNumberOfMonomials();

   const string priToDualCmd = priToDual + " " + solFileName + " PrimalToDual.sol " + cnvt::toString( numCon );
   const string rmSolFileCmd = "rm " + solFileName;

   system( priToDualCmd.c_str() );
   system( rmSolFileCmd.c_str() );
   
   cout << "Processed " << solFileName << " to produce " << outFile << endl;

   double * primal_solution = new double[ numVar ];
   double p_obj;
   double d_obj;

   double * at_act  = new double[ numCon ];
   double * u_lower = new double[ numCon ];

   int status = dual2primal( "PrimalToDual.sol", numVar, numCon, primal_solution, at_act, u_lower, &p_obj, &d_obj );

   const string outFileName = cnvt::changeExtension( fileName, "out" );

   ofstream wrt( outFileName.c_str(), ios::out );
   
   if( !wrt.is_open() ) {
      assert( false );
   }

   switch( status ) {
   case  1:  // PRIMAL_AND_DUAL_FEASIBLE 1
      assignDualValues( at_act );
      GGP.assignPrimalValues( primal_solution );
      GGP.evaluateCnstValues( primal_solution, ( unsigned )numVar );

      GGP.updateMaxLabelLengths();
      dgoptOptimalOutputToOstream( wrt, p_obj, d_obj, GGP );
      break;

   case -1: //NOT_PRIMAL_AND_DUAL_FEASIBLE -1
      wrt << "Problem Status: not primal-dual feasible" << endl;
      break;

   case -2: //MEMORY_ALLOCATION_ERROR -2
      wrt << "Problem Status: memory allocation error" << endl;
      break;

   case -3: //GP_SOLVER_ERROR -3
      wrt << "Problem Status: gp solver error" << endl;
      break;

   case -4: //DGOPT_ERROR -4
      wrt << "Problem Status: dgopt error" << endl;
      break;

   default:
      assert( false );
   }
   wrt.close();

   // when optimum achieved, write the solution to .m file
   if( status == 1 && WRITETOM ) {
      const string mFileName = cnvt::changeExtension( fileName,"m" );
      ofstream wrtm( mFileName.c_str(), ios::out );
      if( !wrtm.is_open() ) assert( false );
      dgoptOptimalVarMatlabToOstream( wrtm, p_obj, d_obj, GGP );
      wrtm.close();
   }

   if( status != -4 ) {
      system( "rm PrimalToDual.sol" );
   }

   delete primal_solution;
   delete at_act;
   delete u_lower;
}

void ggp::writeDgoptFormat( const string & filename ) const {
   static const unsigned WIDTH = 10;

   ofstream wrt( cnvt::changeExtension( filename,"mps" ).c_str(), ios::out );
   if( !wrt.is_open() ) assert( false );

   wrt << "NAME" << endl;
   wrt << "ROWS" << endl;
   wrt << " N  obj" << endl;

   for( int i = 1; i <= symtab.getNumberOfSymbols()+1; i ++ )
      wrt << " E  c" << i << endl;

   wrt << "COLUMNS" << endl;

   int monoNum = 0;
   objFunc.mpsFormatToOstream( wrt, monoNum, symtab, false, true );

   for( int i = 0; i < inCnstPosys.size(); i++ )
      inCnstPosys[i]->mpsFormatToOstream( wrt, monoNum, symtab );

   for( int i = 0; i < eqCnstPosys.size(); i ++ ) {
      eqCnstPosys[i]->mpsFormatToOstream( wrt, monoNum, symtab );
      eqCnstPosys[i]->mpsFormatToOstream( wrt, monoNum, symtab, true );
   }

   wrt << "RHS" << endl;
   wrt << "    " << cnvt::addSpaces( "rhs", WIDTH )
       << cnvt::addSpaces( string( "c" )
                           + cnvt::intToString( symtab.getNumberOfSymbols()+1 ),WIDTH )
       << "1" << endl;

   wrt << "RANGES" << endl;
   wrt << "BOUNDS" << endl;
   wrt << "ENDATA" << endl;

   wrt.close();

   ofstream fwrt( cnvt::changeExtension( filename,"f" ).c_str(), ios::out );
   if( !fwrt.is_open() ) assert( false );

   fwrt << ( 1+getNumberOfInConstraints()+2*getNumberOfEqConstraints() ) << endl;

   objFunc.coefficientsToOstream( fwrt );
   for( int i = 0; i < inCnstPosys.size(); i ++ )
      inCnstPosys[i]->coefficientsToOstream( fwrt );

   for( int i = 0; i < eqCnstPosys.size(); i ++ ) {
      eqCnstPosys[i]->coefficientsToOstream( fwrt );
      eqCnstPosys[i]->coefficientsToOstream( fwrt, true );
   }

   fwrt << objFunc.getNumberOfMonomials() << endl;
   for( int i = 0; i < inCnstPosys.size(); i ++ )
      fwrt << inCnstPosys[i]->getNumberOfMonomials() << endl;

   for( int i = 0; i < eqCnstPosys.size(); i ++ )
      fwrt << "1" << endl << "1" << endl;

   fwrt.close();
}

ostream & ggp::dgoptOptimalOutputToOstream( ostream & os, double p_obj, double d_obj, const ggp & GGP, bool sorted ) const {
   assert( GGP.hasInMaxLabelLengthUpToDate );
   assert( GGP.hasEqMaxLabelLengthUpToDate );

   static const unsigned margin = 20;
   static const unsigned PREC = 3;
   static const unsigned numMargin = 15;

   os << cnvt::addSpaces( "Problem Status:",margin )
      << "primal-dual feasible" << endl;
   os << cnvt::addSpaces( "Solution Status:",margin )
      << "optimal" << endl;
   os << cnvt::addSpaces( "Primal Objective:",margin )
      << cnvt::doubleToString( exp( p_obj ),PREC )
      << ", log of: " << cnvt::doubleToString( p_obj,PREC ) << endl;
   os << cnvt::addSpaces( "Dual Objective:",margin )
      << cnvt::doubleToString( exp( d_obj ),PREC )
      << ", log of: " << cnvt::doubleToString( d_obj,PREC ) << endl << endl;

   os << "Optimal Objective Value: "
      << cnvt::doubleToString( GGP.objValue,PREC ) << endl << endl;

   os << "Optimal Variable Values: " << endl;
   map<string,int>::const_iterator itr;
   int i, ind;
   for( itr = GGP.symtab.getConstMapBegin(), ind = 0;
         itr != GGP.symtab.getConstMapEnd(); itr ++, ind ++ ) {
      i = sorted? itr->second: ind;

      os << cnvt::addSpaces( GGP.symtab[i],margin-3 ) << "   "
         << cnvt::doubleToString( GGP.optValues[i],PREC ) << endl;
   }
   os << endl;

   if( GGP.inCnstLabels.size() != 0 ) {
      os << "Inequality Constraints:" << endl;
      os << cnvt::addSpaces( "LABEL",MAX( GGP.maxInLabelLength,4 )+2 )
         << cnvt::addSpaces( "LHS",numMargin )
         << cnvt::addSpaces( "RHS",numMargin )
         << cnvt::addSpaces( "MARGIN",numMargin )
         << cnvt::addSpaces( "DUALMUL",numMargin )
         << endl;

      map<string,int>::const_iterator itr;
      int i;
      for( itr = GGP.inCnstMap.begin(), ind = 0;
            itr != GGP.inCnstMap.end(); itr ++, ind ++ ) {
         i = sorted? itr->second:ind;

         os << cnvt::addSpaces( GGP.inCnstLabels[i]+":",MAX( GGP.maxInLabelLength,4 )+2 );

         os << cnvt::doubleToString( GGP.inLefts[i],PREC,numMargin )
            << cnvt::doubleToString( GGP.inRights[i],PREC,numMargin )
            << cnvt::doubleToString( GGP.inMargins[i],PREC,numMargin )
            << cnvt::doubleToString( inDuals[GGP.corrInCnstNum[i]],PREC,numMargin )
            << endl;
      }
      os << endl;
   }

   assert( eqCnstLabels.size() == GGP.eqCnstLabels.size() );

   if( GGP.eqCnstLabels.size() != 0 ) {
      os << "Equality Constraints:" << endl;
      os << cnvt::addSpaces( "LABEL",MAX( GGP.maxEqLabelLength,4 )+2 )
         << cnvt::addSpaces( "LHS",numMargin )
         << cnvt::addSpaces( "RHS",numMargin )
         << cnvt::addSpaces( "MARGIN",numMargin )
         << cnvt::addSpaces( "DUALMUL",numMargin )
         << endl;

      map<string,int>::const_iterator itr;
      int i;
      for( itr = GGP.eqCnstMap.begin(), ind = 0;
            itr != GGP.eqCnstMap.end(); itr ++, ind ++ ) {
         i = sorted? itr->second:ind;

         os << cnvt::addSpaces( GGP.eqCnstLabels[i]+":",MAX( GGP.maxEqLabelLength,4 )+2 );

         os << cnvt::doubleToString( GGP.eqLefts[i],PREC,numMargin )
            << cnvt::doubleToString( GGP.eqRights[i],PREC,numMargin )
            << cnvt::doubleToString( GGP.eqMargins[i],PREC,numMargin )
            << cnvt::doubleToString( eqDuals[i],PREC,numMargin )
            << endl;
      }
      os << endl;
   }

   return os;
}

ostream & ggp::dgoptOptimalVarMatlabToOstream( ostream & os, double p_obj, double d_obj, const ggp & GGP ) const {
   assert( GGP.hasInMaxLabelLengthUpToDate );
   assert( GGP.hasEqMaxLabelLengthUpToDate );
   assert( eqCnstLabels.size() == GGP.eqCnstLabels.size() );

   static const unsigned PREC = 3;

   os << "% Optimal Variable Values: " << endl;

   map<string,int>::const_iterator itr;

   for( itr = GGP.symtab.getConstMapBegin();
         itr != GGP.symtab.getConstMapEnd(); itr ++ ) {
      int i = itr->second;
      os << GGP.symtab[i]
         << " = " << cnvt::doubleToString( GGP.optValues[i],PREC ) << ";" << endl;
   }
   os << endl;


   return os;
}

void ggp::assignPrimalValues( const double * primals ) {
   optValues.clear();

   for( int i = 0; i < symtab.getNumberOfSymbols(); i ++ )
      optValues.push_back( primals[i] );
}

void ggp::assignDualValues( const double * duals ) {
   assert( hasCnstPosysUpToDate );
   inDuals.clear();
   eqDuals.clear();

   int num = objFunc.getNumberOfMonomials();
   double dual;

   for( int i = 0; i < inCnstPosys.size(); i ++ ) {
      dual = 0.0;
      for( int j = 0; j < inCnstPosys[i]->getNumberOfMonomials(); j ++ )
         dual += duals[num++];
      inDuals.push_back( dual );
   }

   for( int i = 0; i < eqCnstPosys.size(); i ++ ) {
      eqDuals.push_back( duals[num]-duals[num+1] );
      num += 2;
   }
}

void ggp::evaluateCnstValues( const double * primals, unsigned size ) {
   inLefts.clear();
   inRights.clear();
   inMargins.clear();
   eqLefts.clear();
   eqRights.clear();
   eqMargins.clear();

   vector<double> pms;

   for( unsigned i = 0; i < size; i ++ ) pms.push_back( primals[i] );

   objValue = objFunc.evaluate( pms );

   double left, right;
   for( int i = 0; i < inCnstLabels.size(); i ++ ) {
      inLefts.push_back  ( ( left=inCnstLeft[i]->evaluate( pms ) ) );
      inRights.push_back ( ( right=inCnstRight[i]->evaluate( pms ) ) );
      inMargins.push_back( log( right/left ) );
   }

   for( int i = 0; i < eqCnstLabels.size(); i ++ ) {
      eqLefts.push_back  ( ( left=eqCnstLeft[i]->evaluate( pms ) ) );
      eqRights.push_back ( ( right=eqCnstRight[i]->evaluate( pms ) ) );
      eqMargins.push_back( log( right/left ) );
   }
}

/*
void ggp::writeMSKgpoptFormat( const string & filename, int cs )
{
  maxElimination( cs );

  ofstream wrt( cnvt::changeExtension(filename,"m").c_str(), ios::out );
  if( !wrt.is_open() ) assert(false);

  wrt << "% this is an output file of ggp parser" << endl;
  wrt << endl;
  wrt << "clear;" << endl;

  int rowNum = nomaxObjVec[cs].writeMSKgpoptFormat( wrt, 1, 0 );
  nomaxCnstsVec[cs].writeMSKgpoptFormat( wrt, rowNum );

  wrt << endl;

  wrt << "A=sparse(A);" << endl;
  wrt << "[res] = mskgpopt(c,A,map);" << endl;
  wrt << endl;

  wrt << "fprintf('\\nPrimal optimal solution to original gp:');" << endl;
  wrt << "fprintf(' %e',exp(res.sol.itr.yy));" << endl;
  wrt << "fprintf('\\n\\n');" << endl;
  wrt << endl;

  wrt << "% Compute the optimal objective value and" << endl;
  wrt << "% the constraint activities." << endl;
  wrt << "v = c.*exp(A*res.sol.itr.yy);" << endl;
  wrt << endl;

  wrt << "% Add appropriate terms together." << endl;
  wrt << "f = sparse(map+1,1:length(map),ones(size(map)))*v;" << endl;
  wrt << endl;

  wrt << "% First objective value. Then constraint values." << endl;
  wrt << "fprintf('Objective value: %e\\n',log(f(1)));" << endl;
  wrt << "fprintf('Constraint values:');" << endl;
  wrt << "fprintf(' %e',log(f(2:end)));" << endl;
  wrt << "fprintf('\\n\\n');" << endl;
  wrt << endl;

  wrt << "% Dual multipliers (should be negative)" << endl;
  wrt << "fprintf('Dual variables (should be negative):');" << endl;
  wrt << "fprintf(' %e',res.sol.itr.y);" << endl;
  wrt << "fprintf('\\n\\n');" << endl;
  wrt << endl;

  wrt.close();
}
*/
