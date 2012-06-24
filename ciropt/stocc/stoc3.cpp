/**************************** STOC3.CPP *********************** 2002-10-02 AF *
*
* Non-uniform random number generator functions.
*
* This file contains source code for the class StochasticLib3 derived
* from StochasticLib or StochasticLib2, defined in stocc.h.
*
* This class implements methods for sampling from the noncentral and extended
* hypergeometric distributions, as well as the multivariate versions of these.
*
* Documentation:
* ==============
* The file stocc.h contains class definitions.
* The file stocc.htm contains further instructions.
* The file nchyp.pdf, available from www.agner.org/random/theory contains
* theoretical description of the noncentral and extended hypergeometric
* distributions and the methods used in this code to sample from these.
*
* © 2002 Agner Fog. GNU General Public License www.gnu.org/copyleft/gpl.html
******************************************************************************/

#include <math.h>                      // math functions
#include <string.h>                    // memcpy function
#include "stocc.hpp"                   // class definitions
#include "nchyppro.cpp"                // calculate noncentral hypergeometric probability
#include "xhyppro.cpp"                 // calculate extended hypergeometric probability


/******************************************************************************
         Methods for class StochasticLib3
******************************************************************************/


/***********************************************************************
             Constructor
***********************************************************************/
StochasticLib3::StochasticLib3( int seed ) : StochasticLib( seed ) {
   SetAccuracy( 1.E-8 );
}  // set default accuracy


/***********************************************************************
             SetAccuracy
***********************************************************************/
void StochasticLib3::SetAccuracy( double accur ) {
   // define accuracy of calculations for
   // NonCentralHypergeometric and MultiNonCentralHypergeo
   if ( accur < 0. ) accur = 0.;
   if ( accur > 0.01 ) accur = 0.01;
   accuracy = accur;
}


/***********************************************************************
         Non-central Hypergeometric distribution
***********************************************************************/

long int StochasticLib3::NonCentralHypergeometric ( long int n, long int m, long int N, double odds ) {
   /*
      This function generates a random variate with the noncentral
      hypergeometric distribution. Note, that the same name is sometimes used
      for the extended hypergeometric distribution.

      The noncentral hypergeometric distribution is the distribution you get when
      drawing balls without replacement from an urn containing red and white balls,
      with bias.

      We define the weight of the balls so that the probability of taking a
      particular ball is proportional to its weight. The value of odds is the
      normalized odds ratio: odds = weight(red) / weight(white).
      If all balls have the same weight, i.e. odds = 1, then we get the
      hypergeometric distribution.

      n is the number of balls you take,
      m is the number of red balls in the urn,
      N is the total number of balls in the urn,
      odds is the odds ratio,
      and the return value is the number of red balls you get.

      Four different calculation methods are implemented. This function decides
      which method to use, based on the parameters.
   */

   // check parameters
   if ( n >= N || m >= N || n <= 0 || m <= 0 || odds <= 0. ) {
      // trivial cases
      if ( n == 0 || m == 0 ) return 0;
      if ( m == N ) return n;
      if ( n == N ) return m;
      if ( odds == 0. ) {
         if ( n > N-m ) FatalError( "Not enough items with nonzero weight in function NonCentralHypergeometric" );
         return 0;
      }
      // illegal parameter
      FatalError( "Parameter out of range in NonCentralHypergeometric function" );
   }

   if ( odds == 1. ) {
      // use hypergeometric function if odds == 1
      return Hypergeometric( n, m, N );
   }

   if ( n < 30 ) {
      return NoncentralHypergeometricUrn( n, m, N, odds );
   }

   if ( double( n )*N < 10000 ) {
      return NoncentralHypergeometricTable( n, m, N, odds );
   }

   return NoncentralHypergeometricRatioOfUnifoms( n, m, N, odds );
   // the decision to use NoncentralHypergeometricInversion is
   // taken inside NoncentralHypergeometricRatioOfUnifoms based
   // on the calculated variance.
}


/***********************************************************************
         Subfunctions for non-central Hypergeometric distribution
***********************************************************************/

long int StochasticLib3::NoncentralHypergeometricUrn ( long int n, long int m, long int N, double odds ) {
   // sampling from noncentral hypergeometric distribution
   // by simulating urn model
   long int x;                          // sample
   long int m2;                         // items of color 2 in urn
   double mw1, mw2;                     // total weight of balls of color 1 or 2
   x = 0;
   m2 = N - m;
   mw1 = m * odds;
   mw2 = m2;
   do {
      if ( Random() * ( mw1 + mw2 ) < mw1 ) {
         x++;
         m--;
         if ( m == 0 ) break;
         mw1 = m * odds;
      } else {
         m2--;
         if ( m2 == 0 ) {
            x += n-1;
            break;
         }
         mw2 = m2;
      }
   } while ( --n );
   return x;
}


long int StochasticLib3::NoncentralHypergeometricTable ( long int n, long int m, long int N, double odds ) {
   // Sampling from noncentral hypergeometric distribution
   // using chop-down search from a table created by recursive calculation.
   // This method is fast when n is low or when called repeatedly with
   // the same parameters.
   static long int nch_n_last = -1, nch_m_last = -1, nch_N_last = -1; // previous parameters
   static double nch_o_last = -1;

   const int TABLELENGTH = 512;         // max length of table
   static double ytable[TABLELENGTH];   // table of probability values
   static long int len;                 // length of table
   static long int x1;                  // lower x limit for table
   long int x2;                         // upper x limit for table
   long int x;                          // sample
   double u;                            // uniform random number
   int success;                         // table long enough

   if ( n != nch_n_last || m != nch_m_last || N != nch_N_last || odds != nch_o_last ) {
      // set-up: This is done only when parameters have changed
      nch_n_last = n;
      nch_m_last = m;
      nch_N_last = N;
      nch_o_last = odds;

      NonCentralHypergeometricC nch( n,m,N,odds ); // make object for calculation
      success = nch.MakeTable( ytable, TABLELENGTH, &x1, &x2 ); // make table of probability values
      if ( success ) {
         len = x2 - x1 + 1;
      }              // table long enough. remember length
      else {
         len = 0;
      }
   }                       // remember failure

   if ( len == 0 ) {
      // table not long enough. Use another method
      return NoncentralHypergeometricRatioOfUnifoms( n,m,N,odds );
   }

   while ( 1 ) {                        // repeat in the rare case of failure

      u = Random();                      // uniform variate to convert
      for ( x=0; x<len; x++ ) {          // chop-down search
         u -= ytable[x];
         if ( u < 0. ) return x + x1;
      }
   }
}    // value found


