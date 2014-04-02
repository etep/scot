#include <d2p.hpp>

/*#define DEBUG */
/*
 * Generate the primal solution from "dgopt.sol".
 * The primal solution is written in "primal.sol".
 */

#ifdef DEBUG
void main ( void ) {
    int i, numvar, numcon, status;
    double * primal_solution;
    double * at_act;
    double * u_lower;

    double p_obj, d_obj;
    numvar = 3;
    numcon = 5;

    primal_solution = ( double * ) malloc( numvar * sizeof( double ) );
    at_act = ( double * ) malloc( numcon * sizeof( double ) );
    u_lower = ( double * ) malloc( numcon * sizeof( double ) );

    status = dual2primal ( numvar, numcon, primal_solution, at_act, u_lower, &p_obj, &d_obj );

    if ( status == PRIMAL_AND_DUAL_FEASIBLE ) {
        fprintf( stdout, "Solution status: primal-dual feasible\n" );
        fprintf( stdout, "Primal objective: " );
        printf( "%3.8f\n", p_obj );
        fprintf( stdout, "Dual objective: " );
        printf( "%3.8f\n", d_obj );
        for( i = 0; i <= numvar-1; i++ ) {
            printf( "%3.8f\n", primal_solution [i] );
        }
        fprintf( stdout, "\n" );
        fprintf( stdout, "VARIABLES:\n" );

        for( i = 0; i <= numcon-1; i++ ) {
            printf( "%3.8f\n", at_act [i] );
        }
        fprintf( stdout, "DUAL_LOWER:\n" );
        for( i = 0; i <= numcon-1; i++ ) {
            printf( "%3.15f\n", u_lower [i] );
        }
    }
}
#endif

