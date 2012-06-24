/***************************** STOCC.H ************************* 2001-10-24 AF *
*
* This file contains function prototypes and class declarations for the C++
* library of non-uniform random number generators. Most functions are fast and
* accurate, even for extreme values of the parameters.
*
*
* functions without classes:
* ==========================
*
* void EndOfProgram(void);
* System-specific exit code. You may modify this to make it fit your
* user interface.
*
* void FatalError(char * ErrorText);
* Used for outputting error messages from the other functions and classes.
* You may have to modify this function to make it fit your user interface.
*
* double LnFac(long int n);
* Returns the natural logarithm of the factorial of n
*
*
* class StochasticLib:
* ====================
* This class can be derived from any of the uniform random number generators
* defined in randomc.h. StochasticLib provides the following non-uniform random
* variate generators:
*
* int Bernoulli(double p);
* Bernoulli distribution. Gives 0 or 1 with probability 1-p and p.
*
* double Normal(double m, double s);
* Normal distribution with mean m and standard deviation s.
*
* long int Poisson (double L);
* Poisson distribution with mean L.
*
* long int Binomial (long int n, double p);
* Binomial distribution. n trials with probability p.
*
* long int Hypergeometric (long int n, long int m, long int N);
* Hypergeometric distribution. Taking n items out N, m of which are colored.
*
* void Multinomial (long int * destination, double * source, long int n, int colors);
* void Multinomial (long int * destination, long int * source, long int n, int colors);
* Multivariate binomial distribution.
*
* void MultiHypergeo (long int * destination, long int * source, long int n, int colors);
* Multivariate hypergeometric distribution.
*
* void Shuffle(int * list, int min, int n);
* Shuffle a list of integers.
*
*
* class StochasticLib2:
* =====================
* This class is derived from class StochasticLib. It redefines the functions
* Poisson, Binomial and HyperGeometric.
* In StochasticLib, these functions are optimized for being called with
* parameters that vary. In StochasticLib2, the same functions are optimized
* for being called repeatedly with the same parameters. If your parameters
* seldom vary, then StochasticLib2 is faster. The two classes use different
* calculation methods, both of which are accurate.
*
*
* class StochasticLib3:
* =====================
* This class can be derived from either StochasticLib or StochasticLib2,
* whichever is preferred. It contains functions for generating variates with
* the univariate and multivariate noncentral and extended hypergeometric
* distributions.
*
* long int NonCentralHypergeometric (long int n, long int m, long int N, double odds);
* Sampling from noncentral hypergeometric distribution.
* Taking n items out N, m of which are colored, with bias.
*
* long int ExtendedHypergeometric (long int n, long int m, long int N, double odds);
* Sampling from extended hypergeometric distribution.
*
* void MultiNonCentralHypergeo (long int * destination, long int * source, double * weights, long int n, int colors);
* Sampling from multivariate noncentral hypergeometric distribution.
*
* void MultiExtendedHypergeo (long int * destination, long int * source, double * weights, long int n, int colors);
* Sampling from multivariate extended hypergeometric distribution.
*
*
* class NonCentralHypergeometricC:
* ================================
* This class implements various methods for calculating the probability
* function and the mean and variance of the noncentral hypergeometric distri-
* bution. It is used by StochasticLib3 and can also be used independently.
*
*
* class MultiNonCentralHypergeometricC
* ====================================
* This class implements various methods for calculating the probability func-
* tion and the mean of the multivariate noncentral hypergeometric distribution.
* It is used by StochasticLib3 and can also be used independently.
*
*
* class MultiNonCentralHypergeometricMoments
* ==========================================
* This class calculates the exact mean and variance of the multivariate
* noncentral hypergeometric probability distribution.
*
*
* class ExtendedHypergeometricC
* =============================
* This class calculates the probability function and the mean and variance
* of the extended hypergeometric distribution.
*
*
* MultiExtendedHypergeometricC
* ============================
* This class calculates the probability function and the mean and variance
* of the multivariate extended hypergeometric distribution.
*
* Uniform random number generators (integer or float) are also available, as
* these are inherited from the random number generator class that is the base
* class of StochasticLib.
*
*
* source code:
* ============
* The code for EndOfProgram and FatalError is found in the file userintf.cpp.
* The code for the functions in StochasticLib  is found in the file stoc1.cpp.
* The code for the functions in StochasticLib2 is found in the file stoc2.cpp.
* The code for the functions in StochasticLib3 is found in the file stoc3.cpp.
* The code for the functions in NonCentralHypergeometricC,
* MultiNonCentralHypergeometricC and MultiNonCentralHypergeometricMoments is
* found in the file nchyppro.cpp.
* The code for the functions in ExtendedHypergeometricC and
* MultiExtendedHypergeometricC is found in the file xhyppro.cpp
*
*
* Examples:
* =========
* The file ex-stoc.cpp contains an example of how to use this class library.
*
* The file ex-cards.cpp contains an example of how to shuffle a list of items.
*
* The file ex-lotto.cpp contains an example of how to generate a sequence of
* random integers where no number can occur more than once.
*
* The file testbino.cpp contains an example of sampling from the binomial distribution.
*
* The file testhype.cpp contains an example of sampling from the hypergeometric distribution.
*
* The file testpois.cpp contains an example of sampling from the poisson distribution.
*
* The file testnchy.cpp contains an example of sampling from the noncentral hypergeometric distribution.
*
* The file testxhy.cpp contains an example of sampling from the extended hypergeometric distribution.
*
* The file testmnch.cpp contains an example of sampling from the multivariate noncentral hypergeometric distribution.
*
* The file testmxh.cpp contains an example of sampling from the multivariate extended hypergeometric distribution.
*
* The file evolc.zip contains examples of how to simulate biological evolution using this class library.
*
*
* Documentation:
* ==============
* The file stocc.htm contains further instructions.
*
* The file distrib.pdf contains definitions of the standard statistic distributions:
* Bernoulli, Normal, Poisson, Binomial, Hypergeometric, Multinomial, MultiHypergeo.
*
* The file sampmet.pdf contains theoretical descriptions of the methods used
* for sampling from these distributions.
*
* The file nchyp.pdf, available from www.agner.org/random/, contains
* definitions of the univariate and multivariate extended and noncentral
* hypergeometric distributions and explanations of the methods for calculating
* and sampling from these.
*
* © 2002 Agner Fog. GNU General Public License www.gnu.org/copyleft/gpl.html
*******************************************************************************/

