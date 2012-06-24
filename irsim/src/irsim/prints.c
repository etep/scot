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
 * Prints various messages but deals with varargs correctly.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "defs.h"


extern	FILE  *logfile;


private void logprint( char * s ) {
   static int  docomment = 1;

   while( *s != '\0' ) {
      if( docomment ) {
         ( void ) putc( '|', logfile );
         ( void ) putc( ' ', logfile );
         docomment = 0;
      }
      ( void ) putc( *s, logfile );
      if( *s++ == '\n' )
         docomment = 1;
   }
}


/* VARARGS */
public void lprintf( FILE * fp, const char * fmt, ... ) {
   va_list  args;
   char     buff[ 300 ];

   va_start( args, fmt );

   ( void ) vsprintf( buff, fmt, args );
   va_end( args );
   ( void ) fputs( buff, fp );

   if( logfile != NULL ) {
      logprint( buff );
   }
}


/* VARARGS */
public void error( const char * filename, int lineno, const char * fmt, ... ) {
   va_list  args;
   char     buf1[ 4096 ], buf2[ 4096 ];

   va_start( args, fmt ); 

   ( void ) sprintf( buf1, "(%s,%d): ", filename, lineno );
   ( void ) vsprintf( buf2, fmt, args );
   va_end( args );

   ( void ) fputs( buf1, stderr );
   ( void ) fputs( buf2, stderr );

   if( logfile != NULL ) {
      logprint( buf1 );
      logprint( buf2 );
   }
}
