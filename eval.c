/* KERNEL, 6/7/87 -- S. Hekmatpour
 * evaluation functions:
 */
#include "kernel.h"

kerncell
eval (expr)  /* --------------------------------------------- evaluate expr */
kerncell expr;
{
	if (ISsym(expr)) {
	   if (ISconst(expr) || ISvar(expr) || ISinternal(expr))
	      return(CONVsym(expr)->bind);
	   error(evalsym,"unbound symbol",EVALpush(expr));
	}
	else if (ISlist(expr))
	{  int	    save_celltop = celltop;	  /* save top of cell stack */
	   kerncell save = evalcall(expr->CELLcar,EVALpush(expr),0);
	   EVALpop();
	   celltop = save_celltop;	       /* restore top of cell stack */
	   return(save);
	}
	else
	   return(expr);	    /* any other object evaluates to itself */
} /* eval */

/* evalcall is used for 2 purposes:
 * 1) for evaluating a list, in which case 'head' and 'tail' will be the
 *    head and tail of this list, and 'stacked' will be zero.
 * 2) for doing a call by 'Vcall' or 'Lapply', e.g. (call fun arg1 ... argn).
 *    In this case 'head' will be 'fun', and 'tail' is ignored (can be
 *    anything.) Furthermore, 'fun','arg1' ... 'argn' are already stacked,
 *    hence 'stacked' will be non-zero.
 */
kerncell
evalcall (head,list,stacked)  /* ----------------- evaluate a function call */
register kerncell head, list;
int stacked;			  /* non-zero when args are already stacked */
{
   kerncell fox;
   kerncell (* fun) ();
   int	    arg1;

     start:
	if (ISlist(head)) {			   /* ((...) arg1 ... argn) */
	   if ((fox = head->CELLcar) == CONVcell(lamsym))
	      return(evallam(head,list->CELLcdr,stacked));
	   if (fox == CONVcell(vlamsym))
	      return(evalvlam(head,list->CELLcdr,stacked));
	   if (fox == CONVcell(ulamsym))
	      return(expand(head,list,stacked));
	   if (fox == CONVcell(mlamsym))
	      return(eval(expand(head,list,stacked)));
	   head = evalcall(head->CELLcar,head,0);
	}
	if (ISsym(head)) {			    /* (head arg1 ... argn) */
	   if (head == CONVcell(quotesym))			   /* 'expr */
	      return(list->CELLcdr->CELLcar);
	   switch (head->flag) {	    /* what kind of symbol is head? */
	     case FUNCTION:
		    head = CONVsym(head)->bind;		/* function binding */
		    goto start;
	     case LBINARY:
	     case VBINARY:
		    if (!stacked) {
		       arg1 = argtop+1;
		       list = list->CELLcdr;
		       while (ISlist(list)) {
			  fox = eval(list->CELLcar);	   /* evaluate args */
			  ARGpush(fox);		/* push args onto arg stack */
			  list = list->CELLcdr;
		       }
		       ARGpush(CONVcell(arg1)); /* push position of 1st arg */
		    }
		    fox = ((fun = (kerncell (*)()) CONVsym(head)->bind) !=
									(kerncell (*)()) CONVcell(Lcxxr)
			   ? (* fun)()
			   : (* fun)(CONVsym(head)->name));
		    if (!stacked)
		       ARGSpop();
		    return(fox);
	     case UBINARY:
		    fox = (stacked ? mkargslist() : list->CELLcdr);
		    ARGpush(fox);
		    fun = (kerncell (*)())CONVsym(head)->bind;
		    fox = (* fun)();
		    ARGpop();
		    return(fox);
	     case MBINARY:
		    fox = (stacked ? mkargslist() : list->CELLcdr);
		    ARGpush(fox);
		    fun = (kerncell (*)()) CONVsym(head)->bind;
		    fox = (* fun)();
		    ARGpop();
		    return(eval(stacked
				? fox		   /* substitute the result */
				: (ISlist(fox)
				   ? (list->CELLcar = fox->CELLcar,
				      list->CELLcdr = fox->CELLcdr,
				      list)
				   : (list->CELLcar = CONVcell(voidsym),
				      list->CELLcdr = mkcell(fox,nil)))));
	   } /* switch */
	}
	if (ISvector(fox = head) ||			 /* indexed vector? */
	    ISvar(fox) && ISvector(fox = CONVsym(fox)->bind))
	  return(evalvector(fox,list->CELLcdr,stacked));
	error(evalsym,"undefined function",head);
} /* evalcall */