int dual2primal( const std::string & solFileName, int numvar, int numcon, double * primal_solution, double * at_act, double * u_lower, double * p_obj, double * d_obj ) {
    /*
    printf( "numvar = %d\n", numvar );
    printf( "numcon = %d\n", numcon );
    */

    char * inbuf;
    char * obj_start;
    char * L_start;
    char * U_start;

    char primal_obj [EXPRESSION_LEN];
    char dual_obj [EXPRESSION_LEN];
    char dual_lower [EXPRESSION_LEN];
    char dual_upper [EXPRESSION_LEN];
    char at_activity [EXPRESSION_LEN];

    int i, k, line, c, length;
    double lambda, lambda1, lambda2;

    FILE * dual_solution_file;
    char * dual_solution_file_name = strdup( solFileName.c_str() );

    //strcpy( dual_solution_file_name, "dgopt.sol" );
    dual_solution_file = fopen( dual_solution_file_name, "rb" );

    if( dual_solution_file == NULL ) {
        //fprintf( stderr, "can't open the %s\n", dual_solution_file_name );
        return DGOPT_ERROR;
    }

    inbuf = ( char * ) malloc( ( int ) 300 );

    if( inbuf == NULL ) {
        fprintf( stderr, "Memory allocation error\n" );
        fclose ( dual_solution_file );
        return MEMORY_ALLOCATION_ERROR;
    }

    //
    // Initialization
    //
    for( i = 0; i <= EXPRESSION_LEN-1; i++ ) {
        primal_obj[i]=dual_obj[i]=0;
        dual_lower[i]=dual_upper[i]=0;
    }

    line = 1;
    k=0;
    while ( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    //
    // Check the status of the optimal solution.
    //
    line = 2;
    k=0;

    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    length = k;
    obj_start = strstr( inbuf, "PRIMAL_AND_DUAL_FEASIBLE" );

    if( obj_start == NULL ) {
        fprintf( stderr,"not primal-dual feasible\n" );
        free ( inbuf );
        fclose ( dual_solution_file );
        return NOT_PRIMAL_AND_DUAL_FEASIBLE;
    }

    line = 3;
    k=0;

    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    line = 4;
    k=0;
    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    obj_start = strstr ( inbuf, "obj" );

    if( obj_start == NULL ) {
        fprintf( stderr, "No obj\n" );
        free ( inbuf );
        fclose ( dual_solution_file );
        return GP_SOLVER_ERROR;
    }

    //
    // Extract the primal objective.
    //
    line = 5;
    k=0;
    while ( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }
    k=0;
    for ( k = 0; k <= 14; k++ ) {
        c = obj_start[k];
        if ( ( isdigit( c ) ) | ( c == '.' ) | ( c == 'e' ) | ( c == '+' ) | ( c == '-' ) ) {
            primal_obj[k] = c;
        }
        else {
            primal_obj[k] = 0;
        }
    }
    *p_obj=atof( primal_obj );

    //
    // Extract the dual objective.
    //
    line = 6;
    k=0;
    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    k=0;
    for( k = 0; k <= 14; k++ ) {

        c = obj_start[k];

        if( ( isdigit( c ) ) | ( c == '.' ) | ( c == 'e' ) | ( c == '+' ) | ( c == '-' ) ) {
            dual_obj[k] = c;
        }
        else {
            dual_obj[k] = 0;
        }
    }

    *d_obj = atof( dual_obj );

    line = 7;
    k=0;
    while ( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    line = 8;
    k=0;
    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    line = 9;
    k=0;
    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    L_start = strstr ( inbuf, "DUAL LOWER" );
    if( L_start == NULL ) {
        fprintf( stderr, "NO DUAL LOWER\n" );
        free ( inbuf );
        fclose ( dual_solution_file );
        return GP_SOLVER_ERROR;
    }

    U_start = strstr ( inbuf, "DUAL UPPER" );
    if( U_start == NULL ) {
        fprintf( stderr, "NO DUAL UPPER\n" );
        free ( inbuf );
        fclose ( dual_solution_file );
        return GP_SOLVER_ERROR;
    }

    //
    // Extract the primal solution.
    //
    for( i = 0; i <= numvar-1; i++ ) {

        line++;

        k=0;
        while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
            inbuf[k++] = ( char ) c;
        }

        k=0;
        for( k = 0; k <= EXPRESSION_LEN-1; k++ ) {
            c = L_start[k];
            if( ( isdigit( c ) ) | ( c == '.' ) | ( c == 'e' ) | ( c == '+' ) | ( c == '-' ) ) {
                dual_lower[k] = c;
            }
            else {
                dual_lower[k] = 0;
            }
        }

        k=0;
        for( k = 0; k <= EXPRESSION_LEN-1; k++ ) {

            c = U_start[k];
            if( ( isdigit( c ) ) | ( c == '.' ) | ( c == 'e' ) | ( c == '+' ) | ( c == '-' ) ) {
                dual_upper[k] = c;
            }
            else {
                dual_upper[k] = 0;
            }
        }
        lambda1 = atof( dual_lower );
        lambda2 = atof( dual_upper );
        lambda = lambda2 - lambda1;
        primal_solution [i] = exp( lambda );
    }

    line = 9 + numvar;
    k=0;
    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    line = 9 + numvar +1;
    k=0;
    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    line = 9 + numvar + 2;
    k=0;
    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    L_start = strstr ( inbuf, "VARIABLES" );
    if( L_start == NULL ) {
        fprintf( stderr, "NO VARIABLES\n" );
        free ( inbuf );
        fclose ( dual_solution_file );
        return GP_SOLVER_ERROR;
    }

    line = 9 + numvar + 3;
    k=0;
    while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
        inbuf[k++] = ( char ) c;
    }

    L_start = strstr ( inbuf, "AT ACTIVITY" );
    if( L_start == NULL ) {
        fprintf( stderr, "NO AT ACTIVITY\n" );
        free ( inbuf );
        fclose ( dual_solution_file );
        return GP_SOLVER_ERROR;
    }

    U_start = strstr ( inbuf, "DUAL LOWER" );
    if( U_start == NULL ) {
        fprintf( stderr, "NO DUAL UPPER\n" );
        free ( inbuf );
        fclose ( dual_solution_file );
        return GP_SOLVER_ERROR;
    }

    //
    // Extract the Lagrange multiplies.
    //
    for( i = 0; i <= numcon-1; i++ ) {

        line++;

        k=0;
        while( ( c = fgetc( dual_solution_file ) ) != '\n' ) {
            inbuf [k++] = ( char ) c;
        }

        k=0;
        for( k = 0; k <= EXPRESSION_LEN-1; k++ ) {

            c = L_start [k+3];
            if( ( isdigit( c ) ) | ( c == '.' ) | ( c == 'e' ) | ( c == '+' ) | ( c == '-' ) ) {
                at_activity [k] = c;
            }
            else {
                at_activity [k] = 0;
            }
        }

        k=0;
        for( k = 0; k <= EXPRESSION_LEN-1; k++ ) {

            c = U_start [k];
            if( ( isdigit( c ) ) | ( c == '.' ) | ( c == 'e' ) | ( c == '+' ) | ( c == '-' ) ) {
                dual_lower [k] = c;
            }
            else {
                dual_lower [k] = 0;
            }
        }
        at_act [i] = atof ( at_activity );
        u_lower [i] = atof ( dual_lower );
    }

    free ( inbuf );
    fclose ( dual_solution_file );
    return PRIMAL_AND_DUAL_FEASIBLE;
}
