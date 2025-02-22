/* KERNEL, 6/7/87 -- S. Hekmatpour
 * list manipulation functions:
 */
#include "kernel.h"

kerncell
Lcar ()  /* --------------------------------------------------- (car 'list) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(carsym,1);
	return(ISlist(arg)
	       ? arg->CELLcar
	       : (arg == NIL ? NIL
			     : CONVcell(error(carsym,err_list,arg))));
} /* Lcar */

kerncell
Lcdr ()  /* --------------------------------------------------- (cdr 'list) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(cdrsym,1);
	return(ISlist(arg)
	       ? arg->CELLcdr
	       : (arg == NIL ? NIL
			     : CONVcell(error(cdrsym,err_list,arg))));
} /* Lcdr */

kerncell
Lcxxr (xx)  /* ----------------------------------------------- (c..r 'list) */
register char *xx;
{
   register kerncell arg = ARGnum1;

	CHECKlargs(_cxxrsym,1);
	xx = xx + strlen(xx) - 2;   /* xx now points to the last 'a' or 'd' */
	while (*xx != 'c') {
	   if (ISlist(arg))
	      arg = (*xx-- == 'a' ? arg->CELLcar : arg->CELLcdr);
	   else if (arg == NIL)
	      return(NIL);
	   else
	      error(_cxxrsym,err_list,arg);
	}
	return (arg);
} /* Lcxxr */

kerncell
Lnthelem ()  /* ---------------------------------------- (nthelem 'list 'n) */
{
   register kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;
   register int num;

	CHECKlargs(nthelemsym,2);
	CHECKlist(nthelemsym,arg1);
	num = GETint(nthelemsym,arg2);
	while (num-- > 1) {
	   if (arg1 == NIL)
	      return(NIL);
	   arg1 = arg1->CELLcdr;
	}
	return(arg1 == NIL ? NIL : arg1->CELLcar);
} /* Lnthelem */

kerncell
Lnthpair ()  /* ---------------------------------------- (nthpair 'list 'n) */
{
   register kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;
   register int num;

	CHECKlargs(nthpairsym,2);
	CHECKlist(nthpairsym,arg1);
	num = GETint(nthpairsym,arg2);
	while (num-- > 1) {
	   if (arg1 == NIL)
	      return(NIL);
	   arg1 = arg1->CELLcdr;
	}
	return(arg1);
} /* Lnthpair */

kerncell
Lrplaca ()  /* ------------------------------------ (rplaca 'list 'newhead) */
{
   kerncell arg1 = ARGnum1;

	CHECKlargs(rplacasym,2);
	CHECKpair(rplacasym,arg1);
	arg1->CELLcar = ARGnum2;
	return(arg1);
} /* Lrplaca */

kerncell
Lrplacd ()  /* ------------------------------------ (rplacd 'list 'newtail) */
{
   kerncell arg1 = ARGnum1;

	CHECKlargs(rplacdsym,2);
	CHECKpair(rplacdsym,arg1);
	arg1->CELLcdr = ARGnum2;
	return(arg1);
} /* Lrplacd */

kerncell
Llastpair ()  /* ----------------------------------------- (lastpair 'list) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(lastpairsym,1);
	CHECKlist(lastpair,arg);
	return(lastpair(arg));
} /* Llastpair */

kerncell
lastpair (list) /* ----------------------------------------- lastpair(list) */
register kerncell list;
{
	if (list == NIL)
	   return(NIL);
	while (ISlist(list->CELLcdr))
	   list = list->CELLcdr;
	return(list);
} /* lastpair */

kerncell
Llastelem ()  /* ----------------------------------------- (lastelem 'list) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(lastelemsym,1);
	CHECKlist(lastelemsym,arg);
	return(arg == NIL ? NIL : lastpair(arg)->CELLcar);
} /* Llastelem */

kerncell
Lcons ()  /* ------------------------------------------- (cons 'expr 'list) */
{
   kerncell arg2 = ARGnum2;
   kerncell newcell;

	CHECKlargs(conssym,2);
	CHECKlist(conssym,arg2);
	newcell = freshcell();
	newcell->flag = LISTOBJ;
	newcell->CELLcar = ARGnum1;
	newcell->CELLcdr = arg2;
	return(newcell);
} /* Lcons */

