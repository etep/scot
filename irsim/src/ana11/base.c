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

#include "ana.h"
#include "ana_glob.h"
#include "graphics.h"


public void ChangeBase( char * base ) {
   int    change;
   short  bdigits;

   if( not selectedTrace ) {
      PRINT( "\nSelect a trace first!" );
      XBell( display, 0 );
      return;
   }

   switch( *base ) {
   case 'b' :
      bdigits = 1;
      break;
   case 'o' :
      bdigits = 3;
      break;
   case 'h' :
      bdigits = 4;
      break;
   }

   if( IsVector( selectedTrace ) and selectedTrace->bdigit != bdigits ) {
      selectedTrace->bdigit = bdigits;
      change = WindowChanges();
      if( change & RESIZED )
         return;		/* will get ExposeWindow event later */

      if( change & WIDTH_CHANGE ) {	/* reshape the trace window */
         DrawScrollBar( FALSE );
         RedrawTimes();
         DrawCursVal( cursorBox );
         DrawTraces( tims.start, tims.end );
      } else {
         BBox  rb;

         rb.top = selectedTrace->top;
         rb.bot = selectedTrace->bot;
         rb.left = cursorBox.left;
         rb.right = cursorBox.right;
         DrawCursVal( rb );
         rb.left = traceBox.left;
         rb.right = traceBox.right;
         RedrawTraces( &rb );		/* just redraw this trace */
      }

   }
}
