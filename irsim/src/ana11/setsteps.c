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
#include <stdlib.h>
#include "ana.h"
#include "ana_glob.h"


private void GetWidth( char  *str ) {
   TimeType  steps;
   double    tmp;

   if( str == NULL ) {
      XBell( display, 0 );
      return;
   }
   tmp = atof( str );
   steps = ns2d( tmp );
   if( steps < 10 or ( tims.start + steps ) > max_time ) {
      XBell( display, 0 );
      return;
   }

   tims.steps = steps;
   tims.end = tims.start + steps;
   RedrawTimes();
   UpdateScrollBar();
   DrawTraces( tims.start, tims.end );
}


public void SetWidth( char  *s ) {
   Query( "\nEnter Time Steps > ", (void (*)(void*)) GetWidth );
}
