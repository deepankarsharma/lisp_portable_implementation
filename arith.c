/* KERNEL, 20/7/87 -- S. Hekmatpour
 * arithmetic functions:
 */
#include "kernel.h"

kerncell
Lplus ()  /* ---------------------------------------------- (+ 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(plussym,2);
	if (ISint(arg1) || ISreal(arg1)) {
	   if (ISint(arg2))
	      return(ISint(arg1) ? mkinum(arg1->CELLinum + arg2->CELLinum)
				 : mkrnum(arg1->CELLrnum + arg2->CELLinum));
	   if (ISreal(arg2))
	      return(ISint(arg1) ? mkrnum(arg1->CELLinum + arg2->CELLrnum)
				 : mkrnum(arg1->CELLrnum + arg2->CELLrnum));
	   arg1 = arg2;
	}
	error(plussym,err_num,arg1);
} /* Lplus */

kerncell
Lminus ()  /* --------------------------------------------- (- 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(minussym,2);
	if (ISint(arg1) || ISreal(arg1)) {
	   if (ISint(arg2))
	      return(ISint(arg1) ? mkinum(arg1->CELLinum - arg2->CELLinum)
				 : mkrnum(arg1->CELLrnum - arg2->CELLinum));
	   if (ISreal(arg2))
	      return(ISint(arg1) ? mkrnum(arg1->CELLinum - arg2->CELLrnum)
				 : mkrnum(arg1->CELLrnum - arg2->CELLrnum));
	   arg1 = arg2;
	}
	error(minussym,err_num,arg1);
} /* Lminus */

kerncell
Ltimes ()  /* --------------------------------------------- (* 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(timessym,2);
	if (ISint(arg1 = ARGnum1) || ISreal(arg1)) {
	   if (ISint(arg2 = ARGnum2))
	      return(ISint(arg1) ? mkinum(arg1->CELLinum * arg2->CELLinum)
				 : mkrnum(arg1->CELLrnum * arg2->CELLinum));
	   if (ISreal(arg2))
	      return(ISint(arg1) ? mkrnum(arg1->CELLinum * arg2->CELLrnum)
				 : mkrnum(arg1->CELLrnum * arg2->CELLrnum));
	   arg1 = arg2;
	}
	error(timessym,err_num,arg1);
} /* Ltimes */

kerncell
Ldiv ()  /* ----------------------------------------------- (/ 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(divsym,2);
	if (ISint(arg1) || ISreal(arg1)) {
	   if (ISint(arg2))
	      return(ISint(arg1) ? mkinum(arg1->CELLinum / arg2->CELLinum)
				 : mkrnum(arg1->CELLrnum / arg2->CELLinum));
	   if (ISreal(arg2))
	      return(ISint(arg1) ? mkrnum(arg1->CELLinum / arg2->CELLrnum)
				 : mkrnum(arg1->CELLrnum / arg2->CELLrnum));
	   arg1 = arg2;
	}
	error(divsym,err_num,arg1);
} /* Ldiv */

kerncell
Vsum ()  /* ----------------------------------------- (sum 'num1 ... 'numn) */
{
   double   sum = 0;
   int	    has_real = 0;
   register int idx = ARGidx1;
   register kerncell arg;

	while (idx < argtop) {
	   if (ISint(arg = argstk[idx++]))
	      sum += arg->CELLinum;
	   else if (ISreal(arg)) {
	      has_real = 1;
	      sum += arg->CELLrnum;
	   }
	   else
	      error(sumsym,err_num,arg);
	}
	return(has_real ? mkrnum((real) sum)
			: mkinum((int)  sum));
} /* Vsum */

kerncell
Vprod ()  /* --------------------------------------- (prod 'num1 ... 'numn) */
{
   double   prod = 1;
   int	    has_real = 0;
   register int idx = ARGidx1;
   register kerncell arg;

	while (idx < argtop) {
	   if (ISint(arg = argstk[idx++]))
	      prod *= arg->CELLinum;
	   else if (ISreal(arg)) {
	      has_real = 1;
	      prod *= arg->CELLrnum;
	   }
	   else
	      error(prodsym,err_num,arg);
	}
	return(has_real ? mkrnum((real) prod)
			: mkinum((int)  prod));
} /* Vprod */

