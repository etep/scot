// probdist.h

#ifndef _SH_PROBDIST_H_
#define _SH_PROBDIST_H_

#include "stocc.hpp"

#include <iostream.h>
#include <string>
#include <vector>

class ProbDist;

class ProbDist {
public:
   static const unsigned ll;
   static const double plottingSmoothingFactor;
   static const double PLOTTING_HALF_WIDTH_IN_STD;

   static double getNormInv( double p );
   static double getNormPDF( double xx );
   static double getNormCDF( double xx );

   static double * linSpace( double s, double e, unsigned num );

private:
   // Gaussian \Phi^{-1}
   static const double norminv[];
   static const double norminvN;
   static const double norminv_precision;

   static const size_t numBins;
   static const double xinc;
   static const double normx[];
   static const double normpdf[];
   static const double normcdf[];

   // percentile point
   static const double ap[];
   static const unsigned apSize;

   // data type
   const bool byDist;
   const std::string type;

   // distribution
   bool   hasEvaluated;
   double mean, std;
   double * pctl;

   bool   hasXPCAssigned;
   double * x;
   double * pdf;
   double * cdf;

   // simulation
   static StochasticLib STO;
   const unsigned N;
   double * sim;

   // this (private) constructor just takes the double type pointer {sm}
   // and keeps it not making a separate copy.
   // therefore, once the client (or most likely implementer) passes {sm}
   // she/he must not change the contents of this double type array.
   // it'd be even better if she/he loses this pointer value,
   // since the memory will be freed when the destructor of the class instance
   // is called.
   ProbDist( double * sm, unsigned n );

public:
   ProbDist( const std::string & detVal );
   ProbDist( double detVal );
   ProbDist( const std::string & dist, unsigned n, double mn, double sd );
   ProbDist( const ProbDist & pd, double mn, double sd );

   ~ProbDist() {
      if( hasEvaluated )    delete pctl;
      if( hasXPCAssigned )  delete x, pdf, cdf;
      if( type != "fixed" ) delete sim;
   }

   // getters
   double getPercentilePoint( double a );

   inline unsigned getNumberOfSimPoints() const {
      return N;
   }

   inline double getSimPoint( unsigned i ) const {
      assert( i<N );
      return sim[i];
   }

   inline double getMean() const {
      return ( ( type=="uniform" )?( ( mean+std )/2.0 ):mean );
   }

   inline double getSTD() const {
      return ( ( type=="uniform" )?( ( std-mean )/sqrt( 12.0 ) ):std );
   }

   inline bool isByDist() const {
      return byDist;
   }

   inline const std::string & getDistType() const {
      assert( isByDist() );
      return type;
   }

   double * getAddedSimCopy( double ) const;

   inline size_t getNumberOfX() {
      assignXPC();
      return numBins;
   }

   inline const double * getX() {
      assignXPC();
      return x;
   }

   inline const double * getPDF() {
      assignXPC();
      return pdf;
   }

   inline const double * getCDF() {
      assignXPC();
      return cdf;
   }

   inline double getXMin() {
      assignXPC();
      return x[0];
   }

   inline double getXMax() {
      assignXPC();
      return x[numBins-1];
   }

   double getCDF( double x );

   double getPDF( double x );

   // operators
   ProbDist & operator+ ( const ProbDist & pd ) const;

   // ostream methods
   ostream &   xMatVecToOstream( ostream & os );
   ostream & pdfMatVecToOstream( ostream & os );
   ostream & cdfMatVecToOstream( ostream & os );
   ostream & cdfMatlabToOstream( ostream & os );

private:
   void montecarlo();
   void init();
   void evalMSP();
   void assignXPC();

public:
   // public static methods
   static void doubleVectorToSmoothedPDF
   ( std::vector<double> & x, std::vector<double> & y,
     const std::vector<double> & samples ) {
      doubleVectorToSmoothedPDF( x, y, samples, plottingSmoothingFactor );
   }

   static void doubleVectorToSmoothedPDF
   ( std::vector<double> & x, std::vector<double> & y,
     const std::vector<double> & samples, double smoothingFactor );

   static ProbDist & getEqualMixture( const ProbDist & a, const ProbDist & b ) {
      std::vector<const ProbDist *> pvec;
      pvec.push_back( &a );
      pvec.push_back( &b );
      return getEqualMixture( pvec );
   }

   static ProbDist & getEqualMixture( const std::vector<const ProbDist *> & ps );

   static ProbDist & getAddMaxProbDist
   ( const std::vector<ProbDist *> & ps1,
     const std::vector<ProbDist *> & ps2,
     std::vector<ProbDist *> & sums ) {
      return * createAddMaxProbDist( ps1, ps2, sums );
   }

   static ProbDist * createAddMaxProbDist
   ( const std::vector<ProbDist *> &, const std::vector<ProbDist *> &,
     std::vector<ProbDist *> & );

   static ProbDist & getMaxProbDist( const std::vector<ProbDist *> & ps ) {
      return * createMaxProbDist( ps );
   }