kerncell
evallam (lam,args,stacked) /* ------------------ evaluate a lam application */
register kerncell lam, args;
int stacked;			  /* non-zero when args are already stacked */
{
   int	    arg1, nvars;
   kerncell obj, vars;
   register kerncell vs;

	lam = lam->CELLcdr;				   /* drop lam head */
	if ((vars = lam->CELLcar) != NIL && !ISlist(vars))
	   error(evalsym,err_pars,vars);
	nvars = checkvars(vars);	 /* check that vars are all symbols */
	if (!stacked) {
	   arg1 = argtop + 1;
	   while (ISlist(args)) {
	      obj = eval(args->CELLcar);      /* evaluate each argument and */
	      ARGpush(obj);			  /* push it onto arg stack */
	      args = args->CELLcdr;
	   }
	   ARGpush(CONVcell(arg1));		/* push position of 1st arg */
	}
	else
	   arg1 = ARGidx1;
	CHECKlargs(lamsym,nvars);		    /* check number of args */
	savevars(vars);			    /* save current binding of vars */
	for (vs=vars; ISlist(vs); vs=vs->CELLcdr)  /* bind the vars to args */
	    CONVsym(vs->CELLcar)->bind = argstk[arg1++];
	obj = NIL;
	lam = lam->CELLcdr;
	while (ISlist(lam)) {
	   obj = eval(lam->CELLcar);	      /* eval each form in lam body */
	   lam = lam->CELLcdr;
	}
	restorevars(vars);		     /* restore the binding of vars */
	if (!stacked)
	   ARGSpop();					   /* pop arguments */
	return(obj);			   /* return the value of last form */
} /* evallam */

kerncell
evalvlam (vlam,args,stacked)  /* -------------- evaluate a vlam application */
register kerncell vlam, args;
int stacked;			  /* non-zero when args are already stacked */
{
   int arg1;
   kerncell obj, vars;
   int save_argtop = _argtop;			   /* for nested vlam calls */

	vlam = vlam->CELLcdr;				  /* drop vlam head */
	if (!ISlist(vars = vlam->CELLcar) || checkvars(vars) != 1)
	   error(evalsym,err_pars,vars);
	if (!stacked) {
	   arg1 = argtop+1;
	   while (ISlist(args)) {
	      obj = eval(args->CELLcar);      /* evaluate each argument and */
	      ARGpush(obj);			  /* push it onto arg stack */
	      args = args->CELLcdr;
	   }
	   ARGpush(CONVcell(arg1));
	}
	else
	   arg1 = ARGidx1;
	_argtop = argtop;      /* save argtop -- for use by 'arg' function: */
	savevars(vars);			     /* save current binding of var */
	/* var is set to the number of arguments: */
	CONVsym(vars->CELLcar)->bind = mkinum(argtop-arg1);
	obj = NIL;
	vlam = vlam->CELLcdr;
	while (ISlist(vlam)) {
	   obj = eval(vlam->CELLcar);	     /* eval each form in vlam body */
	   vlam = vlam->CELLcdr;
	}
	restorevars(vars);		      /* restore the binding of var */
	_argtop = save_argtop;				 /* restore _argtop */
	if (!stacked)
	   ARGSpop();					   /* pop arguments */
	return(obj);			   /* return the value of last form */
} /* evalvlam */

kerncell
expand (fun,list,stacked)   /* --------------- expand ulam/mlam application */
register kerncell fun;
kerncell list;
int	 stacked;		  /* non-zero when args are already stacked */
{
   kerncell fox, vars;
   int	    ismacro = fun->CELLcar == CONVcell(mlamsym);

	fun = fun->CELLcdr;				  /* drop ulam/mlam */
	if (!ISlist(vars = fun->CELLcar) || checkvars(vars) != 1)
	   error(evalsym,err_pars,vars);
	/* the list of arguments is treated as 1 arg: */
	fox = (stacked ? mkargslist() : list->CELLcdr);
	ARGpush(fox);
	savevars(vars);
	CONVsym(vars->CELLcar)->bind = argstk[argtop];
	fox = NIL;
	fun = fun->CELLcdr;
	while (ISlist(fun)) {				   /* evaluate body */
	   fox = eval(fun->CELLcar);
	   fun = fun->CELLcdr;
	}
	restorevars(vars);
	ARGpop();
	return(ismacro && !stacked
	       ? (ISlist(fox)			   /* substitute the result */
		  ? (list->CELLcar = fox->CELLcar,
		     list->CELLcdr = fox->CELLcdr,
		     list)
		  : (list->CELLcar = CONVcell(voidsym),
		     list->CELLcdr = mkcell(fox,nil),
		     list))
	       : fox);
} /* expand */

