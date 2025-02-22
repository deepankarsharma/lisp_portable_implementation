/* KERNEL, 3/9/87 -- S. Hekmatpour
 * set manipulation functions:
 */
#include "kernel.h"

kerncell
Lconvset ()  /* ------------------------------------------- (convset 'list) */
{
   register kerncell arg = ARGnum1;
   kerncell res = NIL;

	CHECKlargs(convsetsym,1);
	CHECKlist(convsetsym,arg);
	while (ISlist(arg)) {
	   if (!member(arg->CELLcar,arg->CELLcdr))
	      res = mkset(arg->CELLcar,res);
	   arg = arg->CELLcdr;
	}
	return(res);
} /* convset */

kerncell
Ldconvset ()  /* ----------------------------------------- (*convset 'list) */
{
   register kerncell arg = ARGnum1;
   kerncell res;

	CHECKlargs(dconvsetsym,1);
	CHECKlist(dconvsetsym,arg);
	while (member(arg->CELLcar,arg->CELLcdr))
	   arg = arg->CELLcdr;
	res = arg;
	while (ISlist(arg->CELLcdr)) {
	   if (member(arg->CELLcdr->CELLcar,arg->CELLcdr->CELLcdr))
	      arg->CELLcdr = arg->CELLcdr->CELLcdr;
	   else {
	      arg->flag = SETOBJ;
	      arg = arg->CELLcdr;
	   }
	}
	if (ISlist(arg))
	   arg->flag = SETOBJ;
	return(res);
} /* Ldconvset */

kerncell
Lconvlist ()  /* ------------------------------------------ (convlist 'set) */
{
   register kerncell arg = ARGnum1;
   kerncell res = NIL;

	CHECKlargs(convlistsym,1);
	CHECKlist(convlistsym,arg);
	while (ISlist(arg)) {
	   res = mkcell(arg->CELLcar,res);
	   arg = arg->CELLcdr;
	}
	return(res);
} /* Lconvlist */

kerncell
Ldconvlist ()  /* ---------------------------------------- (*convlist 'set) */
{
   register kerncell arg = ARGnum1;
   kerncell res = arg;

	CHECKlargs(dconvlistsym,1);
	CHECKlist(dconvlistsym,arg);
	while (ISlist(arg)) {
	   arg->flag = LISTOBJ;
	   arg = arg->CELLcdr;
	}
	return(res);
} /* Ldconvlist */

kerncell
Veset ()  /* ------------------------------------- (eset 'expr1 ... 'exprn) */
{
   register int idx = ARGidx1;
   register kerncell res = NIL;

	while (idx < argtop)
	   res = mkset(argstk[idx++],res);
	return(remrep(res));
} /* Veset */

kerncell
remrep (set)  /* ------------------------------ remove repetitions from set */
register kerncell set;
{
   kerncell res;

	while (member(set->CELLcar,set->CELLcdr))
	   set = set->CELLcdr;
	res = set;
	while (ISlist(set->CELLcdr)) {
	   if (member(set->CELLcdr->CELLcar,set->CELLcdr->CELLcdr))
	      set->CELLcdr = set->CELLcdr->CELLcdr;
	   else
	      set = set->CELLcdr;
	}
	return(res);
} /* remrep */

kerncell
Uiset ()  /* -------- (iset expr (var1 dom1 ... varn domn) expr1 ... exprn) */
{
   kerncell list = argstk[argtop];

	if (checkdoms(list->CELLcdr->CELLcar))
	   error(isetsym,err_dom,NULL);
	return(iset(list->CELLcar,
		    list->CELLcdr->CELLcar,
		    list->CELLcdr->CELLcdr));
} /* Uiset */

kerncell
iset (gen,doms,body)  /* ------------------------ implicit set construction */
kerncell gen, doms, body;
{
   kernsym  sym = CONVsym(doms->CELLcar);
   register kerncell dom = eval(doms->CELLcdr->CELLcar);
   register kerncell list;
   kerncell tmp, res = NIL;

	VARpush(sym,sym->flag,sym->bind);
	sym->flag = VARIABLE;
	doms = doms->CELLcdr->CELLcdr;
	if (!ISlist(dom) && dom != NIL)
	   error(isetsym,err_dom,NULL);
	while (ISlist(dom)) {
	   sym->bind = dom->CELLcar;
	   if (ISlist(doms))
	      res = unionaux(iset(gen,doms,body),res);
	   else {
	      list = body;
	      while (ISlist(list)) {
		 tmp = eval(list->CELLcar);
		 list = list->CELLcdr;
	      }
	      if (tmp != NIL)
		 res = mkset(eval(gen),res);
	   }
	   dom = dom->CELLcdr;
	}
	VARpop();
	return(ISlist(doms) ? res : remrep(res));
} /* iset */

kerncell
unionaux (set1,set2)  /* --------------------------- union of set1 and set2 */
register kerncell set1, set2;
{
	while (ISlist(set1)) {
	   if (!member(set1->CELLcar,set2))
	      set2 = mkset(set1->CELLcar,set2);
	   set1 = set1->CELLcdr;
	}
	return(set2);
} /* unionaux */

kerncell
Vunion ()  /* ------------------------------------- (union 'set1 ... 'setn) */
{
   register int idx = ARGidx1;
   register kerncell argi;
   kerncell res;

	if (idx == argtop)
	   return(NIL);
	if (idx+1 == argtop)
	   return(ARGnum1);
	res = ARGnum1;
	CHECKlist(unionsym,res);
	while (++idx < argtop) {
	   argi = argstk[idx];
	   CHECKlist(unionsym,argi);
	   while (ISlist(argi)) {
	      if (!member(argi->CELLcar,res))
		 res = mkset(argi->CELLcar,res);
	      argi = argi->CELLcdr;
	   }
	}
	return(res);
} /* Vunion */

kerncell
Vintsec ()  /* ----------------------------------- (intsec 'set1 ... 'setn) */
{
   register int idx = ARGidx1;
   register kerncell argi;
   kerncell tmp, res;

	if (idx == argtop)
	   return(NIL);
	if (idx+1 == argtop)
	   return(ARGnum1);
	tmp = ARGnum1;
	CHECKlist(intsecsym,tmp);
	while (++idx < argtop) {
	   argi = argstk[idx];
	   CHECKlist(intsecsym,argi);
	   res = NIL;
	   while (ISlist(argi)) {
	      if (member(argi->CELLcar,tmp))
		 res = mkset(argi->CELLcar,res);
	      argi = argi->CELLcdr;
	   }
	   tmp = res;
	}
	return(res);
} /* Vintsec */

kerncell
Ldiff ()  /* ------------------------------------------- (diff 'set1 'set2) */
{
   register kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;
   kerncell res = NIL;

	CHECKlargs(diffsym,2);
	CHECKlist(diffsym,arg1);
	CHECKlist(diffsym,arg2);
	while (ISlist(arg1)) {
	   if (!member(arg1->CELLcar,arg2))
	      res = mkset(arg1->CELLcar,res);
	   arg1 = arg1->CELLcdr;
	}
	return(res);
} /* Ldiff */

kerncell
Lsubset ()  /* --------------------------------------- (subset 'set1 'set2) */
{
   register kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(subsetsym,2);
	CHECKlist(subsetsym,arg1);
	CHECKlist(subsetsym,arg2);
	while (ISlist(arg1)) {
	   if (!member(arg1->CELLcar,arg2))
	      return(NIL);
	   arg1 = arg1->CELLcdr;
	}
	return(TTT);
} /* Lsubset */