long int StochasticLib3::NoncentralHypergeometricRatioOfUnifoms ( long int n, long int m, long int N, double odds ) {
   // sampling from noncentral hypergeometric distribution
   // using ratio-of-uniforms rejection method.
   static long int nch_n_last = -1, nch_m_last = -1, nch_N_last = -1; // previous parameters
   static double nch_o_last = -1;
   static long int nch_bound1, nch_bound2;// lower and upper bound
   static double nch_a;                 // hat center
   static double nch_h;                 // hat width
   static double nch_k;                 // probability value at mode
   static int UseChopDown;              // use chop down inversion instead
   long int mode;                       // mode
   long int xmin, xmax;                 // x limits
   double mean;                         // mean
   double variance;                     // variance
   double x;                            // real sample
   long int xi;                         // integer sample
   double u;                            // uniform random
   double f, f2;                        // probability function value
   double s123;                         // components 1,2,3 of hat width
   double s4;                           // component 4 of hat width
   double r1, r2;                       // temporaries
   static const double rsqrt2pi = 0.3989422804014326857; // 1/sqrt(2*pi)

   // Make object for calculating mean and probability.
   NonCentralHypergeometricC nch( n, m, N, odds, accuracy );

   xmin = m+n-N;
   if ( xmin<0 ) xmin = 0; // calculate limits
   xmax = n;
   if ( xmax>m ) xmax = m;

   if ( n != nch_n_last || m != nch_m_last || N != nch_N_last || odds != nch_o_last ) {
      // set-up: This is done only when parameters are changed
      nch_n_last = n;
      nch_m_last = m;
      nch_N_last = N;
      nch_o_last = odds;

      // find approximate mean
      mean = nch.mean();

      // find approximate variance from extended hypergeometric approximation
      r1 = mean * ( m-mean );
      r2 = ( n-mean )*( mean+N-n-m );
      variance = N*r1*r2/( ( N-1 )*( m*r2+( N-m )*r1 ) );
      UseChopDown = variance < 5.;       // use chop-down method if variance is low

      if ( !UseChopDown ) {
         // find mode
         mode = ( long int )( mean );
         f2 = 0.;
         if ( odds < 1. ) {
            if ( mode < xmax ) mode++;
            for ( xi = mode; xi >= xmin; xi-- ) {
               f = nch.probability( xi );
               if ( f <= f2 ) break;
               mode = xi;
               f2 = f;
            }
         } else {
            if ( mode < xmin ) mode++;
            for ( xi = mode; xi <= xmax; xi++ ) {
               f = nch.probability( xi );
               if ( f <= f2 ) break;
               mode = xi;
               f2 = f;
            }
         }
         nch_k = f2;                        // value at mode

         // find approximate variance from normal distribution approximation
         variance = rsqrt2pi / nch_k;
         variance *= variance;

         // find center and width of hat function
         nch_a = mean + 0.5;
         s123 = 0.40 + 0.8579*sqrt( variance+0.5 ) + 0.4*fabs( mean-mode );
         s4 = 0.;
         r1 = xmax - mean - s123;
         r2 = mean - s123 - xmin;
         if ( r1 > r2 ) r1 = r2;
         if ( ( odds>5. || odds<0.2 ) && r1>=-0.5 && r1<=8. ) {
            // s4 correction needed
            if ( r1 < 1. ) r1 = 1.;
            s4 = 0.029 * pow( N,0.23 ) / ( r1*r1 );
         }
         nch_h = 2. * ( s123 + s4 );

         // find safety bounds
         nch_bound1 = ( long int )( mean - 4. * nch_h );
         if ( nch_bound1 < xmin ) nch_bound1 = xmin;
         nch_bound2 = ( long int )( mean + 4. * nch_h );
         if ( nch_bound2 > xmax ) nch_bound2 = xmax;
      }
   }

   if ( UseChopDown ) { // for small variance, use chop down inversion
      return NoncentralHypergeometricInversion( n,m,N,odds );
   }

   while( 1 ) {
      u = Random();
      if ( u == 0. ) continue;                   // avoid division by 0
      x = nch_a + nch_h * ( Random()-0.5 )/u;
      if ( x < 0. || x > 2E9 ) continue;         // reject, avoid overflow
      xi = ( long int )( x );                    // truncate
      if ( xi < nch_bound1 || xi > nch_bound2 ) {
         continue;
      }                               // reject if outside safety bounds
      f = nch.probability( xi );                 // function value
      if ( f > nch_k * u * u ) {
         break;
      }                                  // acceptance
   }                                          // rejection
   return xi;
}


