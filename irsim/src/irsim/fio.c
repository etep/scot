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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef OS2
#include <stdlib.h>
#include <io.h>
#include <conio.h>
#define INCL_DOSPROCESS
#include <os2.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
extern   Display   *  display;
#else
#include <defs.h>
#include "net.h"
#include "globals.h"
#ifndef clearerr
extern  void    clearerr();
#endif
#endif  /* OS2 */

/*
 * My version of fgets, fread, and fwrite.  These routines provide the same
 * functionality as the stdio ones; taking care of restarting the operation
 * upon an interrupt condition.  This is mostly for system V.
 */

public char * fgetline( char * bp, int len, FILE * fp ) {
    register char * buff = bp;
    register int  c;

    contline = 0;
    while( --len > 0 ) {

again :
#ifdef OS2
        if( !kbhit() && ( isatty( ( int ) fileno( fp ) ) ) ) {
            if ( analyzerON ) {
                DosSleep( 50 );
                EnableInput();
            }
            DosSleep( 50 );
            goto again;
        }
        if( fp == stdin ) {
            c = _getch();
            if( c != '\b' ) { printf( "%c", c ); }
            fflush( stdout );
        }
        else
#endif  /* OS2 */
            c = getc( fp );
        if( c == EOF ) {
            if( feof( fp ) == 0 ) {
                clearerr( fp );
                goto again;
            }
            *buff = '\0';
            return( NULL );
        }
        if ( ( fp == stdin ) && ( c == '\b' ) && ( buff > bp ) ) {
            printf( "\b \b" );
            fflush( stdout );
            *buff--;
        }
        else {
            if( ( c == '\\' ) && ( *bp != '|' ) ) {
                c = getc( fp );
                contline++;
                if( isatty( ( int ) fileno( fp ) ) ) {
                    printf( "cont>" );
                    fflush( stdout );
                }
                goto again;
            }
            if ( c != '\b' ) {
                *buff++ = c;
            }
        }
        if( ( c == '\n' ) || ( c == 0xd ) ) {
            c = '\n';
            break;
        }
    }
    *buff = '\0';
    if ( len <= 0 ) {
        printf( "Command line max length exceeded.\n" );
        exit( -1 );
    }
    return( bp );
}


public int Fread( char * ptr, int size, FILE * fp ) {
    register int  ret;

again :
    ret = fread( ptr, 1, size, fp );
    if( ret <= 0 and feof( fp ) == 0 ) {
        clearerr( fp );
        goto again;
    }
    return( ret );
}


public int Fwrite( char * ptr, int size, FILE * fp ) {
    register int  ret;

again :
    ret = fwrite( ptr, 1, size, fp );
    if( ret <= 0 and feof( fp ) == 0 ) {
        clearerr( fp );
        goto again;
    }
    return( ret );
}