kerncell
Vlist ()  /* -------------------------------------- (list 'expr1 .. 'exprn) */
{
   register kerncell list;
   kerncell res = NIL;
   register int idx;

	if ((idx = ARGidx1) == argtop)
	   return(NIL);
	while (idx < argtop) {
	   if (res == NIL)
	      res = list = freshcell();
	   else {
	      list->CELLcdr = freshcell();
	      list = list->CELLcdr;
	   }
	   list->flag = LISTOBJ;
	   list->CELLcar = argstk[idx++];
	}
	return(res);
} /* Vlist */

kerncell
Llength ()  /* --------------------------------------------- (length 'list) */
{
   register kerncell arg = ARGnum1;
   register int len;

	CHECKlargs(lengthsym,1);
	CHECKlist(lengthsym,arg);
	for (len=0; ISlist(arg); ++len)
	    arg = arg->CELLcdr;
	return(mkinum(len));
} /* Llength */

kerncell
Vconc ()  /* ------------------------------------- (conc 'list1 ... 'listn) */
{
   register kerncell list = TTT;
   kerncell res = NIL, arg;
   register int idx;

	CHECKvargs1(concsym,2);
	for (idx=ARGidx1; idx < argtop-1; ++idx) {
	    if ((arg = argstk[idx]) == NIL)		    /* ignore nil's */
	       continue;
	    CHECKlist(concsym,arg);
	    if (list == TTT)   /* this happens for the 1st non-nil arg only */
	       res = list = copytop(arg);
	    else
	       (list = lastpair(list))->CELLcdr = copytop(arg);
	}
	return(res == NIL ? argstk[idx]
			  : (lastpair(list)->CELLcdr = argstk[idx], res));
} /* Vconc */

kerncell
Vdconc ()  /* ----------------------------------- (*conc 'list1 ... 'listn) */
{
   register kerncell list = TTT;
   kerncell res = NIL, arg;
   register int idx;

	CHECKvargs1(dconcsym,2);
	for (idx=ARGidx1; idx < argtop-1; ++idx) {
	    if ((arg = argstk[idx]) == NIL)		    /* ignore nil's */
	       continue;
	    CHECKlist(dconcsym,arg);
	    if (list == TTT)   /* this happens for the 1st non-nil arg only */
	       res = list = arg;
	    else
	       (list = lastpair(list))->CELLcdr = arg;
	}
	return(res == NIL ? argstk[idx]
			  : (lastpair(list)->CELLcdr = argstk[idx], res));
} /* Vdconc */

kerncell
copytop(arg)  /* -------------------------------------------- copytop(list) */
register kerncell arg;
{
   register kerncell list;
   kerncell res = NIL;

	while (ISlist(arg)) {
	   if (res == NIL)
	      res = list = freshcell();
	   else {
	      list->CELLcdr = freshcell();
	      list = list->CELLcdr;
	   }
	   list->flag = LISTOBJ;
	   list->CELLcar = arg->CELLcar;
	   arg = arg->CELLcdr;
	}
	return(res);
} /* copytop */

kerncell
Lremove ()  /* --------------------------------------- (remove 'elem 'list) */
{
   kerncell arg1 = ARGnum1;
   register kerncell arg2 = ARGnum2;
   register kerncell list;
   kerncell res = NIL;

	CHECKlargs(removesym,2);
	CHECKlist(removesym,arg2);
	while (ISlist(arg2)) {
	   if (equal(arg2->CELLcar,arg1)) {
	      arg2 = arg2->CELLcdr;
	      continue;
	   }
	   if (res == NIL)
	      res = list = freshcell();
	   else {
	      list->CELLcdr = freshcell();
	      list = list->CELLcdr;
	   }
	   list->flag = LISTOBJ;
	   list->CELLcar = arg2->CELLcar;
	   arg2 = arg2->CELLcdr;
	}
	return(res);
} /* Lremove */

