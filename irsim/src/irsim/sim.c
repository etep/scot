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
 * The routines in this file handle network input (from sim files).
 * The input routine "rd_network" replaces the work formerly done by presim.
 * This version differs from the former in the following:
 *  1. voltage drops across transistors are ignored (assumes transistors
 *     driven by nodes with voltage drops have the same resistance as those
 *     driven by a full swing).
 *  2. static power calculations not performed (only useful for nmos anyhow).
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "net.h"
#include "globals.h"
#include "net_macros.h"


#define LSIZE       2000        /* size for input line buffer */
#define MAXARGS     50      /* max. # of arguments per line */


public  nptr   VDD_node;        /* power supply nodes */
public  nptr   GND_node;

public  lptr   on_trans;        /* always on transistors */

public  int    nnodes;          /* number of actual nodes */
public  int    naliases;        /* number of aliased nodes */
public  int    ntrans[ NTTYPES ];   /* number of txtors indexed by type */

public  lptr   freeLinks = NULL;    /* free list of Tlist structs */
public  tptr   freeTrans = NULL;    /* free list of Transistor structs */
public  nptr   freeNodes = NULL;    /* free list of Node structs */

public  tptr   tcap = NULL;     /* list of capacitor-transistors */

private int    lineno;          /* current input file line number */
private char  * simfname;       /* current input filename */
private int    nerrs = 0;       /* # of erros found in sim file */
private int    isBinFile;       /* TRUE binary file, FALSE sim file */

private tptr   rd_tlist;        /* list of transistors just read */

public
#define MIN_CAP     0.00001     /* minimum node capacitance (in pf) */


#define WARNING     0
#define WARN        1
#define MAX_ERRS    20
#define FATAL       (MAX_ERRS + 1)


#define MIT 0
#define LBL 1
#define SU  2

private short int simFormat = MIT, offsLBL = 0 ;


private char    bad_argc_msg[] = "Wrong number of args for '%c' (%d)\n";

#define BAD_ARGC( CMD, ARGC, ARGV )         \
  {                         \
    error( simfname, lineno, bad_argc_msg, CMD, ARGC ); \
    PrArgs( ARGC, ARGV );               \
    CheckErrs( WARN );                  \
    return;                     \
  }                         \
 

private void PrArgs( int argc, char ** argv ) {
    while( argc-- != 0 ) {
        ( void ) fprintf( stderr, "%s ", *argv++ );
    }
    ( void ) fputs( "\n", stderr );
}


private void CheckErrs( int n ) {
    nerrs += n;
    if( nerrs > MAX_ERRS ) {
        if( n != FATAL )
            ( void ) fprintf( stderr, "Too many errors in sim file <%s>\n",
                              simfname );
        exit( 1 );
    }
}



/*
 * Traverse the transistor list and add the node connection-list.  We have
 * to be careful with ALIASed nodes.  Note that transistors with source/drain
 * connected VDD and GND nodes are not linked.
 */
private nptr connect_txtors() {
    register tptr  t, tnext;
    register nptr  gate, src, drn, nd_list;
    register int   type;
    register long  visited;

    visited = VISITED;
    nd_list = NULL;

    for( t = rd_tlist; t != NULL; t = tnext ) {
        tnext = t->scache.t;
        for( gate = t->gate; gate->nflags & ALIAS; gate = gate->nlink );
        for( src = t->source; src->nflags & ALIAS; src = src->nlink );
        for( drn = t->drain; drn->nflags & ALIAS; drn = drn->nlink );

        t->gate = gate;
        t->source = src;
        t->drain = drn;

        type = t->ttype;
        t->state = ( type & ALWAYSON ) ? WEAK : UNKNOWN;
        t->tflags = 0;

        ntrans[ type ]++;
        if( src == drn or ( src->nflags & drn->nflags & POWER_RAIL ) ) {
            t->ttype |= TCAP;      /* transistor is just a capacitor */
            LINK_TCAP( t );
        }
        else {
            /* do not connect gate if ALWAYSON since they do not matter */
            if( t->ttype & ALWAYSON ) {
                CONNECT( on_trans, t );
            }
            else {
                CONNECT( t->gate->ngate, t );
            }

            if( not ( src->nflags & POWER_RAIL ) ) {
                CONNECT( src->nterm, t );
                LINK_TO_LIST( src, nd_list, visited );
            }
            if( not ( drn->nflags & POWER_RAIL ) ) {
                CONNECT( drn->nterm, t );
                LINK_TO_LIST( drn, nd_list, visited );
            }
        }
    }

    return( nd_list );
}


