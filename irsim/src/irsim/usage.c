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

#ifdef OS2
#define SYS_V
#endif  /* OS2 */
#ifdef SYS_V

#include <stdio.h>
#include <sys/types.h>
#ifndef OS2
#include <sys/times.h>
#include <sys/param.h>
#else
#include <sys/time.h>
#include <time.h>
#include <sys/timeb.h>
#define HZ 60
#endif  /* OS2 */
#include "defs.h"


/* time and usage at program start */
private	time_t      time0;
#ifdef OS2
private struct timeb ru0;
#else
private struct tms  ru0;
#endif  /* OS2 */
/* time and usage before command is executed */
private	time_t      time1, t_usr;
#ifdef OS2
private struct timeb ru1, ru_usr;
#else
private struct tms  ru1, ru_usr;
#endif  /* OS2 */
private	long mem0;


public void InitUsage() {
   time0 = time( 0 );
   #ifndef OS2
   ( void ) times( &ru0 );
   mem0 = ( long ) sbrk( 0 );
   #endif  /* OS2 */
}

public void set_usage() {
   time1 = time( 0 );
   #ifndef OS2
   ( void ) times( &ru1 );
   #endif  /* OS2 */
}

public void uset_usage() {
   t_usr = time( 0 );
   #ifndef OS2
   ( void ) times( &ru_usr );
   #endif  /* OS2 */
}


private char *pr_secs( char * dst, long l ) {
   register int  i;

   i = l / 3600;
   if( i != 0 ) {
      ( void ) sprintf( dst, "%d:%02d", i, ( l % 3600 ) / 60 );
      i = l % 3600;
   } else {
      i = l;
      ( void ) sprintf( dst, "%d", i / 60 );
   }
   while( *++dst );

   i %= 60;
   *dst++ = ':';
   ( void ) sprintf( dst, "%02d ", i );
   dst += 3;
   return( dst );
}


private void pr_usage( char * dst, timeb * r0, timeb * r1, time_t * t0, time_t * t1 ) {
   // register char         *dst;
   // #ifdef OS2
   // struct timeb *r0, *r1;
   // #else
   // register struct  tms  *r0, *r1;
   // #endif  /* OS2 */
   // register time_t       *t0, *t1;

   register time_t  t, dt;
   int      mem;

#ifdef OS2
   dt = r1->millitm - r0->millitm;
#else
   dt = r1->tms_utime - r0->tms_utime;
#endif  /* OS2 */
   ( void ) sprintf( dst, "%d.%01du ", dt / HZ, dt / ( HZ / 10 ) );
   while( *++dst );

#ifdef OS2
   dt = r1->millitm - r0->millitm;
#else
   dt = r1->tms_stime - r0->tms_stime;
#endif  /* OS2 */
   ( void ) sprintf( dst, "%d.%01ds ", dt / HZ, dt / ( HZ / 10 ) );
   while( *++dst );

   t = *t1 - *t0;
   dst = pr_secs( dst, ( long ) t );

#ifdef OS2
   dt = r1->millitm - r0->millitm /* + r1->tms_stime - r0->tms_stime */;
#else
   dt = r1->tms_utime - r0->tms_utime + r1->tms_stime - r0->tms_stime;
#endif  /* OS2 */
   t *= HZ;

   ( void ) sprintf( dst, "%d%% ", ( int ) ( dt * 100 / ( ( t ? t : 1 ) ) ) );
   while( *++dst );

#ifndef OS2
   mem = sbrk( 0 ) - mem0;
#endif  /* OS2 */
   ( void ) sprintf( dst, "%dK\n", mem / 1024 );
}


public void print_usage( int partial, char * dest ) {
   time_t      time2;
   #ifdef OS2
   struct timeb ru2;
   #else
   struct tms  ru2;
   #endif  /* OS2 */

   time2 = time( 0 );
   #ifndef OS2
   ( void ) times( &ru2 );
   #endif  /* OS2 */
   if( partial ) {
      pr_usage( dest, &ru1, &ru2, &time1, &time2 );
   }
   else {
      pr_usage( dest, &ru0, &ru2, &time0, &time2 );
   }
}


public void get_usage( char * dest ) {
   time_t           time2;
   #ifdef OS2
   struct timeb     ru2;
   #else
   struct tms       ru2;
   #endif  /* OS2 */
   register time_t  dt;
   time_t           msu, mss, ms;

   time2 = time( 0 );
   #ifndef OS2
   ( void ) times( &ru2 );
   #endif  /* OS2 */
   #ifdef OS2
   dt = ru2.millitm - ru_usr.millitm;
   #else
   dt = ru2.tms_utime - ru_usr.tms_utime;
   #endif  /* OS2 */
   msu = dt * 1000 / HZ;
   #ifndef OS2
   dt = ru2.tms_stime - ru_usr.tms_stime;
   #endif  /* OS2 */
   mss = dt * 1000 / HZ;

   dt = ( time2 - t_usr );
   ( void ) sprintf( dest, "%ldu %lds %ldsec", msu, mss, dt );
}