long int StochasticLib3::NoncentralHypergeometricInversion ( long int n, long int m, long int N, double odds ) {
   // sampling from noncentral hypergeometric distribution
   // using down-up search starting at the mean using the chop-down technique.
   // This method is faster than the rejection method when the variance is low.
   long int x1, x2;                     // search values
   long int xmin, xmax;                 // x limits
   double   u;                          // uniform random number to be converted
   double   f;                          // probability function value
   double   accura;                     // absolute accuracy
   int      updown;                     // 1 = search down, 2 = search up, 3 = both

   // Make objects for calculating mean and probability.
   // It is more efficient to have two identical objects, one for down search
   // and one for up search, because they are obtimized for consecutive x values.
   NonCentralHypergeometricC nch1( n, m, N, odds, accuracy );
   NonCentralHypergeometricC nch2( n, m, N, odds, accuracy );

   accura = accuracy * 0.01;
   if ( accura > 1E-7 ) accura = 1E-7;  // absolute accuracy

   x1 = ( long int )( nch1.mean() );    // start at floor and ceiling of mean
   x2 = x1 + 1;
   xmin = m+n-N;
   if ( xmin<0 ) xmin = 0; // calculate limits
   xmax = n;
   if ( xmax>m ) xmax = m;
   updown = 3;                          // start searching both up and down

   while( 1 ) {                         // loop until accepted (normally executes only once)
      u = Random();                      // uniform random number to be converted
      while ( updown ) {                 // search loop
         if ( updown & 1 ) {              // search down
            if ( x1 < xmin ) {
               updown &= ~1;
            }               // stop searching down
            else {
               f = nch1.probability( x1 );
               u -= f;                      // subtract probability until 0
               if ( u <= 0. ) return x1;
               x1--;
               if ( f < accura ) updown &= ~1;
            }
         } // stop searching down
         if ( updown & 2 ) {              // search up
            if ( x2 > xmax ) {
               updown &= ~2;
            }               // stop searching up
            else {
               f = nch2.probability( x2 );
               u -= f;                      // subtract probability until 0
               if ( u <= 0. ) return x2;
               x2++;
               if ( f < accura ) updown &= ~2;
            }
         }
      }
   }
} // stop searching down


/***********************************************************************
           Multivariate noncentral hypergeometric distribution
***********************************************************************/