#ifndef STOCC_H
#define STOCC_H

#include "randomc.hpp"

/***********************************************************************
 Choose which uniform random number generator to base these classes on
***********************************************************************/

#ifndef RANDOM_GENERATOR
// define which random number generator to base stochastic library on:
#define RANDOM_GENERATOR TRandomMersenne
//#define RANDOM_GENERATOR TRanrotWGenerator
//#define RANDOM_GENERATOR TRandomMotherOfAll
#endif

/***********************************************************************
   Define mathematical constants
***********************************************************************/

#ifndef M_PI        // define mathematical constants if not defined in math.h
#define M_PI        3.14159265358979323846
#define M_LOG2E     1.44269504088896340736
#define M_LN2       0.693147180559945309417
#endif

/***********************************************************************
         System-specific user interface functions
***********************************************************************/

void EndOfProgram( void );             // system-specific exit code

void FatalError( const char * ErrorText );   // system-specific error reporting

/***********************************************************************
         Other simple functions
***********************************************************************/

double LnFac( long int n );            // log factorial
const int FAK_LEN = 1024;              // length of factorial table used by LnFac

/***********************************************************************
         Class StochasticLib
***********************************************************************/

class StochasticLib : public RANDOM_GENERATOR {
   // This class encapsulates the random variate generating functions.
   // May be derived from any of the random number generators.
public:
   StochasticLib ( int seed );          // constructor
   int Bernoulli( double p );           // bernoulli distribution
   double Normal( double m, double s ); // normal distribution
   long int Poisson ( double L );       // poisson distribution
   long int Binomial ( long int n, double p ); // binomial distribution
   long int Hypergeometric ( long int n, long int m, long int N ); // hypergeometric distribution
   void Multinomial ( long int * destination, double * source, long int n, int colors ); // multinomial distribution
   void Multinomial ( long int * destination, long int * source, long int n, int colors ); // multinomial distribution
   void MultiHypergeo ( long int * destination, long int * source, long int n, int colors ); // multivariate hypergeometric distribution
   void Shuffle( int * list, int min, int n ); // shuffle integers