kerncell
evalvector (head,tail,stacked)  /* --------------------- vector application */
kerncell head, tail;
int	 stacked;		  /* non-zero when args are already stacked */
{
   kerncell index;

	   if (stacked ? (argtop - ARGidx1 != 1 || !ISint(index = ARGnum1))
		       : (tail == NIL || tail->CELLcdr != NIL
			  || !ISint(index = eval(tail->CELLcar))))
	      error(evalsym,"bad vector index",index);
	   if (index->CELLinum < 0 ||
	       index->CELLinum >= head->CELLdim->CELLinum)
	      error(evalsym,"vector index out of range",index);
	   if (stacked)
	      ARGSpop();
	   return(*(head->CELLvec + index->CELLinum));
} /* evalvector */

checkvars (vars)  /* ---------- check that elements of vars are all symbols */
register kerncell vars;		     /* returns the length of the vars list */
{
   int count = 0;

	while (ISlist(vars)) {
	   ++count;
	   if (!ISsym(vars->CELLcar) || ISconst(vars->CELLcar))
	      error(evalsym,err_pars,vars->CELLcar);
	   vars = vars->CELLcdr;
	}
	return(count);
} /* checkvars */

savevars (vars)  /* ------------------- save the bindings of vars in varstk */
register kerncell vars;
{
   register kernsym var;

	while (ISlist(vars)) {
	   var = CONVsym(vars->CELLcar);
	   /* NOTE: property lists are not stacked: */
	   VARpush(var,var->flag,var->bind);
	   var->flag = VARIABLE;
	   vars = vars->CELLcdr;
	}
} /* savevars */

restorevars (vars)  /* ------------------- restore the binding of variables */
register kerncell vars;
{
	while (ISlist(vars)) {
	   VARpop();
	   vars = vars->CELLcdr;
	}
} /* restorevars */

kerncell
mkargslist ()  /* ------- make an argument list using the entries on argstk */
{
   register int argi = ARGidx1;
   register kerncell arglist = NIL, list;

	while (argi < argtop)
	   if (arglist == NIL)
	      arglist = list = mkcell(argstk[argi++],nil);
	   else
	      list = list->CELLcdr = mkcell(argstk[argi++],nil);
	return(arglist);
} /* mkargslist */

kerncell
Leval ()  /* ------------------------------------------------- (eval 'expr) */
{
	CHECKlargs(evalsym,1);
	return(eval(ARGnum1));
} /* Leval */

kerncell
Vcall ()  /* ---------------------------------- (call 'fun 'arg1 ... 'argn) */
{
   kerncell fox;

	CHECKvargs1(callsym,1);
	fox = ARGnum1;			       /* the function to be called */
	/* pretend fun is not on the stack: */
	argstk[argtop] = CONVcell(CONVint(argstk[argtop]) + 1);
	fox = evalcall(fox,nil,1);			     /* do the call */
	/* restore argstk: */
	argstk[argtop] = CONVcell(CONVint(argstk[argtop]) - 1);
	return(fox);
} /* Vcall */

kerncell
Lapply ()  /* --------------------------------------- (apply 'fun 'arglist) */
{
   kerncell arg1 = ARGnum1;
   register kerncell arg2 = ARGnum2;
   kerncell fox  = CONVcell(argtop + 1);

	CHECKlargs(applysym,2);
	CHECKlist(applysym,arg2);
	while (ISlist(arg2)) {			     /* stack the arguments */
	   ARGpush(arg2->CELLcar);
	   arg2 = arg2->CELLcdr;
	}
	ARGpush(fox);
	fox = evalcall(arg1,nil,1);
	ARGSpop();
	return(fox);
} /* Lapply */