void StochasticLib3::MultiNonCentralHypergeo ( long int * destination,
      long int * source, double * weights, long int n, int colors ) {
   /*
      This function generates a vector of random variables with the
      multivariate noncentral hypergeometric distribution.

      This distribution resembles the multivariate extended hypergeometric
      distribution and the two names are often confused, so make sure you
      get the right distribution.

      The multivariate noncentral hypergeometric distribution is the
      distribution you get when drawing colored balls from an urn
      with any number of colors, without replacement, and with bias.

      The weights are defined so that the probability of taking a particular
      ball is proportional to its weight.

      Parameters:
      destination:    An output array to receive the number of balls of each
                      color. Must have space for at least 'colors' elements.
      source:         An input array containing the number of balls of each
                      color in the urn. Must have 'colors' elements.
                      All elements must be non-negative.
      weights:        The odds of each color. Must have 'colors' elements.
                      All elements must be non-negative.
      n:              The number of balls to draw from the urn.
                      Cannot exceed the total number of balls with nonzero weight
                      in source.
      colors:         The number of possible colors.

      MAXCOLORS  (defined in stocc.h): You may adjust MAXCOLORS to the maximum
                 number of colors you need.

      The function will reduce the number of colors, if possible, by eliminating
      colors with zero weight or zero number and pooling together colors with the
      same weight. The problem thus reduced is handled in the arrays osource,
      urn, oweights and osample of size colors2.

      The sampling proceeds by either of two methods: simulating urn experiment,
      or conditional method followed by Metropolis-Hastings sampling.

      Simulating the urn experiment is simply taking one ball at a time, requiring
      n uniform random variates. The problem is reduced whenever a color has been
      exhausted.

      The conditional method divides the colors into groups where the number of
      balls in each group is determined by sampling from the marginal distribution
      which is approximated by the univariate noncentral hypergeometric distri-
      bution. Each group is then subdivided by sampling one color at a time until
      all colors have been sampled.

      The sample from the conditional method does not have the exact distribution,
      but it is used as a starting point for the Metropolis-Hastings sampling,
      which proceeds as follows: colors c1 and c2 are re-sampled using the
      univariate noncentral hypergeometric distribution, keeping the samples of
      all other colors constant. The new sample is accepted or the old sample
      retained, according to the Metropolis formula which corrects for the
      slight error introduced by not using the true conditional distribution.
      c1 and c2 are rotated in an order determined by the variance of each color.
      This rotation (scan) is repeated nHastings times.
   */

   // variables
   int order1[MAXCOLORS];       // sort order, index into source and destination
   int order2[MAXCOLORS];       // corresponding index into arrays when equal weights pooled together
   int order3[MAXCOLORS];       // secondary index for sorting by variance
   long int osource[MAXCOLORS]; // contents of source, sorted by weight with equal weights pooled together
   long int urn[MAXCOLORS];     // balls from osource not taken yet
   long int osample[MAXCOLORS]; // balls sampled
   double oweights[MAXCOLORS];  // sorted list of weights
   double wcum[MAXCOLORS];      // list of accumulated probabilities
   double var[MAXCOLORS];       // sorted list of variance
   double w;                    // weight of balls of one color
   double w1, w2;               // odds within group; mean weight in group
   double wsum;                 // total weight of all balls of several or all colors
   double p;                    // probability
   double f0, f1;               // multivariate probability function
   double g0, g1;               // conditional probability function
   double r1, r2;               // temporaries in calculation of variance
   long int nn;                 // number of balls left to sample
   long int m;                  // number of balls of one color
   long int msum;               // total number of balls of several or all colors
   long int N;                  // total number of balls with nonzero weight
   long int x0, x;              // sample of one color
   long int n1, n2, ng;         // size of weight group sample or partial sample
   long int m1, m2;             // size of weight group
   int i, j, k;                 // loop counters
   int c, c1, c2;               // color index
   int colors2;                 // reduced number of colors
   int a, b;                    // color index delimiting weight group
   int nHastings;               // number of scans in Metropolis-Hastings sampling

   // check validity of parameters
   if ( n < 0 || colors < 0 || colors > MAXCOLORS ) FatalError( "Parameter out of range in function MultiNonCentralHypergeo" );
   if ( colors == 0 ) return;
   if ( n == 0 ) {
      for ( i=0; i<colors; i++ ) destination[i] = 0;
      return;
   }

   // check validity of array parameters
   for ( i=0, msum=0; i < colors; i++ ) {
      m = source[i];
      w = weights[i];
      if ( m < 0 || w < 0 ) FatalError( "Parameter negative in function MultiNonCentralHypergeo" );
      if ( w ) msum += m;
   }
   N = msum;

   // sort colors by weight, heaviest first
   for ( i=0; i < colors; i++ ) order1[i] = order3[i] = i;
   for ( i=0; i < colors-1; i++ ) {
      c = order1[i];
      k = i;
      w = weights[c];
      if ( source[c]==0 ) w = 0; // zero number treated as zero weight
      for ( j=i+1; j < colors; j++ ) {
         c2 = order1[j];
         if ( weights[c2] > w && source[c2] ) {
            w = weights[c2];
            k = j;
         }
      }
      order1[i] = order1[k];
      order1[k] = c;
   }

   // skip any colors with zero weight or zero number.
   // this solves all problems with zero weights
   while ( colors && ( weights[c=order1[colors-1]]==0 || source[c]==0 ) ) {
      colors--;
      destination[c] = 0;
   }

   // check if there are more than n balls with nonzero weight
   if ( n >= N ) {
      if ( n > N ) FatalError( "Taking more items than there are in function MultiNonCentralHypergeo" );
      for ( i = 0; i < colors; i++ ) {
         c = order1[i];
         destination[c] = source[c];
      }
      return;
   }

   // copy source and weights into ordered lists
   // and pool together colors with same weight
   for ( i=0, c2=-1; i < colors; i++ ) {
      c = order1[i];
      if ( i==0 || weights[c] != w ) {
         c2++;
         x = source[c];
         oweights[c2] = w = weights[c];
      } else {
         x += source[c];
      } // join colors with same weight
      urn[c2] = osource[c2] = x;
      order2[i] = c2;
      osample[c2] = 0;
   }
   colors2 = c2 + 1;

   // check number of colors left
   if ( colors2 < 3 ) {
      // simple cases
      if ( colors2 == 1 ) osample[0] = n;
      if ( colors2 == 2 ) {
         x = NonCentralHypergeometric( n, osource[0], N, oweights[0]/oweights[1] );
         osample[0] = x;
         osample[1] = n - x;
      }
   } else {

      // more than 2 colors
      nn = n;

      // decide which method to use
      if ( nn < 5000 * colors2 ) {

         // Simulate urn experiment

         // Make list of accumulated probabilities of each color
         for ( i=0, wsum=0; i < colors2; i++ ) {
            wsum += urn[i] * oweights[i];
            wcum[i] = wsum;
         }

         // take one item nn times
         j = colors2-1;
         do {

            // get random color according to probability distribution wcum
            p = Random() * wcum[colors2-1];
            // get color from search in probability distribution wcum
            for ( i=0; i < j; i++ ) {
               if ( p < wcum[i] ) break;
            }

            // sample one ball of color i
            osample[i]++;
            urn[i]--;
            nn--;

            // check if this color has been exhausted
            if ( urn[i] == 0 ) {
               if ( i != j ) {
                  // put exhausted color at the end of lists so that colors2 can be reduced
                  m = osource[i];
                  osource[i] = osource[j];
                  osource[j] = m;
                  m = urn[i];
                  urn[i] = urn[j];
                  urn[j] = m;
                  m = osample[i];
                  osample[i] = osample[j];
                  osample[j] = m;
                  w = oweights[i];
                  oweights[i] = oweights[j];
                  oweights[j] = w;
                  // update order2 list (no longer sorted by weight)
                  for ( k=0; k<colors; k++ ) {
                     if ( order2[k] == i ) order2[k] = j;
                     else if ( order2[k] == j ) order2[k] = i;
                  }
               }
               colors2--;
               j = colors2-1;  // decrement number of colors left in urn

               if ( colors2 == 2 && nn > 50 ) {
                  // two colors left. use univariate distribution for the rest
                  x = NonCentralHypergeometric( nn, urn[0], urn[0]+urn[1], oweights[0]/oweights[1] );
                  osample[0] += x;
                  osample[1] += nn - x;
                  break;
               }

               if ( colors2 == 1 ) {
                  // only one color left. The rest is deterministic
                  osample[0] += nn;
                  break;
               }

               // make sure wcum is re-calculated from beginning
               i = 0;
            }

            // update list of accumulated probabilities
            wsum = i > 0 ? wcum[i-1] : 0.;
            for ( k=i; k<colors2; k++ ) {
               wsum += urn[k] * oweights[k];
               wcum[k] = wsum;
            }
         }

         while ( nn );
      }

      else {
         // use conditional method to make starting point for
         // Metropolis-Hastings sampling

         // divide weights into two groups, heavy and light
         a = 0;
         b = colors2-1;
         w = sqrt( oweights[0] * oweights[colors2-1] );
         do {
            c = ( a + b ) / 2;
            if ( oweights[c] > w ) a = c;
            else b = c;
         } while ( b > a + 1 );
         // heavy group goes from 0 to b-1, light group goes from b to colors2-1

         // calculate mean weight for heavy color group
         for ( i=0, m1=0, wsum=0; i < b; i++ ) {
            m1 += urn[i];
            wsum += oweights[i] * urn[i];
         }
         w1 = wsum / m1;

         // calculate mean weight for light color group
         for ( i=b, m2=0, wsum=0; i < colors2; i++ ) {
            m2 += urn[i];
            wsum += oweights[i] * urn[i];
         }
         w2 = wsum / m2;

         // split partial sample n into heavy (n1) and light (n2)
         n1 = NonCentralHypergeometric( n, m1, m1+m2, w1/w2 );
         n2 = n - n1;

         // set parameters for first group (heavy)
         a = 0;
         ng = n1;

         // loop twice, for the two groops
         for ( k=0; k < 2; k++ ) {

            // split group into single colors by calling univariate distribution b-a-1 times
            for ( i = a; i < b-1; i++ ) {
               m = urn[i];
               w = oweights[i];

               // calculate mean weight of remaining colors
               for ( j=i+1, msum=0, wsum=0; j < b; j++ ) {
                  m1 = urn[j];
                  w1 = oweights[j];
                  msum += m1;
                  wsum += m1 * w1;
               }

               // sample color i in group
               x = wsum ? NonCentralHypergeometric( ng, m, msum + m, w * msum / wsum ) : ng;

               osample[i] = x;
               ng -= x;
            }

            // get the last one in the group
            osample[i] = ng;

            // set parameters for second group (light)
            a = b;
            b = colors2;
            ng = n2;
         }

         // finished with conditional method.
         // osample contains starting point for Metropolis-Hastings sampling

         // make object for calculating probabilities and mean
         MultiNonCentralHypergeometricC mnc( n, osource, oweights, colors2 );

         mnc.mean( var ); // calculate mean
         // calculate variance from mean
         for ( i=0; i<colors; i++ ) {
            r1 = var[i] * ( osource[i]-var[i] );
            r2 = ( n-var[i] )*( var[i]+N-n-osource[i] );
            if ( r1 <= 0. || r2 <= 0. ) {
               var[i] = 0.;
            } else {
               var[i] = N*r1*r2/( ( N-1 )*( osource[i]*r2+( N-osource[i] )*r1 ) );
            }
         }

         // sort again, this time by variance
         for ( i=0; i < colors2-1; i++ ) {
            c = order3[i];
            k = i;
            w = var[c];
            for ( j=i+1; j < colors2; j++ ) {
               c2 = order3[j];
               if ( var[c2] > w ) {
                  w = var[c2];
                  k = j;
               }
            }
            order3[i] = order3[k];
            order3[k] = c;
         }

         // number of scans (this value of nHastings has not been fine-tuned)
         nHastings = 4;
         if ( accuracy < 1E-6 ) nHastings = 6;
         if ( colors2 > 5 ) nHastings++;

         // Metropolis-Hastings sampler
         f0 = -1.;
         for ( k = 0; k < nHastings; k++ ) {
            for ( i = 0; i < colors2; i++ ) {
               j = i+1;
               if ( j >= colors2 ) j = 0;
               c1 = order3[i];
               c2 = order3[j];
               w = oweights[c1] / oweights[c2];
               n1 = osample[c1] + osample[c2];
               x0 = osample[c1];
               x = NonCentralHypergeometric( n1, osource[c1], osource[c1]+osource[c2], w );
               if ( x == x0 ) continue; // accepted
               if ( f0 < 0. ) f0 = mnc.probability( osample );
               NonCentralHypergeometricC nc( n1, osource[c1], osource[c1]+osource[c2], w, accuracy );
               g0 = nc.probability( x0 );
               g1 = nc.probability( x );
               osample[c1] = x;
               osample[c2] = n1 - x;
               f1 = mnc.probability( osample );
               g0 = f1 * g0;
               g1 = f0 * g1;
               if ( g0 >= g1 || g0 > g1 * Random() ) {
                  // new state accepted
                  f0 = -1.;
               } else {
                  // rejected. restore old sample
                  osample[c1] = x0;
                  osample[c2] = n1 - x0;
               }
            }
         }
      }
   }

   // finished sampling by either method
   // un-sort sample into destination and untangle re-orderings
   for ( i=0; i < colors; i++ ) {
      c1 = order1[i];
      c2 = order2[i];
      if ( source[c1] == osource[c2] ) {
         destination[c1] = osample[c2];
      } else {
         // split colors with same weight that have been treated as one
         x = Hypergeometric( osample[c2], source[c1], osource[c2] );
         destination[c1] = x;
         osample[c2] -= x;
         osource[c2] -= source[c1];
      }
   }
}


