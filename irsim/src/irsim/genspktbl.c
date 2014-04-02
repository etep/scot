/*
 *     *********************************************************************
 *     * Copyright (C) 1988, 1990 Stanford University.                     *
 *     * Permission to use, copy, modify, and distribute this              *
 *     * software and its documentation for any purpose and without        *
 *     * fee is hereby granted, provided that the above copyright          *
 *     * notice appear in all copies.  Stanford University                 *
 *     * makes no representations about the suitability of this            *
 *     * software for any purpose.  It is provided "as is" without         *
 *     * express or implied warranty.  Export of this software outside     *
 *     * of the United States of America may require an export license.    *
 *     *********************************************************************
 */

/*
 * Program to generate the spike fluctuation and delay lookup tables.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define SPIKETBLSIZE    10
#define STEP        (1.0 / SPIKETBLSIZE)       /* Step of computation */

#define NLSPKMIN    0
#define NLSPKMAX    1
#define LINEARSPK   2


double  nldltab[ SPIKETBLSIZE + 1 ][ SPIKETBLSIZE + 1 ];
double  nldhtab[ SPIKETBLSIZE + 1 ][ SPIKETBLSIZE + 1 ];
double  linrtab[ SPIKETBLSIZE + 1 ][ SPIKETBLSIZE + 1 ];
double  delaytab[ SPIKETBLSIZE + 1 ][ SPIKETBLSIZE + 1 ];


#define SMALL   0.01
#define LARGE   0.99


FILE  *  F;

double nldh_rk4( double alpha, double beta, double h );
double nldl_rk4( double alpha, double beta, double h );
void rk4_error( char * which, double alpha, double beta, double h );
void linear_spike( double alpha, double beta, double * delay, double * peak );
void nldl_vector( double alpha, double beta, double h, double v1, double v2, double * k1, double * k2 );
void nldh_vector( double alpha, double beta, double h, double v1, double v2, double * k1, double * k2 );

void PrintTable( double tab[ SPIKETBLSIZE+1 ][ SPIKETBLSIZE+1 ], const char * fmt ) {
    register  int  alpha, beta;

    for( beta = 0; beta <= SPIKETBLSIZE; beta++ ) {
        if( beta == 0 ) {
            fprintf( F, "/* .01 */" );
        }
        else if( beta == SPIKETBLSIZE ) {
            fprintf( F, "/* .99 */" );
        }
        else {
            fprintf( F, "/* 0.%d */", beta );
        }

        for( alpha = 0; alpha <= SPIKETBLSIZE; alpha++ ) {
            fprintf( F, "  " );
            fprintf( F, fmt, tab[ beta ][ alpha ] );
            fprintf( F, "," );
            if( ( alpha + 1 ) % 4 == 0 ) {
                fprintf( F, "\n         " );
            }
        }
        fprintf( F, "\n" );
    }
}



/*
 * Peak voltage for a 2-transistors-2-capacitors network, as a function
 * of alpha and beta.  Compute spikes for nmos network driven to Gnd and
 * to Vdd, placing the results in nldltab and nldhtab respectively.
 *
 * Alpha is the ratio of resistance of the resistor closer to the driver to
 * the total resistance.
 * Beta is the ratio of capacitance of the capacitor closer to the driver to
 * the total capacitance.
 *
 */
void BuildTables() {
    double        alpha, beta, step;
    register int  i, j;


    for( beta = SMALL, i = 0; i <= SPIKETBLSIZE; i++ ) {
        if( i == SPIKETBLSIZE ) {
            beta = LARGE;
        }
        for( alpha = SMALL, j = 0; j <= SPIKETBLSIZE; j++ ) {
            if( j == SPIKETBLSIZE ) {
                alpha = LARGE;
            }
            if( ( alpha < .0399 ) || ( alpha > .9601 ) ) {
                step = .0001;
            }
            else if( ( alpha < .0999 ) || ( alpha > .9001 ) ) {
                step = .001;
            }
            else {
                step = .01;
            }

            if( ( beta < .0399 ) || ( beta > .9601 ) ) {
                step = .0001;
            }
            else if( ( beta < .0999 ) || ( beta > .9001 ) ) {
                step = ( step < .0009 ) ? step : .001;
            }

            nldltab[ i ][ j ] = nldl_rk4( alpha, beta, step );
            nldhtab[ i ][ j ] = 1.0 - nldh_rk4( alpha, beta, step );
            linear_spike( alpha, beta, &( delaytab[i][j] ), &( linrtab[i][j] ) );

            if( alpha == SMALL ) {
                alpha = STEP;
            }
            else {
                alpha += STEP;
            }
        }
        if( beta == SMALL ) {
            beta = STEP;
        }
        else {
            beta += STEP;
        }
    }
}


