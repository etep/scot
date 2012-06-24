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
#include <string.h>

#ifdef SYS_V
#    include <sys/types.h>
#    include <unistd.h>
#endif

#ifndef OS2
#include <sys/file.h>
#else
#include <stdlib.h>
#include <io.h>
#define F_OK 0
#define access _access
#endif  /* OS2 */
#include <stdio.h>
#ifndef OS2
#include <pwd.h>
#endif  /* OS2 */
#include "defs.h"
#include "mem.h"

public	char    *cad_lib;
public	char    *cad_bin;

#ifndef OS2
// extern	char           *getenv();
#endif  /* OS2  */
extern	struct passwd  *getpwnam();
// extern	char           *Valloc();


public void InitCAD() {
   const char     *s;
   struct passwd  *pwd;
   int            len;

   /* first try CAD_HOME env. variable */

   s = getenv( "CAD_HOME" );
   if( s ) {
      if( access( s, F_OK ) == 0 ) {
         goto go_it;
      }
   }

   /* try "~cad" */
   #ifndef OS2
   pwd = getpwnam( "cad" );
   s = ( pwd ) ? pwd->pw_dir : 0;
   #endif  /* OS2 */
   if( s ) {
      if( access( s, F_OK ) == 0 ) {
         goto go_it;
      }
   }

   /* default */

   s = "/projects/cad";

   go_it:

   len = strlen( s );
   cad_lib = Valloc( len + 5, 1 );
   cad_bin = Valloc( len + 5, 1 );
   ( void ) sprintf( cad_lib, "%s/lib", s );
   ( void ) sprintf( cad_bin, "%s/bin", s );
}
