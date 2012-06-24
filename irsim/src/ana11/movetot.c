/***** Copyright (C) 1989 Stanford University, Arturo Salz ****/
#include <stdlib.h>
#include "ana.h"
#include "ana_glob.h"


private void MoveToT( char  *str ) {
   TimeType  start;
   double    tmp;

   if( str == NULL ) {
      XBell( display, 0 );
      return;
   }
   tmp = atof( str );
   start = ( int ) ns2d( tmp );
   if( start < tims.first or start > tims.last or start == tims.start )
      return;

   tims.start = start;
   tims.end = start + tims.steps;
   RedrawTimes();
   UpdateScrollBar();
   DrawTraces( start, tims.end );
}


public void MoveToTime( char  *s ) {
   
   Query( "\nEnter Time > ", (void (*)(void*)) MoveToT );
}