kerncell
Ldremove ()  /* ------------------------------------- (*remove 'elem 'list) */
{
   kerncell arg1 = ARGnum1;
   register kerncell arg2 = ARGnum2;
   kerncell res = arg2;

	CHECKlargs(dremovesym,2);
	CHECKlist(dremovesym,arg2);
	while (ISlist(arg2->CELLcdr))
	   if (equal(arg2->CELLcdr->CELLcar,arg1))
	      arg2->CELLcdr = arg2->CELLcdr->CELLcdr;
	   else
	      arg2 = arg2->CELLcdr;
	return(equal(res->CELLcar,arg1) ? res->CELLcdr : res);
} /* Ldremove */

kerncell
Lsubst ()  /* ----------------------------------- (subst 'this 'that 'list) */
{
   kerncell arg3 = ARGnum3;

	CHECKlargs(substsym,3);
	CHECKlist(substsym,arg3);
	return(subst(ARGnum1,ARGnum2,arg3));
} /* Lsubst */

kerncell
subst (arg1,arg2,arg3)  /* -------------------------- subst(this,that,list) */
kerncell arg1, arg2;
register kerncell arg3;
{
   register kerncell list;
   kerncell res = NIL;

	while (ISlist(arg3)) {
	   if (res == NIL)
	      res = list = freshcell();
	   else {
	      list->CELLcdr = freshcell();
	      list = list->CELLcdr;
	   }
	   list->flag = LISTOBJ;
	   list->CELLcar = (equal(arg3->CELLcar,arg2)
			    ? arg1
			    : (ISlist(arg3->CELLcar)
			       ? subst(arg1,arg2,arg3->CELLcar)
			       : arg3->CELLcar));
	   arg3 = arg3->CELLcdr;
	}
	return(res);
} /* subst */

kerncell
Ldsubst ()  /* --------------------------------- (*subst 'this 'that 'list) */
{
   kerncell arg3 = ARGnum3;

	CHECKlargs(dsubstsym,3);
	CHECKlist(dsubstsym,arg3);
	return(dsubst(ARGnum1,ARGnum2,arg3));
} /* Ldsubst */

kerncell
dsubst (arg1,arg2,arg3)  /* ------------------------ dsubst(this,that,list) */
kerncell arg1, arg2;
register kerncell arg3;
{
   kerncell res = arg3;

	while (ISlist(arg3)) {
	   if (equal(arg3->CELLcar,arg2))
	      arg3->CELLcar = arg1;
	   else if (ISlist(arg3->CELLcar))
	      dsubst(arg1,arg2,arg3->CELLcar);
	   arg3 = arg3->CELLcdr;
	}
	return(res);
} /* dsubst */

kerncell
Lreverse () /* -------------------------------------------- (reverse 'list) */
{
   register kerncell arg = ARGnum1;
   kerncell res = NIL;

	CHECKlargs(reversesym,1);
	CHECKlist(reversesym,arg);
	while (ISlist(arg)) {
	   res = mkcell(arg->CELLcar,res);
	   arg = arg->CELLcdr;
	}
	return(res);
} /* Lreverse */

kerncell
Ldreverse () /* ------------------------------------------ (*reverse 'list) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(dreversesym,1);
	CHECKlist(dreversesym,arg);
	return(dreverse(arg));
} /* Ldreverse */

kerncell
dreverse (list) /* ----------------------------------------- dreverse(list) */
register kerncell list;
{
   kerncell prev = NIL, save = list;

	while (ISlist(list)) {
	   list = list->CELLcdr;
	   save->CELLcdr = prev;
	   prev = save;
	   save = list;
	}
	return(prev);
} /* dreverse */

kerncell
Lmember ()  /* --------------------------------------- (member 'expr 'list) */
{
   kerncell arg1 = ARGnum1;
   register kerncell arg2 = ARGnum2;

	CHECKlargs(membersym,2);
	CHECKlist(membersym,arg2);
	while (ISlist(arg2)) {
	   if (equal(arg1,arg2->CELLcar))
	      return(arg2);
	   arg2 = arg2->CELLcdr;
	}
	return(NIL);
} /* Lmember */

kerncell
Lmemq ()  /* ------------------------------------------- (memq 'expr 'list) */
{
   kerncell arg1 = ARGnum1;
   register kerncell arg2 = ARGnum2;

	CHECKlargs(memqsym,2);
	CHECKlist(memqsym,arg2);
	while (ISlist(arg2)) {
	   if (arg1 == arg2->CELLcar)
	      return(arg2);
	   arg2 = arg2->CELLcdr;
	}
	return(NIL);
} /* Lmemq */

