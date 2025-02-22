/* KERNEL, 9/7/87 -- S. Hekmatpour
 * Non-conventional flow of control functions:
 * These include the basic catch and throw functions,
 * error handling functions, and the top level functions.
 */
#include "kernel.h"
#include <setjmp.h>

#define CATpush()				\
	if (++cattop < CATSTKSIZE) {		\
	   catstk[cattop].evaltop = evaltop;	\
	   catstk[cattop].celltop = celltop;	\
	   catstk[cattop].vartop  = vartop;	\
	   catstk[cattop].argtop  = argtop;	\
	   catstk[cattop]._argtop = _argtop;	\
	} else faterr(err_catstk)
#define CATpop()		--cattop

struct catframe {	/* catch frame */
	jmp_buf jmp;		/* for setjmp and longjmp */
	int	evaltop;	/* evaltop at the time of setjmp */
	int	celltop;	/* celltop at the time of setjmp */
	int	vartop;		/* vartop at the time of setjmp */
	int	argtop;		/* argtop at the time of setjmp */
	int	_argtop;	/* argtop of the last vlam */
};
struct catframe catstk[CATSTKSIZE];   /* the catch stack */
int cattop = -1;		/* top of jump stack */
int errtrap = 0;		/* no error capture when zero */
int errshow = 1;		/* errors are reported when non-zero */
int errocc = 0;			/* set when an error occurs */
int level = 0;			/* kernel level */

kerncell
Ucatch ()  /* ---------------------------------------- (catch 'expr ['tag]) */
{
   kerncell list = argstk[argtop];

	if (list == NIL || list->CELLcdr->CELLcdr != NIL)
	   error(catchsym,err_args,NULL);
	return(catch(list->CELLcar,eval(list->CELLcdr->CELLcar),NULL));
} /* Ucatch */

kerncell
catch (expr,tag,more)  /* ------------------ catch throws during evaluation */
kerncell expr, tag;
word	 more;
{
	CATpush();
	/* get ready for throws: */
	if (!CONVcell(setjmp(catstk[cattop].jmp)))
	   if (more.i)
	   {  kerncell (* cfun)() = (kerncell (*)()) expr;
	      expr = (* cfun)(more);		    /* cfun may have throws */
	   }
	   else
	      expr = eval(expr);		 /* expr may contain throws */
	else if (tag == NIL && !ISinternal(CONVsym(catres->CELLcar))
		 || catres->CELLcar == NIL && !ISinternal(tag) && !ISlist(tag)
		 || catres->CELLcar == tag
		 || ISlist(tag) && memq(catres->CELLcar,tag)) {
	   cleanup();
	   CATpop();				     /* catch the throw and */
	   return(catres->CELLcdr);		       /* return its result */
	}
	else if (cattop < 1)	     /* one catch is reserved for top level */
	   error(catchsym,"no catch for this tag",catres->CELLcar);
	else {
	   cleanup();
	   longjmp(catstk[CATpop()].jmp,catres);       /* try another catch */
	}
	CATpop();		 /* there was no throw, so ignore the catch */
	return(expr);
} /* catch */

cleanup ()  /* ------------------------------ clean up stacks after a throw */
{
   register int vtop = catstk[cattop].vartop;

	while (vtop < vartop)			 /* restore non-global vars */
	   VARpop();
	evaltop = catstk[cattop].evaltop;	      /* restore eval stack */
	celltop = catstk[cattop].celltop;	      /* restore eval stack */
	argtop  = catstk[cattop].argtop;	       /* restore arg stack */
	_argtop = catstk[cattop]._argtop;		 /* restore _argtop */
} /* cleanup */

kerncell
Vthrow ()  /* ----------------------------------------- (throw 'obj ['tag]) */
{
	CHECKvargs(throwsym,1,2);
	return(throw(ARGnum1,(argtop - ARGidx1 == 1 ? NIL : ARGnum2)));
} /* Vthrow */

kerncell
throw (expr,tag)  /* ---------------- evaluate and throw expr, plus its tag */
kerncell expr, tag;
{
	catres->CELLcar = tag;
	catres->CELLcdr = expr;
	longjmp(catstk[cattop].jmp,catres);
} /* throw */

kerncell
Ucaperr ()  /* -------------------------------------- (caperr 'expr [hide]) */
{
   kerncell list = argstk[argtop];

	if (list == NIL || list->CELLcdr->CELLcdr != NIL)
	   error(caperrsym,err_args,NULL);
	return(caperr(list->CELLcar,list->CELLcdr->CELLcar,NULL));
} /* Ucaperr */