/*
 * node area and perimeter info (N sim command).
 */
private void node_info( int targc, char ** targv ) {
    register nptr  n;

    if( targc != 8 ) {
        BAD_ARGC( 'N', targc, targv );
    }

    n = GetNode( targv[1] );

    n->ncap +=   atof( targv[4] ) * ( CMA * LAMBDA2 ) +
                 atof( targv[5] ) * ( CPA * LAMBDA2 ) +
                 atof( targv[6] ) * ( CDA * LAMBDA2 ) +
                 atof( targv[7] ) * 2.0 * ( CDP * LAMBDA );
}


/*
 * new format node area and perimeter info (M sim command).
 */
private void nnode_info( int targc, char ** targv ) {
    register nptr  n;

    if( targc != 14 ) {
        BAD_ARGC( 'M', targc, targv );
    }

    n = GetNode( targv[1] );

    n->ncap +=   atof( targv[4] ) * ( CM2A * LAMBDA2 ) +
                 atof( targv[5] ) * 2.0 * ( CM2P * LAMBDA ) +
                 atof( targv[6] ) * ( CMA * LAMBDA2 ) +
                 atof( targv[7] ) * 2.0 * ( CMP * LAMBDA ) +
                 atof( targv[8] ) * ( CPA * LAMBDA2 ) +
                 atof( targv[9] ) * 2.0 * ( CPP * LAMBDA ) +
                 atof( targv[10] ) * ( CDA * LAMBDA ) +
                 atof( targv[11] ) * 2.0 * ( CDP * LAMBDA ) +
                 atof( targv[12] ) * ( CPDA * LAMBDA2 ) +
                 atof( targv[13] ) * 2.0 * ( CPDP * LAMBDA );
}

#ifdef USER_SUBCKT
/*
 * new subckt with (num_inputs, inputs...., outputs...., subckt_name) HMD
 */
