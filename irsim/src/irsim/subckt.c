#include <stdio.h>
#include "defs.h"
#include "net.h"
#include "globals.h"
#include "subckt.h"

int unused;

#ifdef USER_SUBCKT

typedef struct _Sub {
   char	*name;			/* name of this sub-circuit */
   ufun	model;			/* function that models sub-circuit */
   ufun	init;			/* Function to call to init */
   int		inst;			/* Number of instantiations */
   struct _Sub	*next;			/* list of commands in bucket */
} SubCircuit;

char pot2char[N_POTS] = { 'L' , 'x', 'X' , 'H' } ;

#define SUBSTBLSIZE	16

private SubCircuit *substbl[SUBSTBLSIZE];


extern	userSubCircuit	subs[];


private int subs_cmp( SubCircuit *a, SubCircuit *b ) {
   return( str_eql( a->name, b->name ) );
}


public void init_subs() {
   register int n;
   register userSubCircuit *s;
   register SubCircuit *sl;

   for ( n=0; n < SUBSTBLSIZE; n++ )
      substbl[n] = NULL;
   for ( s=subs; s->name != NULL; s += 1 ) {
      sl = ( SubCircuit * ) malloc( sizeof( SubCircuit ) );
      sl->name = s->name;
      sl->model = s->model;
      sl->init = s->init;
      sl->inst = 0 ;
      n = HashSub( s->name );
      sl->next = substbl[n];
      substbl[n] = sl;
   }
}


private int HashSub( char  *name ) {
   register int  hashcode = 0;

   do
      hashcode = ( hashcode << 1 ) ^ ( *name | 0x20 );
   while( *( ++name ) );
   return( ( ( hashcode >= 0 ) ? hashcode : ~hashcode ) % SUBSTBLSIZE );
}

public	ufun subckt_instantiate( char * sname, ufun * initfunP, int * instP ) {
   // char *sname;
   // ufun *initfunP;
   // int  *instP;

   register int n;
   register SubCircuit *s;

   n = HashSub( sname );

   for ( s = substbl[n]; s != NULL; s = s->next ) {
      if ( str_eql( s->name, sname ) == 0 ) {
         *initfunP = s->init;
         *instP = s->inst ;
         s->inst++;
         return( s->model );
      }
   }
   return( ( ufun )NULL );


}


#define INVERT(x)	((x == LOW) ? HIGH : ((x == HIGH) ? LOW : X))

public void subckt_model_C( tptr t ) {
   int i;
   register short nins = t->subptr->nins;
   register short nouts = t->subptr->nouts;
   register nptr *nodes = t->subptr->nodes;
   uptr udata = t->subptr->udata;

   char *in, *out;
   double *delay;

   in  = ( char * ) malloc( nins*sizeof( char ) );
   out = ( char * ) malloc( nouts*sizeof( char ) );
   delay = ( double * ) malloc( nouts*sizeof( double ) );
   for ( i = 0; i < nins; i++ )
      in[i] = nodes[i]->npot;
   for ( i = 0; i < nouts; i++ )
      out[i] = nodes[i+nins]->npot;
   ( *t->subptr->model )( in, out, delay, udata );
   for ( i = 0; i < nouts; i++ ) {
      if ( out[i] != X_X ) {
         QueueFVal( nodes[i+nins],INVERT( out[i] ),
                    ( double ) 1.0, delay[i] );
         QueueFVal( nodes[i+nins+nouts],INVERT( out[i] ),
                    ( double ) 1.0, delay[i] );
      } else {
         QueueFVal( nodes[i+nins], HIGH,
                    ( double ) 1.0, delay[i] );
         QueueFVal( nodes[i+nins+nouts], LOW,
                    ( double ) 1.0, delay[i] );
      }
   }
   free( in );
   free( out );
   free( delay );
}
#endif
