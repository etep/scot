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

private	char    result[ 200 ];
private	char    HexMap[] = "0123456789abcdefX";


/*
 * Convert a trace entry (vector) to an ascii string
 */
public char *HistToStr( hptr * hist, int nbits, int b_digit, int offset ) {
    register char  *p; 
    register int   i, j, digit;

    p = result;
    j = nbits % b_digit;
    if( j == 0 )
	j = b_digit;
    for( i = nbits; i > 0; i -= j )
      {
	digit = 0;
	do
	  {
	    switch( (*hist)->val )
	      {
		case LOW :
		    digit = (digit << 1);
		    break;
		case HIGH :
		    digit = (digit << 1) | 1;
		    break;
		case X :
		    digit = 16;
		    while( j != 1 )
		      {
			j--;
			hist += offset;
		      }
		    break;
	      }
	    j--;
	    hist += offset;
	  }
	while( j > 0 );
	*p++ = HexMap[ digit ];
	j = b_digit;
      }
    *p = '\0';
    return( result );
  }