private void newsubckt( int targc, char ** targv ) {
    double fabs(), atof();
    int i, n, ninputs, noutputs;
    long width, length;
    ufun model, init ;
    register tptr t;
    tptr t2;
    lptr l;
    /* tptr sc_list = (tptr)NULL; */
    char * sname, *out_name_Ub, *out_name_D;
    int subckt_out_counter = 0;
    nptr * nodes;
    lptr diodes = ( lptr )NULL, d;
    uptr usrData;
    int  inst;

    ninputs   = atoi( targv[1] );
    noutputs  = ( targc - 3 - ninputs )/2;
    sname     = ( char * )malloc( strlen( targv[targc-1] )+1 );
    out_name_Ub = ( char * )malloc( strlen( targv[targc-1] )+20 );
    out_name_D  = ( char * )malloc( strlen( targv[targc-1] )+20 );
    strcpy( sname, targv[targc-1] );
    model     = subckt_instantiate( sname, &init, &inst );
    if ( model == ( ufun )NULL ) {
        lprintf( stdout, "Error: subckt `%s' not defined\n",sname );
        exit( 1 );
    }
    targc -= 2;
    targv += 2;
    nodes = ( nptr * )malloc( ( ninputs + 2*noutputs ) * sizeof( nptr ) );

    lprintf( stdout, "defining new blackbox `%s' instance %d #i:%d #o:%d\n",
             sname,inst, ninputs, noutputs );

    if ( ninputs <= 0 ) {
        lprintf( stderr,"\terror 0 inputs\n" );
        exit( 1 );
    }
    if ( noutputs < 0 ) {
        lprintf( stderr,"\terror < 0 outputs\n" );
        exit( 1 );

    }
    if ( targc < ninputs + 1 ) {
        lprintf( stderr,"\terror wrong # args %d\n",targc );
        exit( 1 );
    }
    if ( init ) {
        usrData = ( uptr )( *init )();
    }

    for ( i = 0; i<ninputs; i++ ) { /* create new transistor */
        float ct ;

        NEW_TRANS( t );
        NEW_SUBCKT( t->subptr );
        t->ttype = SUBCKT;
        t->gate = GetNode( targv[i] );
        t->subptr->nodes = nodes;
        t->source = VDD_node;
        t->drain = t->gate;
        /* link it to the list */
        t->scache.t = rd_tlist;
        rd_tlist = t;
        /* t->dcache.t = sc_list; sc_list = t; */
        t->r = requiv( RESIST, 2 , 1000000*LAMBDACM );
        t->subptr->model = model;
        t->subptr->sname = sname;

        /* All this can be referenced by a pointer to a subckt structure */
        NEW_LINK( d );
        d->xtor = t;
        d->next = diodes;
        t->subptr->ndiode = d;
        diodes = d;
        t->subptr->nins = ninputs;
        t->subptr->nouts = noutputs;
        nodes[i] = t->gate;
        t->subptr->udata = usrData;
    }
    for( l = t->subptr->ndiode; l != NULL; l = l->next ) {
        t2 = l->xtor;
        t2->subptr->ndiode = diodes;;
    }

    for ( n=i ; i<targc-1; i+=2, n++ ) {
        int rt;
        /* defining two transistors for each output : */

        sprintf( out_name_Ub, "%sUb_%d_%d", sname, inst, subckt_out_counter );
        sprintf( out_name_D , "%sD_%d_%d", sname, inst, subckt_out_counter++ );
        if ( find( out_name_Ub ) || find( out_name_D ) ) {
            lprintf( stderr,"Error %s or %s already exists\n",
                     out_name_Ub, out_name_D );
            exit( 1 );
        }
        rt = ( int ) fabs( atof( targv[i+1] ) );

        NEW_TRANS( t );   /* Add pmos output driver */
        t->ttype    = PCHAN;
        t->gate     = GetNode( out_name_Ub );
        nodes[n]    = t->gate;
        t->drain    = VDD_node;
        t->source   = GetNode( targv[i] );
        t->scache.t = rd_tlist; /* link it to the list */
        rd_tlist    = t;
        /* t->dcache.t = sc_list; sc_list = t; */
        t->drain->ncap += 0;
        t->r        = requiv( RESIST, 2 , rt*LAMBDACM );

        NEW_TRANS( t );   /* Add nmos output driver */
        t->ttype    = NCHAN;
        t->gate     = GetNode( out_name_D );
        t->drain    = GetNode( targv[i] );
        nodes[n+noutputs]    = t->gate;
        t->source   = GND_node;
        t->scache.t = rd_tlist; /* link it to the list */
        rd_tlist    = t;
        /* t->dcache.t = sc_list; sc_list = t; */
        t->source->ncap += 0;
        t->r        = requiv( RESIST, 2 , rt*LAMBDACM );
    } /* for loop for outputs */
    free( out_name_D );
    free( out_name_Ub );
}
#endif

private int    AP_error = FALSE;

