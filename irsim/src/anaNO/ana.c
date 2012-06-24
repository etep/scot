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
#include "defs.h"
#include "net.h"


extern void lprintf();


public typedef long TimeType;


public int AddNode( nptr  nd, int *flag ) {
   return( 1 );
}

public int AddVector( bptr  vec, int *flag ) {
   return( 1 );
}

public void DisplayTraces( int isMapped )
{}

public void StopAnalyzer()
{}

public void RestartAnalyzer( long first_time, long last_time, int same_hist ) {}

public void ClearTraces()
{}

public void RemoveVector ( bptr  b );
void RemoveVector ( bptr  b ) {}

public void RemoveNode   ( nptr  n );
void RemoveNode   ( nptr  n ) {}

public void RemoveAllDeleted();
void RemoveAllDeleted() {}

public void UpdateWindow( TimeType endT );
void UpdateWindow( TimeType endT ) {}

public void TerminateAnalyzer()
{}

public int InitDisplay( char * fname, char * display_unit ) {
   ( void ) lprintf( stdout, "No analyzer in this version\n" );
   return( FALSE );
}

private void InitTimes( TimeType firstT, TimeType stepsize, TimeType lastT )
// TimeType  firstT, stepsize, lastT;
{}