kerncell
caperr (expr,hide,more)  /* ----- captures errors during evaluation of expr */
kerncell expr, hide;	  /* when hide is non-nil error messages are hidden */
word	 more;
{
   int savetrap = errtrap;		     /* save the values of errtrap, */
   int saveshow = errshow;				    /* errshow, and */
   int saveocc  = errocc;					  /* errocc */
   kerncell res;

	errtrap = 1;			     /* come here when error occurs */
	errshow = eval(hide) == NIL;	    /* activate/hide error messages */
	errocc = 0;		   /* pretend there were no previous errors */
	res = catch(expr,_errtagsym,more);		   /* evaluate expr */
	/* return nil if error occurs, and a list of res otherwise: */
	res = (errocc ? NIL : mkcell(res,NIL));
	errtrap = savetrap;		   /* restore the things we changed */
	errshow = saveshow;
	errocc = saveocc;
	return(res);
} /* caperr */

kerncell
Verror ()  /* --------------------------- (error 'source 'message ['extra]) */
{
   kerncell arg2;

	CHECKvargs(errorsym,2,3);
	arg2 = ARGnum2;
	error(ARGnum1,GETstr(errorsym,arg2),
		      (argtop - ARGidx1 == 2 ? NULL : ARGnum3));
	return(TTT);
} /* Verror */

error (source,message,extra)  /* ------------------- error handling routine */
kerncell source;
char *message;
kerncell extra;
{
	errocc = 1;					  /* set error flag */
	if (errshow) {
	   bufprint(PRINT,_errchan,"ERROR, ");
	   if (source != NULL) {
	      PRINTchan(source,errchan);
	      bufprint(PRINT,_errchan,": ");
	   }
	   bufprint(PRINT,_errchan,"%s",message);
	   if (extra != NULL) {
	      bufprint(PRINT,_errchan,": ");
	      PRINTchan(extra,errchan);
	   }
	   bufprint(PRINT,_errchan,"\n");
	}
	if (errtrap)
	   throw(NIL,_errtagsym);      /* throw this to the catch of caperr */
	else
	   EVALpush(CONVcell(errorsym));
	errlevel();				       /* enter error level */
} /* error */

errlevel ()  /* ----------------------------- error level's read-eval-print */
{
   kerncell obj;

	++level;				     /* increment the level */
	for (;;) {					 /* read-eval-print */
	    bufprint(PRINT,_outchan,"=%1d=> ",level);		  /* prompt */
	    obj = catch(read_and_eval,_errtagsym,NULL);
	    if (obj == CONVcell(eofsym)) {		/* quit this level? */
	       --level;
	       throw(NIL,_errtagsym);		    /* go to previous level */
	    }
	    PRINTout(obj);				    /* -eval-print- */
	    bufprint(PRINT,_outchan,"\n");
	}
} /* errlevel */

faterr (message)  /* --------------------------------- fatal error handling */
char *message;
{
	printf("FATAL ERROR: %s\n",message);
	exit(1);
} /* faterr */

topexec ()  /* ------------------------------------------- kernel executive */
{
	bufprint(PRINT,_outchan,"KERNEL V1, Aug 87\n");
	for (;;) {
	    errtrap = errocc = 0;
	    errshow = 1;
	    level = 0;				/* top level: level is zero */
	    catch(top_lev_call,top_lev_tags,NULL);
	    bufprint(PRINT,_outchan,"\n[KERNEL top level]\n");
	}
} /* topexec */

kerncell
Ltoplevel()  /* ------------------------------------------------ (toplevel) */
{
   kerncell obj;

	CHECKlargs(toplevelsym,0);
	for (;;) {				    /* read-eval-print loop */
	    bufprint(PRINT,_outchan,"=> ");
	    if ((obj = eval(read_and_eval)) == CONVcell(eofsym)) {
	       bufprint(PRINT,_outchan,"\n");
	       exit(0);
	    }
	    PRINTout(obj);
	    bufprint(PRINT,_outchan,"\n");
	}
} /* Ltoplevel */

kerncell
Lreset ()  /* ----------------------------------------------------- (reset) */
{
	CHECKlargs(resetsym,0);
	throw(NIL,_toptagsym);
} /* Lreset */

kerncell
Vexit ()  /* ----------------------------------------------- (exit ['code]) */
{
   kerncell arg;
   int	    idx1 = ARGidx1;
   int	    exitcode = 0;

	CHECKvargs2(exitsym,1);
	if (argtop == idx1 + 1) {
	   arg = ARGnum1;
	   exitcode = GETint(exitsym,arg);
	}
	exit(exitcode);
} /* Vexit */