void rk4_error( const char * which, double alpha, double beta, double h ) {
    fprintf( stderr, "RK4 error: %s: Didn't reach the peak\n", which );
    fprintf( stderr, "alpha = %f beta = %f h = %f\n", alpha, beta, h );
    exit( 1 );
}


/*
 * 4th order Runge-Kutta method to solve 2-transistors-2-capacitors
 * networks assuming quadratic transistor model.
 *
 * 1) Driven path is ground.
 * 2) Node 1 is discharged initially, and the ratio of its capacitance
 *    to the total capacitance is equal to beta.
 * 3) The ratio of resistance of the resistor closer to ground to the
 *    total resistrance is equal to alpha.
 * 4) Step size is h.
 * 5) Return the peak voltage observed at node 1.
 *
 * This routine is numerically stable when
 *         i) h == .001; .04 < alpha < .96; .04 < beta < .96
 *    ii) h == .01;  .1  < alpha < .9;  .1  < beta < .9
 */
double nldl_rk4( double alpha, double beta, double h ) {
    double        k11, k12, k21, k22, k31, k32, k41, k42;
    double        v1old, v1new, v2old, v2new;
    double        tmp1, tmp2;
    register int  i, j;

    v1old = 0.0;            /* Initial Voltages */
    v2old = 1.0;
    j = ( int ) ( 1.0 / h );    /* Maximum number of iterations */

    for( i = 0; i < j; i++ ) {
        nldl_vector( alpha, beta, h, v1old, v2old, &k11, &k12 );
        tmp1 = v1old + .5 * k11;
        tmp2 = v2old + .5 * k12;
        nldl_vector( alpha, beta, h, tmp1, tmp2, &k21, &k22 );
        tmp1 = v1old + .5 * k21;
        tmp2 = v2old + .5 * k22;
        nldl_vector( alpha, beta, h, tmp1, tmp2, &k31, &k32 );
        tmp1 = v1old + k31;
        tmp2 = v2old + k32;
        nldl_vector( alpha, beta, h, tmp1, tmp2, &k41, &k42 );
        v1new = v1old + ( k11 + 2.0 * k21 + 2.0 * k31 + k41 ) / 6.0;
        v2new = v2old + ( k12 + 2.0 * k22 + 2.0 * k32 + k42 ) / 6.0;
        if ( v1new < v1old ) {
            return( v1old );    /* v1old is the peak */
        }
        v1old = v1new;
        v2old = v2new;
    }

    /* i >= j */
    rk4_error( "nldl", alpha, beta, h );
    return( 0.0 );
}



/*
 * This routine computes the parameter vector for Runge-Kutta method.
 * Customized for 2-transistors-2-capacitors network driven by Gnd.
 */
void nldl_vector( double alpha, double beta, double h, double v1, double v2, double * k1, double * k2 ) {
    double  u1, u2;

    u1 = 2.0 * v1 - v1 * v1;
    u2 = 2.0 * v2 - v2 * v2;

    *k1 = h * ( alpha * u2 - u1 ) / ( alpha * beta * ( 1.0 - alpha ) );
    *k2 = h * ( u1 - u2 ) / ( ( 1.0 - alpha ) * ( 1.0 - beta ) );
}


/*
 * 4th order Runge-Kutta method to solve 2-transistors-2-capacitors
 * networks assuming quadratic transistor model.
 *
 * 1) Driven path is Vdd.
 * 2) Node 1 is charged high initially, and the ratio of its capacitance
 *    to the total capacitance is equal to beta.
 * 3) The ratio of resistance of the resistor closer to driver to the
 *    total resistrance is equal to alpha.
 * 4) Step size is h.
 * 5) Return the peak voltage observed at node 1.
 *
 * This routine is numerically stable when
 *         i) h == .001; .04 < alpha < .96; .04 < beta < .96
 *    ii) h == .01;  .1  < alpha < .9;  .1  < beta < .9
 */