/******************************************************************************
       Multivariate complementary noncentral hypergeometric distribution
******************************************************************************/

void StochasticLib3::MultiComplementaryNonCentralHypergeo (
   long int * destination, long int * source, double * weights, long int n, int colors ) {
   // This function generates a vector of random variables with the
   // multivariate complementary noncentral hypergeometric distribution.
   // See MultiNonCentralHypergeo for details.
   double rweights[MAXCOLORS];          // reciprocal weights
   long int sample[MAXCOLORS];          // balls sampled
   double w;                            // weight
   long int N;                          // total number of balls
   int i;                               // color index

   // make reciprocal weights and calculate N
   for ( i=0, N=0; i<colors; i++ ) {
      w = weights[i];
      if ( w == 0 ) FatalError( "Zero weight in function MultiComplementaryNonCentralHypergeo" );
      rweights[i] = 1. / w;
      N += source[i];
   }

   // use multivariate noncentral hypergeometric distribution
   MultiNonCentralHypergeo( sample, source, rweights, N - n, colors );

   // complementary distribution = balls not taken
   for ( i=0; i<colors; i++ ) {
      destination[i] = source[i] - sample[i];
   }
}


/******************************************************************************
                Extended hypergeometric distribution
******************************************************************************/
long int StochasticLib3::ExtendedHypergeometric ( long int n, long int m, long int N, double odds ) {
   /*
      This function generates a random variate with the extended hypergeometric
      distribution.

      This distribution resembles the noncentral hypergeometric distribution
      and the two distributions are sometimes confused. A more detailed
      explanation of this distribution is given below under the multivariate
      extended hypergeometric distribution (MultiExtendedHypergeo). For further
      documentation see nchyp.pdf, awailable from www.agner.org/random

      This function uses inversion by chop-down search from zero when parameters
      are small, and the ratio-of-uniforms rejection method when the former
      method would be too slow or would give overflow.
   */
   long int fak, addd;     // used for undoing transformations
   long int x;             // result

   // check if parameters are valid
   if ( n > N || m > N || n < 0 || m < 0 || odds <= 0. ) {
      if ( odds == 0. ) {
         if ( n > N-m ) FatalError( "Not enough items with nonzero weight in function ExtendedHypergeometric" );
         return 0;
      }
      FatalError( "Parameter out of range in function ExtendedHypergeometric" );
   }

   if ( odds == 1. ) {
      // use hypergeometric function if odds == 1
      return Hypergeometric ( n, m, N );
   }

   // symmetry transformations
   fak = 1;
   addd = 0;
   if ( m > N/2 ) {
      // invert m
      m = N - m;
      fak = -1;
      addd = n;
   }

   if ( n > N/2 ) {
      // invert n
      n = N - n;
      addd += fak * m;
      fak = - fak;
   }

   if ( n > m ) {
      // swap n and m
      x = n;
      n = m;
      m = x;
   }

   // cases with only one possible result end here
   if ( n == 0 || odds == 0. ) return addd;

   if ( fak == -1 ) {
      // reciprocal odds if inverting
      odds = 1. / odds;
   }

   // choose method
   if ( n < 30 && N < 1024 && odds > 1.E-5 && odds < 1.E5 ) {
      // use inversion by chop down method
      x = ExtendedHypergeometricInversion ( n, m, N, odds );
   }

   else {
      // use ratio-of-uniforms method
      x = ExtendedHypergeometricRatioOfUnifoms ( n, m, N, odds );
   }

   // undo symmetry transformations
   return x * fak + addd;
}