member (expr,list)  /* -------------------------- is expr a member of list? */
kerncell expr;
register kerncell list;
{
	while (ISlist(list)) {
	   if (equal(expr,list->CELLcar))
	      return(1);
	   list = list->CELLcdr;
	}
	return(0);
} /* member */

memq (expr,list)  /* ---------------- has list an element identical to expr? */
kerncell expr;
register kerncell list;
{
	while (ISlist(list)) {
	   if (expr == list->CELLcar)
	      return(1);
	   list = list->CELLcdr;
	}
	return(0);
} /* memq */

kerncell
Lequal ()  /* --------------------------------------- (equal 'expr1 'expr2) */
{
	CHECKlargs(equalsym,2);
	return(equal(ARGnum1,ARGnum2) ? TTT : NIL);
} /* Lequal */

kerncell
Lnequal ()  /* ------------------------------------- (nequal 'expr1 'expr2) */
{
	CHECKlargs(nequalsym,2);
	return(equal(ARGnum1,ARGnum2) ? NIL : TTT);
} /* Lnequal */

equal (expr1,expr2)  /* -------------------------------- equal(expr1,expr2) */
register kerncell expr1, expr2;
{
	if (expr1 == expr2)
	   return(1);
	if (expr1->flag != expr2->flag)
	   return(0);
	switch (expr1->flag) {
	  case INTOBJ:
		return(expr1->CELLinum == expr2->CELLinum);
	  case REALOBJ:
		return(expr1->CELLrnum == expr2->CELLrnum);
	  case STROBJ:
		return(strcmp(expr1->CELLstr,expr2->CELLstr) == 0);
	  case CHANOBJ:
		return(expr1->CELLchan == expr2->CELLchan);
	  case VECTOROBJ:
	     {  register int dim = expr1->CELLdim->CELLinum;
		if (dim != expr2->CELLdim->CELLinum)
		   return(0);
		while (--dim)
		   if (!equal(*(expr1->CELLvec + dim),*(expr2->CELLvec + dim)))
		      return(0);
		return(1);
	     }
	  case LISTOBJ:
		while (ISlist(expr1))
		   if (ISlist(expr2) && equal(expr1->CELLcar,expr2->CELLcar)) {
		      expr1 = expr1->CELLcdr;
		      expr2 = expr2->CELLcdr;
		   } else
		      return(0);
		return(expr1 == NIL ? expr2 == NIL : equal(expr1,expr2));
	  case SETOBJ:
		while (ISlist(expr1) && member(expr1->CELLcar,expr2))
		   expr1 = expr1->CELLcdr;
		return(expr1 == NIL);
	  default:
		return(0);
	}
} /* equal */

kerncell
Leq ()  /* --------------------------------------------- (eq 'expr1 'expr2) */
{
	CHECKlargs(eqsym,2);
	return(ARGnum1 == ARGnum2 ? TTT : NIL);
} /* Leq */

kerncell
Lneq ()  /* ------------------------------------------- (neq 'expr1 'expr2) */
{
	CHECKlargs(neqsym,2);
	return(ARGnum1 == ARGnum2 ? NIL : TTT);
} /* Lneq */

kerncell
Latomp ()  /* ----------------------------------------------- (atom? 'expr) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(atompsym,1);
	return(ISsym(arg)  || ISint(arg) ||
	       ISreal(arg) || ISstr(arg) ? TTT : NIL);
} /* Latomp */

kerncell
Llistp ()  /* ----------------------------------------------- (list? 'expr) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(listpsym,1);
	return(ISlist(arg) || arg == NIL ? TTT : NIL);
} /* Llistp */

kerncell
Lpairp ()  /* ----------------------------------------------- (pair? 'expr) */
{
	CHECKlargs(pairpsym,1);
	return(ISlist(ARGnum1) ? TTT : NIL);
} /* Lpairp */

kerncell
Lnullp ()  /* ----------------------------------------------- (null? 'expr) */
{
	CHECKlargs(nullpsym,1);
	return(ARGnum1 == NIL ? TTT : NIL);
} /* nullp */