   // functions used internally
protected:
   static double fc_lnpk( long int k, long int N_Mn, long int M, long int n ); // used by Hypergeometric

   // subfunctions for each approximation method
   long int PoissonInver( double L );               // poisson by inversion
   long int PoissonRatioUniforms( double L );       // poisson by ratio of uniforms
   long int PoissonLow( double L );                 // poisson for extremely low L
   long int BinomialInver ( long int n, double p ); // binomial by inversion
   long int BinomialRatioOfUniforms ( long int n, double p ); // binomial by ratio of uniforms
   long int HypInversionMod ( long int n, long int M, long int N ); // hypergeometric by inversion searching from mode
   long int HypRatioOfUnifoms ( long int n, long int M, long int N ); // hypergeometric by ratio of uniforms method

   // define constants
   enum constants {
      // maximum value of 'colors' in multivariate distributions:
      MAXCOLORS = 20
   };         // you may change this number
};


/***********************************************************************
         Class StochasticLib2
***********************************************************************/

class StochasticLib2 : public StochasticLib {
   // derived class, redefining some functions
public:
   long int Poisson ( double L );       // poisson distribution
   long int Binomial ( long int n, double p ); // binomial distribution
   long int Hypergeometric ( long int n, long int M, long int N ); // hypergeometric distribution
   StochasticLib2( int seed ):StochasticLib( seed ) {}; // constructor
   // subfunctions for each approximation method:
protected:
   long int PoissonModeSearch( double L ); // poisson by search from mode
   long int PoissonPatchwork( double L ); // poisson by patchwork rejection
   static double PoissonF( long int k, double l_nu, double c_pm ); // used by PoissonPatchwork
   long int BinomialModeSearch( long int n, double p ); // binomial by search from mode
   long int BinomialPatchwork( long int n, double p ); // binomial by patchwork rejection
   double BinomialF( long int k, long int n, double l_pq, double c_pm ); // used by BinomialPatchwork
   long int HypPatchwork ( long int n, long int M, long int N ); // hypergeometric by patchwork rejection
};


/***********************************************************************
         Class StochasticLib3
***********************************************************************/

