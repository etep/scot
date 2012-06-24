#ifndef __IRSIM_MEM_DOT_H__
#define __IRSIM_MEM_DOT_H__

char * Valloc( int nbytes, int no_mem_exit );
void   Vfree( void * ptr );

#endif