/* parse g=...,A_NUM,P_NUM attributes */
private int parseAttr( const char * str, int * a, int * p ) {
    int l;
    const char * s;

    if ( ( l=strlen( str ) ) <= 2 ) {
        *a = 0 ;
        *p = 0;
        return FALSE;
    }

    for ( s = str+l*sizeof( char ) ; *s != 'A' && s != str ; s-- ) ;
    if ( sscanf( s,"A_%d,P_%d", a, p ) != 2 )
        if ( sscanf( s,"a_%d,p_%d", a, p ) != 2 ) {
            error( simfname, lineno, "Bad area/perimeter attributes\n" );
            return FALSE;
        }
    return TRUE;
}


/*
 * new transistor.  Implant specifies type.
 * AreaPos specifies the argument number that contains the area (if any).
 */
private void newtrans( int implant, int targc, char ** targv ) {
    nptr           gate, src, drn, ntemp;
    long           x, y, width, length;
    double         cap;
    register tptr  t;
    unsigned int fetHasAP = FALSE ;

    // -- TODO -- these *were* unsigned (why??)
    int asrc, adrn, psrc, pdrn;

    if( implant == RESIST ) {
        if( targc != 4 ) {
            BAD_ARGC( 'r', targc, targv );
        }

        gate = VDD_node;
        src = GetNode( targv[1] );
        drn = GetNode( targv[2] );

        length = atof( targv[3] ) * LAMBDACM;
        width = 0;
    }
    else {
        if( targc < 4+offsLBL or targc > 11+offsLBL ) {
            BAD_ARGC( targv[0][0], targc, targv );
        }

        gate = GetNode( targv[1] );
        src = GetNode( targv[2] );
        drn = GetNode( targv[3] );

        if( targc > 5+offsLBL ) {
            length = atof( targv[4+offsLBL] ) * LAMBDACM;
            width = atof( targv[5+offsLBL] ) * LAMBDACM;
            if( width <= 0 or length <= 0 ) {
                error( simfname, lineno,
                       "Bad transistor width=%g or length=%g\n", width, length );
                return;
            }
            if( targc > 7+offsLBL ) {
                x = atoi( targv[6+offsLBL] );
                y = atoi( targv[7+offsLBL] );
            }
        }
        else {
            width = length = 2 * LAMBDACM;
        }

        cap = length * width * CTGA;
    }

    NEW_TRANS( t );          /* create new transistor */

    t->ttype = implant;
    t->gate = gate;
    t->source = src;
    t->drain = drn;

    if( targc > 7+offsLBL ) {
        t->x.pos = x;
        t->y.pos = y;
        EnterPos( t, TRUE );      /* Enter transistor position */
        if ( simFormat == SU && targc >= 9 ) { /* parse area and perimeter */
            register int i ;

            for ( fetHasAP = TRUE, i = 8 ; i < targc ; i++ ) {
                if ( targv[i][0] == 's' ) {
                    fetHasAP = fetHasAP && parseAttr( targv[i], &asrc, &psrc );
                }
                else if ( targv[i][0] == 'd' ) {
                    fetHasAP = fetHasAP && parseAttr( targv[i], &adrn, &pdrn );
                }
            }
        }
    }
    else {
        EnterPos( t, FALSE );     /* Enter transistor position */
        if ( simFormat == SU && ! AP_error ) {
            error(  simfname, lineno, "no area/perim S/D attributes on fet\n" );
            AP_error = TRUE;
        }
    }

    t->scache.t = rd_tlist;      /* link it to the list */
    rd_tlist = t;

    t->r = requiv( implant, width, length );

    /* update node capacitances  */
    gate->ncap += cap;

    if ( simFormat == SU ) {
        double capsrc = 0, capdrn =0 ;

        if ( fetHasAP ) {
            if ( implant == PCHAN ) {
                capsrc = asrc * LAMBDA * LAMBDA * CPDA + psrc * LAMBDA *CPDP;
                capdrn = adrn * LAMBDA * LAMBDA * CPDA + pdrn * LAMBDA *CPDP;
            }
            else if ( implant == NCHAN || implant == DEP ) {
                capsrc = asrc * LAMBDA * LAMBDA * CDA + psrc * LAMBDA *CDP;
                capdrn = adrn * LAMBDA * LAMBDA * CDA + pdrn * LAMBDA *CDP;
            }
        }
        else if ( ! AP_error  ) {
            lprintf( stderr, "Warning: Junction capacitances might be incorrect\n" );
            AP_error = TRUE;
        }
#ifdef NOTDEF
        printf( "LAMBDA=%f CDA=%f CDP=%f asrc=%d psrc=%d adrn=%d pdrn=%d capsrc=%lf capdrn=%lf\n", LAMBDA, CDA, CDP, asrc, psrc, adrn, pdrn, capsrc, capdrn );
#endif
        src->ncap += capsrc;
        drn->ncap += capdrn;
    }
    else if( config_flags & TDIFFCAP ) {
        if ( implant == PCHAN ) {
            cap = CPTDW * width + CPTDE;
        }
        else if ( implant == NCHAN || implant == DEP ) {
            cap = CTDW * width + CTDE;
        }
        else { cap = 0.0 ; }
        src->ncap += cap;
        drn->ncap += cap;
    }
}

