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
 * Helper process to simulate SIGIO signals from the X server to irsim's
 * analyzer.  This problem is mostly due to ultrix's inability to generate
 * SIGIO for anything other than tty's "Gumby says Ultrix is snake oil".  It
 * isn't needed for bsd - 4.3.
 *
 * The strategy is to wait for input from the X server by doing a select on
 * the server's file descriptor, which is set to be 0 by irsim, and sending
 * a SIGIO signal to the parent process whenever there's input to be read.
 * Handshaking is accomplished by waiting for a SIGINT signal from irsim.
 * This handshaking is needed since we don't know when the parent process
 * will decide to read the events and thus we avoid sending SIGIO signals
 * until irsim reads all events from the queue.
 */
#include <unistd.h>
#include <signal.h>
#ifdef SYS_V
#    ifndef hpux
#        define signal( SIG, HAND )	sigset( SIG, HAND )
#    endif
#    include <termio.h>
#else
#    include <sys/ioctl.h>
#endif
#include <fcntl.h>


/*
 * SIGINT handler : Do nothing, just terminate pause
 */
void handler( int foo ) {
   #ifdef SYS_V
   signal( SIGINT, handler );
   #endif
}


int main( int argc, char ** argv ) {
   int  parent;		/* parent process */
   fd_set  fdset;			/* fd-bit-set to check: fd == 0 always */
   int  f;
   
   FD_ZERO( &fdset );
   FD_SET( 0, &fdset );

   for( f = 1; f < 15; f++ )
      ( void ) close( f );

   #ifdef TIOCNOTTY
   f = open( "/dev/tty", O_RDWR );	/* disconnect from terminal */
   if( f > 0 ) {
      ioctl( f, TIOCNOTTY, 0 );
      ( void ) close( f );
   }
   #endif

   parent = getppid();
   signal( SIGINT, handler );
   signal( SIGUSR1, SIG_IGN );

   do {
      // fdset = 1;
      FD_SET( 0, &fdset );
      if( select( 1, &fdset, 0, 0, 0 ) != 1 ) {
         continue;
      }
      if( kill( parent, SIGIO ) != 0 ) {
         /* parent died */
         break;
      }
      ( void ) sigpause( 0 );
   } while( 1 );

   return 0;
}