class StochasticLib3 : public StochasticLib {
   // This class can be derived from either StochasticLib or StochasticLib2.
   // Adds more probability distributions
public:
   StochasticLib3( int seed ); // constructor
   void SetAccuracy( double accur ); // define accuracy of calculations
   long int NonCentralHypergeometric ( long int n, long int m, long int N, double odds ); // non-central hypergeometric distribution
   long int ExtendedHypergeometric ( long int n, long int m, long int N, double odds ); // extended hypergeometric distribution
   void MultiNonCentralHypergeo ( long int * destination, long int * source, double * weights, long int n, int colors ); // multivariate noncentral hypergeometric distribution
   void MultiComplementaryNonCentralHypergeo ( long int * destination, long int * source, double * weights, long int n, int colors ); // multivariate complementary noncentral hypergeometric distribution
   void MultiExtendedHypergeo ( long int * destination, long int * source, double * weights, long int n, int colors ); // multivariate extended hypergeometric distribution
   // subfunctions for each approximation method
protected:
   long int NoncentralHypergeometricUrn ( long int n, long int m, long int N, double odds ); // noncentral hypergeometric by urn model
   long int NoncentralHypergeometricInversion ( long int n, long int m, long int N, double odds ); // noncentral hypergeometric by urn model
   long int NoncentralHypergeometricTable ( long int n, long int m, long int N, double odds ); // noncentral hypergeometric by urn model
   long int NoncentralHypergeometricRatioOfUnifoms ( long int n, long int m, long int N, double odds ); // noncentral hypergeometric by urn model
   long int ExtendedHypergeometricInversion ( long int n, long int m, long int N, double odds ); // extended hypergeometric by inversion
   long int ExtendedHypergeometricRatioOfUnifoms ( long int n, long int m, long int N, double odds ); // extended hypergeometric by ratio-of-uniforms
   double accuracy; // accuracy of calculations
};


/***********************************************************************
         Class NonCentralHypergeometricC
***********************************************************************/

class NonCentralHypergeometricC {
   // This class contains methods for calculating the univariate
   // noncentral hypergeometric probability function
public:
   NonCentralHypergeometricC( long int n, long int m, long int N, double odds, double accuracy=1.E-8 ); // constructor
   double probability( long int x ); // calculate probability function
   int MakeTable( double * table, long int MaxLength, long int * start, long int * end ); // make table of probabilities
   double mean( void );       // calculate approximate mean
   double moments( double * mean, double * var ); // calculate exact mean and variance

   // implementations of different calculation methods
protected:
   double recursive( void );  // recursive calculation
   double binoexpand( void ); // binomial expansion of integrand
   double laplace( void );    // Laplace's method with narrow integration interval
   double integrate( void );  // numerical integration

   // other subfunctions
   double lnbico( void );     // natural log of binomial coefficients
   void findpars( void );     // calculate r, w, E
   double integrate_step( double a, double b ); // used by integrate()
   double search_inflect( double t_from, double t_to ); // used by integrate()

   // parameters
   double omega;
   double accuracy;
   long int n, m, N, x;
   long int xmin, xmax;
   // parameters used by lnbico
   double bico, mFac, xFac;
   int ParametersChanged;
   long int xLast;
   // parameters generated by findpars and used by probability, laplace, integrate:
   double r, rd, w, E, phi2d;
};


/***********************************************************************
         Class MultiNonCentralHypergeometricC
***********************************************************************/

class MultiNonCentralHypergeometricC {
   // This class encapsulates the different methods for calculating the
   // multivariate noncentral hypergeometric probability function
public:
   enum constants {
      MAXCOLORS = 20
   };                   // maximum number of colors. May be changed
   MultiNonCentralHypergeometricC( long int n, long int * m, double * odds, int colors, float accuracy=1.E-8f ); // constructor
   double probability( long int * x );  // calculate probability function
   void mean( double * mu );            // calculate approximate mean

   // implementations of different calculation methods
protected:
   double binoexpand( void ); // binomial expansion of integrand
   double laplace( void );    // Laplace's method with narrow integration interval
   double integrate( void );  // numerical integration

   // other subfunctions
   double lnbico( void );     // natural log of binomial coefficients
   void findpars( void );     // calculate r, w, E
   double integrate_step( double a, double b ); // used by integrate()
   double search_inflect( double t_from, double t_to ); // used by integrate()

   // parameters
   double * omega;
   float accuracy;
   long int n, N;
   long int * m, * x;
   int colors;
   // parameters generated by findpars and used by probability, laplace, integrate:
   double r, rd, w, E, phi2d;
   // generated by lnbico
   double bico;
};


/***********************************************************************
         Class MultiNonCentralHypergeometricMoments
***********************************************************************/