#else			/* BSD */

#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "defs.h"


/* time and usage at program start */
private	struct timeval    time0;
private struct rusage     ru0;

/* time and usage before command is executed */
private	struct timeval    time1, t_usr;
private struct rusage     ru1, ru_usr;


public void InitUsage() {
   ( void ) gettimeofday( &time0, ( struct timezone * ) 0 );
   ( void ) getrusage( RUSAGE_SELF, &ru0 );
}


public void set_usage() {
   ( void ) gettimeofday( &time1, ( struct timezone * ) 0 );
   ( void ) getrusage( RUSAGE_SELF, &ru1 );
}

public void uset_usage() {
   ( void ) gettimeofday( &t_usr, ( struct timezone * ) 0 );
   ( void ) getrusage( RUSAGE_SELF, &ru_usr );
}


private void tvsub( timeval * tdiff, timeval * t1, timeval * t0 ) {
   // struct timeval *tdiff, *t1, *t0;
   
   tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
   tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
   if ( tdiff->tv_usec < 0 ) {
      tdiff->tv_sec--;
      tdiff->tv_usec += 1000000;
   }
}


private char *pr_secs( char * dst, long l ) {
   register int  i;

   i = l / 3600;
   if( i != 0 ) {
      ( void ) sprintf( dst, "%d:%02ld", i, ( l % 3600 ) / 60 );
      i = l % 3600;
   } else {
      i = l;
      ( void ) sprintf( dst, "%d", i / 60 );
   }
   while( *++dst );

   i %= 60;
   *dst++ = ':';
   ( void ) sprintf( dst, "%02d ", i );
   dst += 3;
   return( dst );
}


#define	u2m( A )	( (A) / 10000 )		/* usec to msec */
#define	u2d( A )	( (A) / 100000 )	/* usec to 10th sec */

private void pr_usage( char * dst, rusage * r0, rusage * r1, timeval * t0, timeval * t1 ) {
   // register char            *dst;
   // register struct rusage   *r0, *r1;
   // register struct timeval  *t0, *t1;

   register time_t  t;
   struct timeval   dt;
   int              ms;

   tvsub( &dt, &r1->ru_utime, &r0->ru_utime );
   ( void ) sprintf( dst, "%d.%01du ", (int)dt.tv_sec, u2d( dt.tv_usec ) );
   while( *++dst );

   tvsub( &dt, &r1->ru_stime, &r0->ru_stime );
   ( void ) sprintf( dst, "%d.%01ds ", (int)dt.tv_sec, u2d( dt.tv_usec ) );
   while( *++dst );

   ms = ( t1->tv_sec - t0->tv_sec ) * 100 + u2m( t1->tv_usec - t0->tv_usec );
   dst = pr_secs( dst, ( long ) ( ms / 100 ) );

   t = ( r1->ru_utime.tv_sec - r0->ru_utime.tv_sec ) * 100 +
       u2m( r1->ru_utime.tv_usec - r0->ru_utime.tv_usec ) +
       ( r1->ru_stime.tv_sec - r0->ru_stime.tv_sec ) * 100 +
       u2m( r1->ru_stime.tv_usec - r0->ru_stime.tv_usec );

   ( void ) sprintf( dst, "%d%% ", ( int ) ( t * 100 / ( ( ms ? ms : 1 ) ) ) );
   while( *++dst );

   ( void ) sprintf( dst, "%ldK\n", r1->ru_maxrss / 2 );
}


public void print_usage( int partial, char * dest ) {
   struct timeval  time2;
   struct rusage   ru2;

   ( void ) gettimeofday( &time2, ( struct timezone * ) 0 );
   ( void ) getrusage( RUSAGE_SELF, &ru2 );

   if( partial )
      pr_usage( dest, &ru1, &ru2, &time1, &time2 );
   else
      pr_usage( dest, &ru0, &ru2, &time0, &time2 );
}


public void get_usage( char * dest ) {
   struct timeval  t2;
   struct rusage   ru2;
   struct timeval  dt;
   long            msu, mss, ms;

   ( void ) gettimeofday( &t2, ( struct timezone * ) 0 );
   ( void ) getrusage( RUSAGE_SELF, &ru2 );

   tvsub( &dt, &ru2.ru_utime, &ru_usr.ru_utime );
   msu = dt.tv_sec * 1000 + ( dt.tv_usec / 1000 );

   tvsub( &dt, &ru2.ru_stime, &ru_usr.ru_stime );
   mss = dt.tv_sec * 1000 + ( dt.tv_usec / 1000 );

   tvsub( &dt, &t2, &t_usr );
   ms = dt.tv_sec * 1000 + ( dt.tv_usec / 1000 );

   ( void ) sprintf( dest, "%ldu %lds %ld", msu, mss, ms );
}

#endif SYS_V
