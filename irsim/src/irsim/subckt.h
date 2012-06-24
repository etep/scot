
#ifdef USER_SUBCKT 
#ifndef LOW
typedef char  *uptr;
typedef uptr (*ufun)();

#define LOW             0       /* low low */
#define X               1       /* unknown, intermediate, ... value */
#define X_X             2
#define HIGH            3       /* logic high */
#define N_POTS          4       /* number of potentials [LOW-HIGH] */
#define        d2ns( D )               ( (D) * 0.01 ) 
#define        d2ps( D )               ( (D) * 10.0 )

extern	long   cur_delta;

#endif


typedef struct 
{
    char	*name;			/* name of this sub-circuit */
    ufun	model;			/* function that models sub-circuit */
    ufun	init;			/* Function to call to init */
} userSubCircuit;

extern char pot2char[] ;
#endif



