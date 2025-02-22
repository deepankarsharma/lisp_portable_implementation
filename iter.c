/* KERNEL, 24/8/87 -- S. Hekmatpour
 * iteration functions:
 */
#include "kernel.h"

kerncell
Uprog ()  /* ------------------------------- (prog (...vars...) ...body...) */
{
   kerncell list = argstk[argtop];
   register kerncell vars, save;
   kerncell fox;
   register kernsym sym;

	if (list == NIL || (!ISlist(vars = list->CELLcar) && vars != NIL))
	   error(progsym,"bad variable list",vars);
	save = vars;				  /* save a pointer to vars */
	while (ISlist(vars)) {			/* process vars, one by one */
	   if (ISsym(fox = vars->CELLcar) && !ISconst(CONVsym(fox))) {
	      VARpush(sym = CONVsym(fox),sym->flag,sym->bind);
	      sym->flag = VARIABLE;
	      sym->bind = NIL;		/* in which case it is bound to nil */
	   }
	   else if (ISlist(fox)) {		   /* or a list: (var init) */
	      if (ISsym(sym = CONVsym(fox->CELLcar)) && !ISconst(sym))
		 VARpush(sym,sym->flag,sym->bind);
	      else
		 error(progsym,err_var,sym);
	      /* in which case it is bound to init: */
	      sym->flag = VARIABLE;
	      sym->bind = eval(fox->CELLcdr->CELLcar);
	   }
	   else
	      error(progsym,err_var,fox);
	   vars = vars->CELLcdr;
	}
	fox = catch(prog,_rettagsym,list->CELLcdr); /* take care of returns */
	while (ISlist(save)) {		 /* restore variable bindings, etc. */
	   VARpop();
	   save = save->CELLcdr;
	}
	return(fox);
} /* Uprog */

kerncell
prog (list)  /* ------------------------------ list is the body of the prog */
register kerncell list;
{
   kerncell save = list;

    start:
	/* catch go's: */
	if (catch(progaux,_gotagsym,list) == CONVcell(_gotagsym)) {
	   for (list=save; ISlist(list); list=list->CELLcdr)
	       if (list->CELLcar == golabel) {
		  list = list->CELLcdr;
		  goto start;
	       }
	   error(gosym,"no such label",golabel);
	}
	else
	   return(NIL);
} /* prog */

static kerncell
progaux (list)  /* ---------------------------------------------- auxiliary */
register kerncell list;
{
	while (ISlist(list)) {			 /* evaluate each list, but */
	   if (ISlist(list->CELLcar))
	      eval(list->CELLcar);
	   list = list->CELLcdr;
	}
	return(NIL);
} /* progaux */

kerncell
Ugo ()  /* ----------------------------------------------------- (go label) */
{
	if (!ISlist(argstk[argtop]))
	   error(gosym,"label required",NULL);
	golabel = argstk[argtop]->CELLcar;
	throw(_gotagsym,_gotagsym);
} /* Ugo */

kerncell
Vreturn ()  /* ----------------------------------------- (return ['result]) */
{
	CHECKvargs2(returnsym,1);
	throw((ARGidx1 == argtop ? NIL : ARGnum1),_rettagsym);
} /* Vreturn */

kerncell
Udo ()  /* ----------------------------------------- (do n expr1 ... exprn) */
{
   kerncell list = argstk[argtop];
   kerncell res = NIL;
   register kerncell fox;
   register int times;

	if (list->CELLcdr == NIL)
	   error(dosym,err_args,NULL);
	fox = eval(list->CELLcar);
	times = GETint(dosym,fox);
	list = list->CELLcdr;
	while (times-- > 0) {
	   fox = list;
	   while (ISlist(fox)) {
	      res = eval(fox->CELLcar);
	      fox = fox->CELLcdr;
	   }
	}
	return(res);
} /* Udo */