/*
 * accept a bunch of aliases for a node (= sim command).
 */
private void alias( int targc, char ** targv ) {
    register nptr  n, m;
    register int   i;

    if( targc < 3 ) {
        BAD_ARGC( '=', targc, targv );
    }

    n = GetNode( targv[1] );

    for( i = 2; i < targc; i++ ) {
        m = GetNode( targv[i] );
        if( m == n ) {
            continue;
        }

        if( m->nflags & POWER_RAIL ) {
            SWAP_NODES( m, n );
        }

        if( m->nflags & POWER_RAIL ) {
            error( simfname, lineno, "Can't alias the power supplies\n" );
            continue;
        }

        n->ncap += m->ncap;

        m->nlink = n;
        m->nflags |= ALIAS;
        m->ncap = 0.0;
        nnodes--;
        naliases++;
    }
}


/*
 * node threshold voltages (t sim command).
 */
private void nthresh( int targc, char ** targv ) {
    register nptr  n;

    if( targc != 4 ) {
        BAD_ARGC( 't', targc, targv );
    }

    n = GetNode( targv[1] );
    n->vlow = atof( targv[2] );
    n->vhigh = atof( targv[3] );
}


/*
 * User delay for a node (D sim command).
 */
private void ndelay( int targc, char ** targv ) {
    register nptr  n;

    if( targc != 4 ) {
        BAD_ARGC( 'D', targc, targv );
    }

    n = GetNode( targv[1] );
    n->nflags |= USERDELAY;
    n->tplh = ns2d( atof( targv[2] ) );
    n->tphl = ns2d( atof( targv[3] ) );
}


/*
 * add capacitance to a node (c sim command).
 */
private void ncap( int targc, char ** targv ) {
    register nptr n, m;
    float         cap;

    if( targc == 3 ) {
        n = GetNode( targv[1] );
        n->ncap += atof( targv[2] );
    }
    else if( targc == 4 ) {      /* two terminal caps    */
        cap = atof( targv[3] ) / 1000;        /* ff to pf conversion */
        n = GetNode( targv[1] );
        m = GetNode( targv[2] );
        if( n != m ) {        /* add cap to both nodes */
            if( m != GND_node ) { m->ncap += cap; }
            if( n != GND_node ) { n->ncap += cap; }
        }
        else if( n == GND_node ) {    /* same node, only GND makes sense */
            n->ncap += cap;
        }
    }
    else {
        BAD_ARGC( 'c', targc, targv );
    }
}


/*
 * parse input line into tokens, filling up carg and setting targc
 */
private int parse_line( char * line, char ** carg ) {
    register char  ch;
    register int   targc;

    targc = 0;
    while( ch = *line++ ) {
        if( ch <= ' ' ) {
            continue;
        }
        targc++;
        *carg++ = line - 1;
        while( *line > ' ' ) {
            line++;
        }
        if( *line ) {
            *line++ = '\0';
        }
    }
    *carg = 0;
    return( targc );
}


