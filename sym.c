/* KERNEL, 29/9/87 -- S. Hekmatpour
 * symbol functions:
 */
#include "kernel.h"

kerncell
Lsymname ()  /* -------------------------------------------- (symname 'sym) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(symnamesym,1);
	CHECKsym1(symnamesym,arg);
	return(mkstr(CONVsym(arg)->name));
} /* Lsymname */

kerncell
Usynonym ()  /* --------------------------------------- (synonym sym1 sym2) */
{
   kerncell list = argstk[argtop];
   kernsym  sym1, sym2;

	if (list->CELLcdr == NIL || list->CELLcdr->CELLcdr != NIL)
	   error(synonymsym,err_args,NULL);
	sym1 = CONVsym(list->CELLcar);
	sym2 = CONVsym(list->CELLcdr->CELLcar);
	CHECKsym2(symnamesym,sym1);
	CHECKsym1(symnamesym,sym2);
	sym1->flag = sym2->flag;
	sym1->bind = sym2->bind;
	sym1->prop = sym2->prop;
	return(CONVcell(sym1));
} /* Usynonym */

kerncell
Lgensym ()  /* --------------------------------------------------- (gensym) */
{
   static int num = 0;

	CHECKlargs(gensymsym,0);
	sprintf(strbuf,"g%04d",num++);
	return(CONVcell(mksym(strbuf)));
} /* Lgensym */

kerncell
Vconcat ()  /* ----------------------------------- (concat 'str1 ... 'strn) */
{
   kerncell arg;
   register int idx = ARGidx1;
   int	    len = 0;
   char	    *str;
   char	    *buf = strbuf;

	CHECKvargs1(concatsym,1);
	while (idx < argtop) {
	   arg = argstk[idx++];
	   str = GETstr(concatsym,arg);
	   if ((len += strlen(str)) > STRBUFSIZE)
	      error(concatsym,"string buffer overflow",NULL);
	   sprintf(buf,"%s",str);
	   buf = strbuf + len;
	}
	return(CONVcell(mksym(strbuf)));
} /* Vconcat */

kerncell
Lbinding ()  /* ------------------------------------------- (binding 'expr) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(bindingsym,1);
	switch (arg->flag) {
	  case CONSTANT:
	  case VARIABLE:
	  case FUNCTION:
		return(CONVsym(arg)->bind);
	  case LBINARY:
		sprintf(strbuf,"lam#%1d",CONVsym(arg)->bind);
		return(mkstr(strbuf));
	  case VBINARY:
		sprintf(strbuf,"vlam#%1d",CONVsym(arg)->bind);
		return(mkstr(strbuf));
	  case UBINARY:
		sprintf(strbuf,"ulam#%1d",CONVsym(arg)->bind);
		return(mkstr(strbuf));
	  case MBINARY:
		sprintf(strbuf,"mlam#%1d",CONVsym(arg)->bind);
		return(mkstr(strbuf));
	  default:
		return(NIL);
	}
} /* Lbinding */

kerncell
Lsymbolp ()  /* ------------------------------------------- (symbol? 'expr) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(symbolpsym,1);
	return(ISsym(arg) ? TTT : NIL);
} /* Lsymbolp */

kerncell
Lboundp ()  /* ---------------------------------------------- (bound? 'sym) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(boundpsym,1);
	CHECKsym1(boundpsym,arg);
	return(ISunbound(arg) ? NIL : TTT);
} /* Lboundp */
