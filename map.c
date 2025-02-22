/* KERNEL, 20/7/87 -- S. Hekmatpour
 * map functions:
 */
#include "kernel.h"

kerncell
Vmapa ()  /* -------------------------------- (mapa 'fun 'list1 ... 'listn) */
{
   kerncell fun = ARGnum1;
   int	    arg1 = ARGidx1 + 1;
   int	    argn = argtop;
   register int i;
   kerncell res = argstk[arg1];

	CHECKvargs1(mapasym,2);			/* at least 2 args required */
	for (i=arg1; i < argn; ++i)
	    CHECKlist(mapasym,argstk[i]);
	ARGpush(fun);
	for (;;) {				    /* map over the list(s) */
	    if (argstk[arg1] == NIL)
	       break;
	    for (i=arg1; i < argn; ++i) {		    /* prepare args */
		ARGpush(argstk[i]->CELLcar);
		argstk[i] = argstk[i]->CELLcdr;
	    }
	    ARGpush(CONVcell(argn + 1));
	    Vcall();				      /* apply the function */
	    argtop = argn + 1;			   /* keep fun on the stack */
	}
	argtop = argn;					  /* restore argtop */
	return(res);
} /* Vmapa */

kerncell
Vmapcar ()  /* ---------------------------- (mapcar 'fun 'list1 ... 'listn) */
{
   kerncell fun = ARGnum1;
   kerncell res = NIL;
   int	    arg1 = ARGidx1 + 1;
   int	    argn = argtop;
   register int i;

	CHECKvargs1(mapcarsym,2);		/* at least 2 args required */
	for (i=arg1; i < argn; ++i)
	    CHECKlist(mapcarsym,argstk[i]);
	ARGpush(fun);
	for (;;) {				    /* map over the list(s) */
	    if (argstk[arg1] == NIL)
	       break;
	    for (i=arg1; i < argn; ++i) {		    /* prepare args */
		ARGpush(argstk[i]->CELLcar);
		argstk[i] = argstk[i]->CELLcdr;
	    }
	    ARGpush(CONVcell(argn + 1));
	    res = mkcell(Vcall(),res);		      /* apply the function */
	    argtop = argn + 1;			   /* keep fun on the stack */
	}
	argtop = argn;					  /* restore argtop */
	return(dreverse(res));
} /* Vmapcar */

kerncell
Vmapd ()  /* -------------------------------- (mapd 'fun 'list1 ... 'listn) */
{
   kerncell fun = ARGnum1;
   int	    arg1 = ARGidx1 + 1;
   int	    argn = argtop;
   register int i;
   kerncell res = argstk[arg1];

	CHECKvargs1(mapdsym,2);			/* at least 2 args required */
	for (i=arg1; i < argn; ++i)
	    CHECKlist(mapdsym,argstk[i]);
	ARGpush(fun);
	for (;;) {				    /* map over the list(s) */
	    if (argstk[arg1] == NIL)
	       break;
	    for (i=arg1; i < argn; ++i) {		    /* prepare args */
		ARGpush(argstk[i]);
		argstk[i] = argstk[i]->CELLcdr;
	    }
	    ARGpush(CONVcell(argn + 1));
	    Vcall();				      /* apply the function */
	    argtop = argn + 1;			   /* keep fun on the stack */
	}
	argtop = argn;					  /* restore argtop */
	return(res);
} /* Vmapd */

kerncell
Vmapcdr ()  /* ---------------------------- (mapcdr 'fun 'list1 ... 'listn) */
{
   kerncell fun = ARGnum1;
   kerncell res = NIL;
   int	    arg1 = ARGidx1 + 1;
   int	    argn = argtop;
   register int i;

	CHECKvargs1(mapcdrsym,2);		/* at least 2 args required */
	for (i=arg1; i < argn; ++i)
	    CHECKlist(mapcdrsym,argstk[i]);
	ARGpush(fun);
	for (;;) {				    /* map over the list(s) */
	    if (argstk[arg1] == NIL)
	       break;
	    for (i=arg1; i < argn; ++i) {		    /* prepare args */
		ARGpush(argstk[i]);
		argstk[i] = argstk[i]->CELLcdr;
	    }
	    ARGpush(CONVcell(argn + 1));
	    res = mkcell(Vcall(),res);		      /* apply the function */
	    argtop = argn + 1;			   /* keep fun on the stack */
	}
	argtop = argn;					  /* restore argtop */
	return(dreverse(res));
} /* Vmapcdr */