private int    R_error = FALSE;
private int    A_error = FALSE;

private int input_sim( char * simfile ) {
    FILE * fin;
    char  line[LSIZE];
    char * targv[MAXARGS];   /* tokens on current command line */
    int   targc;     /* number of args on command line */
    long  offset;        /* state of previously opened file */
    int   olineno;

    if( ( fin = fopen( simfile, "r" ) ) == NULL ) {
        lprintf( stderr, "cannot open '%s' for sim input\n", simfile );
        return( FALSE );
    }
    simfname = simfile;
    lineno = 0;

    while( fgetline( line, LSIZE, fin ) != NULL ) {
        lineno++;
        targc = parse_line( line, targv );
        if( targv[0] == NULL ) {
            continue;
        }
        switch( targv[0][0] ) {
        case '@' :
            if( targc != 2 ) {
                error( simfname, lineno, bad_argc_msg, '@', targc );
                CheckErrs( WARN );
                break;
            }
            offset = ftell( fin );
            olineno = lineno;
            ( void ) fclose( fin );
            ( void ) input_sim( targv[1] );
            if( ( fin = fopen( simfile, "r" ) ) == NULL ) {
                error( simfname, lineno, "can't re-open sim file '%s'\n",
                       simfile );
                CheckErrs( WARN );
                return( FALSE );
            }
            ( void ) fseek( fin, offset, 0 );
            simfname = simfile;
            lineno = olineno;
            break;

        case '|' :
            if ( lineno > 1 ) { break; }
            if ( targc >= 2 ) {
                double lmbd = atof( targv[2] )/100.0;

                if ( lmbd != LAMBDA ) {
                    error( simfname, lineno, "WARNING\7: sim file lambda (%.2lfu) != config lambda (%.2lfu)\n", lmbd, LAMBDA );
                    error( simfname, lineno, "WARNING\7: Using the config lambda (%.2lfu)\n", LAMBDA );
                }
            }
            if ( targc >= 6 ) {
                if ( !strcasecmp( targv[6], "MIT" ) ) { simFormat = MIT ; }
                else if ( !strcasecmp( targv[6], "LBL" ) ) { simFormat = LBL ; }
                else if ( !strcasecmp( targv[6], "SU" ) ) { simFormat = SU ; }
                else {
                    error( simfname, lineno, "Unknown format '%s' assuming MIT\n", targv[6] );
                    simFormat = MIT ;
                }
                offsLBL = ( simFormat == LBL ) ? 1 : 0 ;/* skip bulk */
                if ( simFormat == SU ) {
                    if ( CDA == 0.0 || CDP == 0.0 || CPDA == 0.0 || CPDP == 0.0 ) {
                        lprintf( stderr,"Warning: SU format and area/perim cap values are zero\n" );
                    }
                    else {
                        lprintf( stderr,"info: SU format --> using S/D attrs to compute junction parasitics\n" );
                    }
                }
            }
            break;
        case 'e' :
        case 'n' :
            newtrans( NCHAN, targc, targv );
            break;
        case 'p' :
            newtrans( PCHAN, targc, targv );
            break;
        case 'd' :
            newtrans( DEP, targc, targv );
            break;
        case 'r' :
            newtrans( RESIST, targc, targv );
            break;
#ifdef  USER_SUBCKT
        case 'x' :
            newsubckt( targc, targv );
            break;
#endif
        case 'N' :
            node_info( targc, targv );
            break;
        case 'M' :
            nnode_info( targc, targv );
            break;
        case 'c' :
        case 'C' :
            ncap( targc, targv );
            break;
        case '=' :
            alias( targc, targv );
            break;
        case 't' :
            nthresh( targc, targv );
            break;
        case 'D' :
            ndelay( targc, targv );
            break;
        case 'R' :
            if( not R_error ) {    /* only warn about this 1 time */
                lprintf( stderr,
                         "%s: Ignoring lumped-resistance ('R' construct)\n",
                         simfname );
                R_error = TRUE;
            }
            break;
        case 'A' :
            if( not A_error ) {    /* only warn about this 1 time */
                lprintf( stderr,
                         "%s: Ignoring attribute-line ('A' construct)\n",
                         simfname );
                A_error = TRUE;
            }
            break;
        case '<' :
            if( lineno == 1 and rd_netfile( fin, line ) ) {
                ( void ) fclose( fin );
                return( TRUE );
            }
            /* fall through if rd_netfile returns FALSE */
        default :
            error( simfname, lineno, "Unrecognized input line (%s)\n",
                   targv[0] );
            CheckErrs( WARN );
        }
    }
    ( void ) fclose( fin );
    lprintf( stdout, "\nRead %s lambda:%.2lfu format:%s\n", simfile, LAMBDA,
             simFormat == MIT ? "MIT" : ( simFormat == LBL ? "LBL" : "SU" ) );
    return( FALSE );
}