kerncell
Lrem ()  /* --------------------------------------------- (% 'inum1 'inum2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(remsym,2);
	return(mkinum(GETint(remsym,arg1) % GETint(remsym,arg2)));
} /* Lrem */

kerncell
Lpow ()  /* ----------------------------------------------- (^ 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;
   double   pow();

	CHECKlargs(powsym,2);
	return(mkrnum((real) pow((double) GETnum(powsym,arg1),
				 (double) GETnum(powsym,arg2))));

} /* Lpow */

kerncell
Linc ()  /* ---------------------------------------------------- (++ 'inum) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(incsym,1);
	return(mkinum(GETint(incsym,arg) + 1));
} /* Linc */

kerncell
Ldec ()  /* ---------------------------------------------------- (-- 'inum) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(decsym,1);
	return(mkinum(GETint(decsym,arg) - 1));
} /* Ldec */

kerncell
Labs ()  /* ---------------------------------------------------- (abs 'num) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(abssym,1);
	if (ISint(arg))
	   return(arg->CELLinum >= 0 ? arg : mkinum(-arg->CELLinum));
	if (ISreal(arg))
	   return(arg->CELLrnum >= 0 ? arg : mkrnum(-arg->CELLrnum));
	error(abssym,err_num,arg);
} /* Labs */

kerncell
Lneg ()  /* --------------------------------------------------- (neg 'inum) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(negsym,1);
	if (ISint(arg))
	   return(mkinum(-arg->CELLinum));
	if (ISreal(arg))
	   return(mkrnum(-arg->CELLrnum));
	error(negsym,err_num,arg);
} /* Lneg */

kerncell
Lint ()  /* --------------------------------------------------- (int 'rnum) */
{
   kerncell arg = ARGnum1;
   double   floor();

	CHECKlargs(intsym,1);
	return(mkinum((int) floor(GETreal(intsym,arg) + 0.0)));
} /* Lint */

kerncell
Lreal ()  /* ------------------------------------------------- (real 'inum) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(realsym,1);
	return(mkrnum((real) (GETint(realsym,arg) + 0.0)));
} /* Lreal */

kerncell
La_lt ()  /* ---------------------------------------------- (< 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(a_ltsym,2);
	return(GETnum(a_ltsym,arg1) < GETnum(a_ltsym,arg2) ? TTT : NIL);
} /* La_lt */

kerncell
La_gt ()  /* ---------------------------------------------- (> 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(a_gtsym,2);
	return(GETnum(a_gtsym,arg1) > GETnum(a_gtsym,arg2) ? TTT : NIL);
} /* La_gt */

kerncell
La_le ()  /* --------------------------------------------- (<= 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(a_lesym,2);
	return(GETnum(a_lesym,arg1) <= GETnum(a_lesym,arg2) ? TTT : NIL);
} /* La_le */

kerncell
La_ge ()  /* --------------------------------------------- (>= 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(a_gesym,2);
	return(GETnum(a_gesym,arg1) >= GETnum(a_gesym,arg2) ? TTT : NIL);
} /* La_ge */

kerncell
La_eq ()  /* ---------------------------------------------- (= 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(a_eqsym,2);
	return(GETnum(a_eqsym,arg1) == GETnum(a_eqsym,arg2) ? TTT : NIL);
} /* La_eq */

kerncell
La_ne ()  /* --------------------------------------------- (/= 'num1 'num2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(a_nesym,2);
	return(GETnum(a_nesym,arg1) != GETnum(a_nesym,arg2) ? TTT : NIL);
} /* La_ne */

kerncell
Lnumberp ()  /* ------------------------------------------- (number? 'expr) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(numberpsym,1);
	return(ISint(arg) || ISreal(arg) ? TTT : NIL);
} /* Lnumberp */

kerncell
Lintp ()  /* ------------------------------------------------- (int? 'expr) */
{
	CHECKlargs(intpsym,1);
	return(ISint(ARGnum1) ? TTT : NIL);
} /* Lintp */

kerncell
Lrealp ()  /* ----------------------------------------------- (real? 'expr) */
{
	CHECKlargs(realpsym,1);
	return(ISreal(ARGnum1) ? TTT : NIL);
} /* Lrealp */
