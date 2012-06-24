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
#include <string.h>
#include <stdio.h>
#include "ana.h"
#include "ana_glob.h"
#include "graphics.h"


#define	PDMENU_TOP		( bannerBox.bot + 1 )


private MenuItem zoom_menu[] = {
   { "in",  (void (*)(void*)) Zoom },
   { "out", (void (*)(void*)) Zoom },
   { NULL }
};

private MenuItem base_menu[] = {
   { "bin", (void (*)(void*)) ChangeBase },
   { "oct", (void (*)(void*)) ChangeBase },
   { "hex", (void (*)(void*)) ChangeBase },
   { NULL }
};

private MenuItem window_menu[] = {
   { "delta T",      (void (*)(void*)) DeltaT },
   { "move to",      (void (*)(void*)) MoveToTime },
   { "set width",    (void (*)(void*)) SetWidth },
   { "name length",  (void (*)(void*)) SetNameLen },
   { "+scroll",      (void (*)(void*)) ScrollUpdate },
   { NULL }
};

private MenuItem print_menu[] = {
   { "file",     (void (*)(void*)) printPS },
   { "+banner",  (void (*)(void*)) SetPSParms },
   { "-legend",  (void (*)(void*)) SetPSParms },
   { "+times",   (void (*)(void*)) SetPSParms },
   { "+outline", (void (*)(void*)) SetPSParms },
   { NULL }
};

public	Menu menu[] = {
   { "zoom",    zoom_menu },
   { "base",    base_menu },
   { "window",  window_menu },
   { "print",   print_menu },
   {  NULL,     NULL }
};


private	int  inMenu;		/* TRUE if mouse is in menu window */


#define PTinBOX( px, py, BOX )						\
    ( ( (py > (BOX).bot) || (py < (BOX).top ) ||			\
      (px > (BOX).right) || (px < (BOX).left ) ) ? 0 : 1 )


#define	ITEMHGT( ITM ) 	     ( (ITM)->bot - (ITM)->top + 1 )


public void InitMenus() {
   Menu      *mp;
   MenuItem  *item;
   int        len, maxlen;
   Coord      ypos;

   for( mp = menu; mp->str != NULL; mp++ ) {
      mp->len = strlen( mp->str );
      maxlen = 0;
      ypos = 0;
      for( item = mp->items; item->str != NULL; item++ ) {
         len = strlen( item->str );
         if( len > maxlen )
            maxlen = len;
         item->len = len;
         item->top = ypos;
         item->bot = ypos + CHARHEIGHT + 1;
         ypos += CHARHEIGHT + 2;
      }
      mp->width = CHARWIDTH * ( maxlen + 2 );
      mp->height = ypos;

      if( mp->w == 0 ) {
         XSetWindowAttributes  att;
         unsigned long         mask;

         att.background_pixel = colors.white;
         att.border_pixel = colors.black;
         att.save_under = True;
         att.override_redirect = True;
         mask = CWBackPixel|CWBorderPixel|CWOverrideRedirect|CWSaveUnder;

         mp->w = XCreateWindow( display, RootWindowOfScreen( screen ),
                                0, 0, mp->width, mp->height, 1, DefaultDepthOfScreen( screen ),
                                InputOutput, ( Visual * ) CopyFromParent, mask, &att );

         XSelectInput( display, mp->w, ExposureMask | EnterWindowMask |
                       LeaveWindowMask | ButtonMotionMask | ButtonPressMask |
                       ButtonReleaseMask );
      }
   }
}


private void PutUpMenu( Menu  *m ) {
   Coord   x, y, sx, sy;
   Window  w;

   sy = PDMENU_TOP;
   sx = ( m->box.right + m->box.left - m->width ) / 2 + 1;
   XTranslateCoordinates( display, window, RootWindowOfScreen( screen ),
                          sx, sy, &x, &y, &w );
   if( x < 0 )
      x = 0;
   else if( x + m->width > WidthOfScreen( screen ) )
      x = WidthOfScreen( screen ) - m->width;

   if( x < 0 )
      y = 0;
   else if( y + m->height > HeightOfScreen( screen ) )
      y = HeightOfScreen( screen ) - m->height;

   XMoveWindow( display, m->w, x, y );
   XMapRaised( display, m->w );
   GrabMouse( m->w, EnterWindowMask | LeaveWindowMask | ButtonMotionMask |
              ButtonPressMask | ButtonReleaseMask, cursors.deflt );
   XFlush( display );
}


private void DrawMenu( Menu  *m ) {
   MenuItem  *item;
   const char      *s;
   int       len;

   for( item = m->items; item->str != NULL; item++ ) {
      s = item->str;
      len = item->len;
      if( *s == MENU_UNMARK ) {
         s++;
         len--;
      } else if( *s == MENU_MARK ) {
         s++;
         len--;
         XCopyArea( display, pix.chk, m->w, gcs.black, 0, 0, 6, 8, 1,
                    ( item->top + item->bot - 8 )/2 );
      }

      StrCenter( m->w, s, len, 0, m->width - 1, item->bot - 1, gcs.black );
   }
}


private MenuItem *MouseMoved( Menu * m, MenuItem * sel, Coord y ) {
   MenuItem  *newMenu;

   if( not inMenu or y < 0 or y > m->height )
      newMenu = NULL;
   else
      for( newMenu = m->items; y > newMenu->bot; newMenu++ );

   if( newMenu != sel ) {
      if( sel != NULL )
         InvAREA( m->w, 0, sel->top, m->width, ITEMHGT( sel ) );
      if( newMenu != NULL )
         InvAREA( m->w, 0, newMenu->top, m->width, ITEMHGT( newMenu ) );
   }
   return( newMenu );
}


public void DoMenu( Coord x, Coord y ) {
   XEvent    ev;
   Menu      *m;
   MenuItem  *select;
   int       button_down;

   for( m = menu; m->str != NULL; m++ ) {
      if( PTinBOX( x, y, m->box ) )
         break;
   }

   if( m->str == NULL )	/* should never happen */
      return;

   InvBox( window, m->box );
   PutUpMenu( m );

   inMenu = FALSE;
   select = NULL;
   button_down = TRUE;

   while( button_down ) {
      XNextEvent( display, &ev );
      switch( ev.type ) {
      case Expose :
         if( ev.xexpose.window == m->w )
            DrawMenu( m );
         break;

      case EnterNotify :
         if( ev.xcrossing.window == m->w )
            inMenu = TRUE;
         x = ev.xcrossing.x;
         y = ( x < 0 or x > m->width ) ? -1 : ev.xcrossing.y;
         select = MouseMoved( m, select, y );
         break;

      case LeaveNotify :
         if( ev.xcrossing.window == m->w )
            inMenu = FALSE;
         select = MouseMoved( m, select, ev.xcrossing.y );
         break;

      case MotionNotify :
         if( ev.xmotion.window == m->w )
            select = MouseMoved( m, select, ev.xmotion.y );
         break;

      case ButtonRelease :
         button_down = FALSE;
         break;

      default :
         ;
      }
   }
   if( select != NULL )
      InvAREA( m->w, 0, select->top, m->width, ITEMHGT( select ) );
   XUngrabPointer( display, CurrentTime );
   XUnmapWindow( display, m->w );
   XFlush( display );
   InvBox( window, m->box );
   if( select != NULL ) {
      void * x = (void *) select->str;
      ( *select->func )( x );
   }
}