private void init_counts() {
    register int  i;

    for( i = 0; i < NTTYPES; i++ ) {
        ntrans[i] = 0;
    }
    nnodes = naliases = 0;
}


public int rd_network( char * simfile ) {
    static int      firstcall = 1;

    if( firstcall ) {
        rd_tlist = NULL;
        init_hash();
#ifdef USER_SUBCKT
        init_subs();
#endif
        init_counts();
        init_listTbl();

        VDD_node = GetNode( "Vdd" );
        VDD_node->npot = HIGH;
        VDD_node->nflags |= ( INPUT | POWER_RAIL );
        VDD_node->head.inp = 1;
        VDD_node->head.val = HIGH;
        VDD_node->head.punt = 0;
        VDD_node->head.time = 0;
        VDD_node->head.t.r.rtime = VDD_node->head.t.r.delay = 0;
        VDD_node->head.next = last_hist;
        VDD_node->curr = &( VDD_node->head );

        GND_node = GetNode( "Gnd" );
        GND_node->npot = LOW;
        GND_node->nflags |= ( INPUT | POWER_RAIL );
        GND_node->head.inp = 1;
        GND_node->head.val = LOW;
        GND_node->head.punt = 0;
        GND_node->head.time = 0;
        GND_node->head.t.r.rtime = GND_node->head.t.r.delay = 0;
        GND_node->head.next = last_hist;
        GND_node->curr = &( GND_node->head );

        NEW_TRANS( tcap );
        tcap->scache.t = tcap->dcache.t = tcap;
        tcap->x.pos = 0;

        firstcall = 0;
    }
    nerrs = 0;

    isBinFile = input_sim( simfile );
    if( nerrs > 0 ) {
        exit( 1 );
    }
}


public void pTotalNodes() {
    lprintf( stdout, "%d nodes", nnodes );
    if( naliases != 0 ) {
        lprintf( stdout, ", %d aliases", naliases );
    }
    lprintf( stdout, "; " );
}


public void pTotalTxtors() {
    int  i;

    lprintf( stdout, "transistors:" );
    for( i = 0; i < NTTYPES; i++ )
        if( ntrans[i] != 0 ) {
            lprintf( stdout, " %s=%d", ttype[i], ntrans[i] );
        }
    if( tcap->x.pos != 0 ) {
        lprintf( stdout, " shorted=%d", tcap->x.pos );
    }
    lprintf( stdout, "\n" );
}


public void ConnectNetwork() {
    nptr  ndlist;

    pTotalNodes();

    ndlist = ( isBinFile ) ? bin_connect_txtors() : connect_txtors();

    make_parallel( ndlist );
    make_stacks( ndlist );

    pTotalTxtors();
    pParallelTxtors();
    pStackedTxtors();
}