double nldh_rk4( double alpha, double beta, double h ) {
    double        k11, k12, k21, k22, k31, k32, k41, k42;
    double        v1old, v1new, v2old, v2new;
    double        tmp1, tmp2;
    register int  i, j;

    v1old = 1.0;            /* Initial Voltages */
    v2old = 0.0;
    j = ( int ) ( 1.0 / h );    /* Maximum number of iterations */

    for( i = 0; i < j; i ++ ) {
        nldh_vector( alpha, beta, h, v1old, v2old, &k11, &k12 );
        tmp1 = v1old + .5 * k11;
        tmp2 = v2old + .5 * k12;
        nldh_vector( alpha, beta, h, tmp1, tmp2, &k21, &k22 );
        tmp1 = v1old + .5 * k21;
        tmp2 = v2old + .5 * k22;
        nldh_vector( alpha, beta, h, tmp1, tmp2, &k31, &k32 );
        tmp1 = v1old + k31;
        tmp2 = v2old + k32;
        nldh_vector( alpha, beta, h, tmp1, tmp2, &k41, &k42 );
        v1new = v1old + ( k11 + 2.0 * k21 + 2.0 * k31 + k41 ) / 6.0;
        v2new = v2old + ( k12 + 2.0 * k22 + 2.0 * k32 + k42 ) / 6.0;
        if( v1new > v1old ) {
            return( v1old );    /* v1old is the peak */
        }
        v1old = v1new;
        v2old = v2new;
    }

    /* i >= j */
    rk4_error( "nldh", alpha, beta, h );
    return( 0.0 );
}



/*
 * This routine computes the parameter vector for Runge-Kutta method.
 * Customized for 2-transistors-2-capacitors network driven by Vdd.
 */
void nldh_vector( double alpha, double beta, double h, double v1, double v2, double * k1, double * k2 ) {
    double  u1, u2;

    u1 = 1.0 - 2.0 * v1 + v1 * v1;
    u2 = 1.0 - 2.0 * v2 + v2 * v2;

    *k1 = -h * ( alpha * u2 - u1 ) / ( alpha * beta * ( 1.0 - alpha ) );
    *k2 = -h * ( u1 - u2 ) / ( ( 1.0 - alpha ) * ( 1.0 - beta ) );
}


/*
 * Compute spike fluctuation and delay using an rc linear model.
 * Use equations for driven by Gnd case
 */
void linear_spike( double alpha, double beta, double * delay, double * peak ) {
    double  N, b, a, x, tmp;

    tmp = alpha * beta + 1.0 - beta;
    N = alpha * beta * ( 1.0 - alpha ) * ( 1.0 - beta ) / ( tmp * tmp );
    a = sqrt( 1.0 - 4.0 * N );
    x = ( 1.0 - a ) / ( 1.0 + a );
    b = ( 1.0 + x ) * exp( x * log( x ) / ( 1.0 - x ) );
    *peak = b * alpha * ( 1.0 - beta ) / tmp;

    x = ( 1.0 + a ) / ( 1.0 - a );
    b = log( x ) / a;
    *delay = b * beta * ( 1.0 - alpha ) / alpha;
}

int main( int argc, char ** argv ) {
    const char * fname;
    const char * size = "SPIKETBLSIZE";

    if( argc == 1 ) {
        fname = "spiketbl.c";
    }
    else if( argc == 2 ) {
        fname = argv[1];
    }
    else {
        fprintf( stderr, "Usage: genspktbl [filename]\n" ), exit( 1 );
    }

    if( ( F = fopen( fname, "w" ) ) == NULL ) {
        fprintf( stderr, "can not open `%s'\n", fname ), exit( 1 );
    }

    BuildTables();

    fprintf( F, "/* DO NOT EDIT. THIS FILE IS GENERATED BY genspktbl */\n\n" );
    fprintf( F, "#define SPIKETBLSIZE    %d\n\n", SPIKETBLSIZE );
    fprintf( F, "#define NLSPKMIN        %d\n", NLSPKMIN );
    fprintf( F, "#define NLSPKMAX        %d\n", NLSPKMAX );
    fprintf( F, "#define LINEARSPK       %d\n", LINEARSPK );
    fprintf( F, "\n" );

    fprintf( F, "static float spikeTable[ 3 ][ %s + 1 ][ %s + 1 ] =\n{\n",
             size, size );
    fprintf( F, "\t/* non-linear nmos driven low / pmos driven high */\n" );
    PrintTable( nldltab, "%.3f" );
    fprintf( F, "\n\t/* non-linear nmos driven high / pmos driven low */\n" );
    PrintTable( nldhtab, "%.3f" );
    fprintf( F, "\n\t/* linear RC (nmos-pmos mix)*/\n" );
    PrintTable( linrtab, "%.3f" );
    fprintf( F, "};\n\n" );

    fprintf( F, "static float delayTable[ %s + 1 ][ %s + 1 ] = \n{\n",
             size, size );
    PrintTable( delaytab, "%.5e" );
    fprintf( F, "};\n" );

    exit( 0 );
    /* NOTREACHED */
}