/***********************************************************************
             Subfunctions used by ExtendedHypergeometric
***********************************************************************/

long int StochasticLib3::ExtendedHypergeometricInversion
( long int n, long int m, long int N, double odds ) {
   /*
     Subfunction for ExtendedHypergeometric distribution.
     Implements extended hypergeometric distribution by inversion method,
     using chop-down search starting at zero.

     Valid only for 0 <= n <= m <= N/2.
     Without overflow check the parameters must be limited to n < 30, N < 1024,
     and 1.E-5 < odds < 1.E5. This limitation is acceptable because this method
     is slow for higher n.

     The execution time of this function grows with n.

     See the file nchyp.pdf for theoretical explanation.
   */
   static long int xh_n_last = -1, xh_m_last = -1, xh_N_last = -1;
   static double   xh_o_last = -1, xh_f0, xh_scale;

   long int x, i, j, k, L;
   double f, u, sum, f1, f2;
   L = N-m-n;

   if ( n != xh_n_last || m != xh_m_last || N != xh_N_last || odds != xh_o_last ) {
      // parameters have changed. set-up
      xh_n_last = n;
      xh_m_last = m;
      xh_N_last = N;
      xh_o_last = odds;

      // f(0) is set to an arbitrary value because it cancels out.
      // A low value is chosen to avoid overflow.
      xh_f0 = 1.E-100;

      // calculate summation of e(x), using the formula:
      // f(x) = f(x-1) * (m-x+1)*(n-x+1)*odds / (x*(L+x))
      // All divisions are avoided by scaling the parameters
      sum = f = xh_f0;
      xh_scale = 1.;
      i = m;
      j = n;
      k = L + 1;
      for ( x=1; x<=n; x++ ) {
         f1 = i * j * odds;
         f2 = x * k;
         i--;
         j--;
         k++;
         f *= f1;
         sum *= f2;
         xh_scale *= f2;
         sum += f;
         // overflow check. not needed if parameters are limited:
         // if (sum > 1E100) {sum *= 1E-100; f *= 1E-100; xh_scale *= 1E-100;}
      }
      xh_f0 *= xh_scale;
      xh_scale = sum;
      // now f(0) = xh_f0 / xh_scale.
      // We are still avoiding all divisions by saving the scale factor
   }

   // uniform random
   u = Random() * xh_scale;

   // recursive calculation:
   // f(x) = f(x-1) * (m-x+1)*(n-x+1)*odds / (x*(L+x))
   f = xh_f0;
   x = 0;
   i = m;
   j = n;
   k = L;
   do {
      u -= f;
      if ( u <= 0 ) break;
      x++;
      k++;
      f *= i * j * odds;
      u *= x * k;
      // overflow check. not needed if parameters are limited:
      // if (u > 1.E100) {u *= 1E-100;  f *= 1E-100;}
      i--;
      j--;
   } while ( x < n );
   return x;
}


long int StochasticLib3::ExtendedHypergeometricRatioOfUnifoms
( long int n, long int m, long int N, double odds ) {
   /*
     Subfunction for ExtendedHypergeometric distribution.
     Valid for 0 <= n <= m <= N/2, odds != 1

     Extended hypergeometric distribution by ratio-of-uniforms rejection method.

     The execution time of this function is almost independent of the parameters.
   */
   static long int xh_n_last = -1, xh_m_last = -1, xh_N_last = -1; // previous parameters
   static double xh_o_last = -1;
   static long int xh_bound;            // upper bound
   static double xh_a;                  // hat center
   static double xh_h;                  // hat width
   static double xh_lfm;                // ln(f(mode))
   static double xh_logb;               // ln(odds)
   long int L;                          // N-m-n
   long int mode;                       // mode
   double mean;                         // mean
   double variance;                     // variance
   double x;                            // real sample
   long int k;                          // integer sample
   double u;                            // uniform random
   double lf;                           // ln(f(x))
   double AA, BB, g1, g2;               // temporary

   L = N-m-n;

   if ( n != xh_n_last || m != xh_m_last || N != xh_N_last || odds != xh_o_last ) {
      // parameters have changed. set-up
      xh_n_last = n;
      xh_m_last = m;
      xh_N_last = N;
      xh_o_last = odds;

      // find approximate mean
      AA = ( m+n )*odds+L;
      BB = sqrt( AA*AA - 4*odds*( odds-1 )*m*n );
      mean = ( AA-BB )/( 2*( odds-1 ) );

      // find approximate variance
      AA = mean * ( m-mean );
      BB = ( n-mean )*( mean+L );
      variance = N*AA*BB/( ( N-1 )*( m*BB+( n+L )*AA ) );

      // compute log(odds)
      xh_logb = log( odds );

      // find center and width of hat function
      xh_a = mean + 0.5;
      xh_h = 1.028 + 1.717*sqrt( variance+0.5 ) + 0.032*fabs( xh_logb );

      // find safety bound
      xh_bound = ( long int )( mean + 4.0 * xh_h );
      if ( xh_bound > n ) xh_bound = n;

      // find mode
      mode = ( long int )( mean );
      g1 =( double )( m-mode )*( n-mode )*odds;
      g2 =( double )( mode+1 )*( L+mode+1 );
      if ( g1 > g2 && mode < n ) mode++;

      // value at mode to scale with:
      xh_lfm = mode * xh_logb - fc_lnpk( mode, L, m, n );
   }

   while( 1 ) {
      u = Random();
      if ( u == 0 ) continue;                     // avoid divide by 0
      x = xh_a + xh_h * ( Random()-0.5 )/u;
      if ( x < 0. || x > 2E9 ) continue;          // reject, avoid overflow
      k = ( long int )( x );                      // truncate
      if ( k > xh_bound ) continue;               // reject if outside safety bound
      lf = k*xh_logb - fc_lnpk( k,L,m,n ) - xh_lfm; // compute function value
      if ( u * ( 4.0 - u ) - 3.0 <= lf ) break;   // lower squeeze accept
      if ( u * ( u-lf ) > 1.0 ) continue;         // upper squeeze reject
      if ( 2.0 * log( u ) <= lf ) break;
   }             // final acceptance

   return k;
}