   static ProbDist * createMaxProbDist( const std::vector<ProbDist *> & pds ) {
      std::vector<unsigned> d;
      return createMaxProbDist( d,pds );
   }

   static ProbDist * createMaxProbDist
   ( std::vector<unsigned> & slctVec, const std::vector<ProbDist *> & );

   static ProbDist * createMaxProbDistwithCriticality
   ( std::vector<double> & slctVec, const std::vector<ProbDist *> & );

   static ProbDist * createLinCom
   ( const ProbDist * pd, double f1,
     const std::vector<ProbDist *> & pds, double f2 );

   static ostream & matlabToOstream( ostream & os,
                                     const std::vector<std::string> & ns,
                                     const std::vector<ProbDist *> & p1, const std::vector<ProbDist *> & p2,
                                     const std::vector<ProbDist *> &ap1, const std::vector<ProbDist *> &ap2,
                                     const std::vector<double> & ov1, const std::vector<double> & ov2,
                                     const std::string & s1, const std::string & s2 ) {
      return matlabToOstream( os,ns,p1,p2,ap1,ap2,ov1,ov2,s1,s2,"first" );
   }

   static ostream & matlabToOstream( ostream & os,
                                     const std::vector<std::string> & nms,
                                     const std::vector<ProbDist *> & p1, const std::vector<ProbDist *> & p2,
                                     const std::vector<ProbDist *> &ap1, const std::vector<ProbDist *> &ap2,
                                     const std::vector<double> & ov1, const std::vector<double> & ov2,
                                     const std::string & s1, const std::string & s2,
                                     const std::string & kind ) {
      return matlabToOstream( os,nms,p1,p2,ap1,ap2,ov1,ov2,s1,s2,kind,0.9 );
   }

   static ostream & matlabToOstream( ostream & os,
                                     const std::vector<std::string> & nms,
                                     const std::vector<ProbDist *> & p1, const std::vector<ProbDist *> & p2,
                                     const std::vector<ProbDist *> &ap1, const std::vector<ProbDist *> &ap2,
                                     const std::vector<double> & ov1, const std::vector<double> & ov2,
                                     const std::string & s1, const std::string & s2,
                                     const std::string & kind, double ap );

   static ostream & matlabToOstream( ostream & os,
                                     const std::vector<std::string> & nms,
                                     const std::vector<ProbDist *> & p1, const std::vector<ProbDist *> & p2,
                                     const std::vector<double> & ov1, const std::vector<double> & ov2,
                                     const std::string & s1, const std::string & s2,
                                     const std::string & kind, double ap );

private:
   static ostream & matlabToOstreamFirstKind( ostream & os,
         const std::vector<std::string> & nms,
         const std::vector<ProbDist *> & p1, const std::vector<ProbDist *> & p2,
         const std::vector<double> & ov1, const std::vector<double> & ov2,
         const std::string & s1, const std::string & s2, double p );

   static ostream & matlabToOstream( ostream &,
                                     const std::string &, ProbDist &, ProbDist &,
                                     double ov1, double ov2,
                                     const std::string &, const std::string &, double, bool cdf );

   static ostream & matlabToOstreamSecondKind( ostream &,
         const std::vector<std::string> &,
         const std::vector<ProbDist *> &, const std::vector<ProbDist *> &,
         const std::vector<double> & ov1, const std::vector<double> & ov2,
         const std::string &, const std::string &, double kappa );

   static ostream & matlabToOstream( ostream & os,
                                     const std::vector<std::string> & ns, const std::vector<ProbDist *> & ps,
                                     bool cdf, double kappa );

   static ostream & matlabToOstream( ostream & os, ProbDist & p1,
                                     bool cdf, double kappa,
                                     const std::string & clr, const std::string & ls, bool lineD );

   static ostream & matlabToOstreamThirdKind( ostream & os,
         const std::vector<std::string> & nms,
         const std::vector<ProbDist *> & p1, const std::vector<ProbDist *> & p2,
         const std::vector<ProbDist *> &ap1, const std::vector<ProbDist *> &ap2,
         const std::vector<double> & ov1, const std::vector<double> & ov2,
         const std::string & s1, const std::string & s2, double kappa );

   static ostream & matlabToOstream( ostream & os,
                                     const std::vector<std::string>& ns,
                                     const std::vector<ProbDist*>& ps, const std::vector<ProbDist*>& aps,
                                     const std::vector<double> & ovs, double kappa );

   static ostream & matlabToOstream( ostream & os,
                                     const std::vector<ProbDist*>& ps1, const std::vector<ProbDist*>& ps2,
                                     const std::vector<ProbDist*>&aps1, const std::vector<ProbDist*>&aps2,
                                     const std::string & s1, const std::string & s2 );

   static void getPDFCDF
   ( double * pdf, double * cdf, const double * x, unsigned num,
     const double * sim, unsigned N, double std );

   static inline double pdfSmoothing( double xx ) {
      return getNormPDF( xx );
   }
   static inline double cdfSmoothing( double xx ) {
      return getNormCDF( xx );
   }
};

#endif