class MultiNonCentralHypergeometricMoments: public MultiNonCentralHypergeometricC {
   // This class calculates the exact mean and variance of the multivariate
   // noncentral hypergeometric distribution by calculating all the
   // possible x-combinations with probability < accuracy
public:
   MultiNonCentralHypergeometricMoments( long int n, long int * m, double * odds, int colors, float accuracy=1.E-8 )
      : MultiNonCentralHypergeometricC( n, m, odds, colors, accuracy ) {};
   double moments( double * mean, double * stddev, long int * combinations = 0 );

protected:
   // functions used internally
   double loop( long int n, int c );    // recursive loops
   // data
   long int xi[MAXCOLORS];              // x vector to calculate probability of
   long int xm[MAXCOLORS];              // rounded approximate mean of x[i]
   long int remaining[MAXCOLORS];       // number of balls of color > c in urn
   double sx[MAXCOLORS];                // sum of x*f(x)
   double sxx[MAXCOLORS];               // sum of x^2*f(x)
   long int sn;                         // number of combinations
};


/***********************************************************************
         Class ExtendedHypergeometricC
***********************************************************************/

class ExtendedHypergeometricC {
   // This class contains methods for calculating the univariate
   // extended hypergeometric probability function
public:
   ExtendedHypergeometricC( long int n, long int m, long int N, double odds ); // constructor
   double probability( long int x );    // calculate probability function
   double probabilityRatio( long int x, long int x0 ); // calculate probability f(x)/f(x0)
   double mean( void );                 // calculate approximate mean
   double moments( double * mean, double * var ); // calculate exact mean and variance

protected:
   double lng( long int x );            // natural log of proportional function

   // parameters
   double odds;                         // odds ratio
   double logodds;                      // ln odds ratio
   long int n, m, N;
   long int xmin, xmax;                 // minimum and maximum of x

   // parameters used by subfunctions
   long int xLast;
   double mFac, xFac;                   // log factorials
   double scale;                        // scale to apply to lng function
   double rsum;                         // reciprocal sum of proportional function
   int ParametersChanged;
};


/***********************************************************************
         Class MultiExtendedHypergeometricC
***********************************************************************/

class MultiExtendedHypergeometricC {
   // This class contains functions for calculating the multivariate
   // extended hypergeometric probability function and its mean and
   // variance. Warning: the time consumption for first call to
   // probability or moments is proportional to the total number of
   // possible x combinations, which may be extreme!
public:
   enum constants {
      MAXCOLORS = 16
   };                   // maximum number of colors. May be changed
   MultiExtendedHypergeometricC( long int n, long int * m, double * odds, int colors, float accuracy = 1E-9f ); // constructor
   double probability( long int * x );  // calculate probability function
   void mean( double * mu );            // calculate approximate mean
   void variance( double * var );       // calculate approximate variance
   double moments( double * mean, double * stddev, long int * combinations = 0 ); // calculate exact mean and variance

protected:
   double lng( long int * x );          // natural log of proportional function
   void SumOfAll( void );               // calculates sum of proportional function for all x combinations
   double loop( long int n, int c );    // recursive loops used by SumOfAll
   long int n, N;                       // copy of parameters
   long int * m;
   double * odds;
   int colors;
   double logodds[MAXCOLORS];           // log odds
   double mFac;                         // sum of log m[i]!
   double scale;                        // scale to apply to lng function
   double rsum;                         // reciprocal sum of proportional function
   float accuracy;                     // accuracy of calculation

   // data used by used by SumOfAll
   long int xi[MAXCOLORS];              // x vector to calculate probability of
   long int xm[MAXCOLORS];              // rounded approximate mean of x[i]
   long int remaining[MAXCOLORS];       // number of balls of color > c in urn
   double sx[MAXCOLORS];                // sum of x*f(x) or mean
   double sxx[MAXCOLORS];               // sum of x^2*f(x) or variance
   long int sn;                         // number of possible combinations of x
};

#endif