/***********************************************************************
           Multivariate extended hypergeometric distribution
***********************************************************************/
void StochasticLib3::MultiExtendedHypergeo ( long int * destination,
      long int * source, double * weights, long int n, int colors ) {
   /*
      This function generates a vector of random variates with the
      multivariate extended hypergeometric distribution.

      This distribution resembles the multivariate noncentral hypergeometric
      distribution and the two names are often confused, so make sure you
      get the right distribution.

      This distribution is defined as the conditional distribution of 'colors'
      independent binomial variates
         x[i] = binomial(source[i], p[i])
      on the condition that the sum of all x[i] is n.
      p[i] = r * weights[i] / (1 + r * weights[i]),
      r is an arbitrary scale factor.

      This distribution is not normally associated with the metaphor or taking
      colored balls from an urn, but here I will apply an urn model, however
      farfetched, for the sake of analogy with the noncentral hypergeometric
      distribution and in order to explain the transformations used for reducing
      the problem, so here it goes: You are taking n balls without replacement
      from an urn containing balls of different colors. The balls have different
      weights which make the sampling biased in favor or the heavier balls.
      Before taking the balls you assign to each ball a probability of being
      taken. These probabilities are calculated so that the expected total number
      of balls taken is n. Now you take or don't take each ball according to the
      assigned probabilities and count the total number of balls taken. If this
      number is not equal to n then put all the balls back in the urn and repeat
      the experiment. You may have to repeat this experiment many times before
      you have a sample containing exactly n balls.

      Parameters:
      destination:    An output array to receive the number of balls of each
                      color. Must have space for at least 'colors' elements.
      source:         An input array containing the number of balls of each
                      color in the urn. Must have 'colors' elements.
                      All elements must be non-negative.
      weights:        The odds of each color. Must have 'colors' elements.
                      All elements must be non-negative.
      n:              The number of balls drawn from the urn.
                      Can't exceed the total number of balls with nonzero weight
                      in the urn.
      colors:         The number of possible colors.

      Method: The conditional method is used for generating a sample with the
      approximate distribution. This sample is used as a starting point for
      a Gibbs sampler. The accuracy depends on the number of scans with the
      Gibbs sampler.

      The function will reduce the number of colors, if possible, by eliminating
      colors with zero weight or zero number and pooling together colors with the
      same weight. A symmetry transformation is used if more than half the balls
      are taken. The problem thus reduced is handled in the arrays osource,
      oweights and osample of dimension colors2.
   */
   int order1[MAXCOLORS];       // sort order, index into source and destination
   int order2[MAXCOLORS];       // corresponding index into osource when equal weights pooled together
   int order3[MAXCOLORS];       // secondary index for sorting by variance
   long int osource[MAXCOLORS]; // contents of source, sorted by weight with equal weights pooled together
   long int osample[MAXCOLORS]; // balls sampled, sorted by weight
   double oweights[MAXCOLORS];  // sorted list of weights
   double var[MAXCOLORS];       // sorted list of variance
   long int x;                  // univariate sample
   long int m;                  // number of items of one color
   long int m1, m2;             // number of items in each weight group
   long int msum;               // total number of items of several or all colors
   long int n0;                 // remaining balls to sample
   long int n1, n2;             // sample size for each weight group
   double w;                    // weight or variance of items of one color
   double w1, w2;               // mean weight of each weight group
   double wsum;                 // total weight of all items of several or all colors
   double odds;                 // weight ratio
   int i, j, k;                 // loop counters
   int a, b;                    // limits for weight group
   int c, c1, c2;               // color index
   int colors2;                 // reduced number of colors, number of entries in osource
   int ngibbs;                  // number of scans in Gibbs sampler
   int invert = 0;              // 1 if symmetry transformation used

   // check validity of parameters
   if ( n < 0 || colors < 0 || colors > MAXCOLORS ) FatalError( "Parameter out of range in function MultiExtendedHypergeo" );
   if ( colors == 0 ) return;
   if ( n == 0 ) {
      for ( i=0; i<colors; i++ ) destination[i] = 0;
      return;
   }

   // check validity of array parameters
   for ( i=0, msum=0; i < colors; i++ ) {
      m = source[i];
      w = weights[i];
      if ( m < 0 || w < 0 ) FatalError( "Parameter negative in function MultiExtendedHypergeo" );
      if ( w ) msum += m;
   }

   // sort by weight, heaviest first
   for ( i=0; i < colors; i++ ) order1[i] = order3[i] = i;
   for ( i=0; i < colors-1; i++ ) {
      c = order1[i];
      k = i;
      w = weights[c];
      if ( source[c]==0 ) w = 0;
      for ( j=i+1; j < colors; j++ ) {
         c2 = order1[j];
         if ( weights[c2] > w && source[c2] ) {
            w = weights[c2];
            k = j;
         }
      }
      order1[i] = order1[k];
      order1[k] = c;
   }

   // Skip any items with zero weight
   // this solves all problems with zero weights
   while ( colors && ( weights[c=order1[colors-1]]==0 || source[c]==0 ) ) {
      colors--;
      destination[c] = 0;
   }

   // check if we are taking all, or too many, balls
   if ( n >= msum ) {
      if ( n > msum ) FatalError( "Taking more items than there are in function MultiExtendedHypergeo" );
      for ( i = 0; i < colors; i++ ) {
         c = order1[i];
         destination[c] = source[c];
      }
      return;
   }

   if ( n > msum / 2 ) {
      // improve accuracy by symmetry transformation
      for ( i=0, j=colors-1; i < j; i++, j-- ) { // reverse order list
         c = order1[i];
         order1[i] = order1[j];
         order1[j] = c;
      }
      n = msum - n;
      invert = 1;
   }

   // copy source and weights into ordered lists and pool together colors with same weight
   for ( i=0, c2=-1; i < colors; i++ ) {
      c = order1[i];
      if ( i==0 || weights[c] != w ) {
         c2++;
         x = source[c];
         oweights[c2] = w = invert ? 1./weights[c] : weights[c];
      } else {
         x += source[c];
      }
      osource[c2] = x;
      order2[i] = c2;
      osample[c2] = 0;
   }
   colors2 = c2 + 1;

   // simple cases
   if ( colors2 == 1 ) osample[0] = n;
   if ( colors2 == 2 ) {
      x = ExtendedHypergeometric( n, osource[0], msum, oweights[0]/oweights[1] );
      osample[0] = x;
      osample[1] = n - x;
   }

   if ( colors2 > 2 ) {

      // divide weights into two groups, heavy and light
      a = 0;
      b = colors2-1;
      w = sqrt( oweights[0] * oweights[colors2-1] );
      do {
         c = ( a + b ) / 2;
         if ( oweights[c] > w ) a = c;
         else b = c;
      } while ( b > a + 1 );
      a = 0; // heavy group goes from a to b-1, light group goes from b to colors2-1

      // calculate mean weight for heavy group
      for ( i=a, m1=0, wsum=0; i < b; i++ ) {
         m1 += osource[i];
         wsum += oweights[i] * osource[i];
      }
      w1 = wsum / m1;

      // calculate mean weight for light group
      for ( i=b, m2=0, wsum=0; i < colors2; i++ ) {
         m2 += osource[i];
         wsum += oweights[i] * osource[i];
      }
      w2 = wsum / m2;

      // split sample n into heavy (n1) and light (n2) groups
      n1 = ExtendedHypergeometric( n, m1, m1+m2, w1/w2 );
      n2 = n - n1;
      n0 = n1;

      // loop twice, for the two groops
      for ( k=0; k < 2; k++ ) {

         // split group into single colors by calling ExtendedHypergeometric b-a-1 times
         for ( i = a; i < b-1; i++ ) {
            m = osource[i];
            w = oweights[i];

            // calculate mean weight of remaining colors
            for ( j=i+1, msum=0, wsum=0; j < b; j++ ) {
               m1 = osource[j];
               w1 = oweights[j];
               msum += m1;
               wsum += m1 * w1;
            }

            // split out color i
            if ( w == w1 ) {
               x = Hypergeometric( n0, m, msum + m );
            } else {
               if ( wsum == 0 ) {
                  x = n0;
               } else {
                  odds = w * msum / wsum;
                  x = ExtendedHypergeometric( n0, m, msum + m, odds );
               }
            }
            osample[i] += x;
            n0 -= x;
         }

         // get the last color in the group
         osample[i] += n0;

         // set parameters for second group
         a = b;
         b = colors2;
         n0 = n2;
      }

      // calculate variance
      MultiExtendedHypergeometricC( n, osource, oweights, colors2 ).variance( var );

      // sort again, this time by variance
      for ( i=0; i < colors2-1; i++ ) {
         c = order3[i];
         k = i;
         w = var[c];
         for ( j=i+1; j < colors2; j++ ) {
            c2 = order3[j];
            if ( var[c2] > w ) {
               w = var[c2];
               k = j;
            }
         }
         order3[i] = order3[k];
         order3[k] = c;
      }

      // determine number of scans (not fine-tuned):
      ngibbs = 4;
      if ( accuracy < 1E-6 ) ngibbs = 6;
      if ( colors2 > 5 ) ngibbs++;

      // Gibbs sampler
      for ( k = 0; k < ngibbs; k++ ) {
         for ( i = 0; i < colors2; i++ ) {
            c1 = order3[i];
            j = i + 1;
            if ( j == colors2 ) j = 0;
            c2 = order3[j];
            n1 = osample[c1] + osample[c2];
            x = ExtendedHypergeometric( n1, osource[c1], osource[c1]+osource[c2], oweights[c1]/oweights[c2] );
            osample[c1] = x;
            osample[c2] = n1 - x;
         }
      }
   }

   if ( invert ) {
      // reverse symmetry transformation on result
      for ( i=0; i < colors2; i++ ) {
         osample[i] = osource[i] - osample[i];
      }
   }

   // un-sort sample into destination
   for ( i=0; i < colors; i++ ) {
      c1 = order1[i];
      c2 = order2[i];
      if ( source[c1] == osource[c2] ) {
         destination[c1] = osample[c2];
      } else {
         x = Hypergeometric( osample[c2], source[c1], osource[c2] );
         destination[c1] = x;
         osample[c2] -= x;
         osource[c2] -= source[c1];
      }
   }
}
