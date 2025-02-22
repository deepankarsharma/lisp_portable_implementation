/* KERNEL, 20/7/87 -- S. Hekmatpour
 * string functions:
 */
#include "kernel.h"

kerncell
Ls_lt ()  /* --------------------------------------------- (<< 'str1 'str2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(s_ltsym,2);
	return(strcmp(GETstr(s_ltsym,arg1),
		      GETstr(s_ltsym,arg2)) < 0 ? TTT : NIL);
} /* Ls_lt */

kerncell
Ls_gt ()  /* --------------------------------------------- (>> 'str1 'str2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(s_gtsym,2);
	return(strcmp(GETstr(s_gtsym,arg1),
		      GETstr(s_gtsym,arg2)) > 0 ? TTT : NIL);
} /* Ls_gt */

kerncell
Ls_eq ()  /* --------------------------------------------- (== 'str1 'str2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(s_eqsym,2);
	return(strcmp(GETstr(s_eqsym,arg1),
		      GETstr(s_eqsym,arg2)) == 0 ? TTT : NIL);
} /* Ls_eq */

kerncell
Lstrcmp ()  /* --------------------------------------- (strcmp 'str1 'str2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(strcmpsym,2);
	return(mkinum(strcmp(GETstr(strcmpsym,arg1),
			     GETstr(strcmpsym,arg2))));
} /* Lstrcmp */

kerncell
Lnthchar ()  /* ----------------------------------------- (nthchar 'str 'n) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;
   register char *str;
   register int n;

	CHECKlargs(nthcharsym,2);
	str = GETstr(nthcharsym,arg1);
	n   = GETint(nthcharsym,arg2);
	while (n > 0 && *str != 0) {
	   --n;
	   ++str;
	}
	return(mkinum(CONVint(*str)));
} /* Lnthchar */

kerncell
Lsubstr ()  /* ---------------------------------------- (substr 'str 'i 'j) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;
   kerncell arg3 = ARGnum3;
   register char *str;
   register int m, n;

	CHECKlargs(substrsym,3);
	str = GETstr(substrsym,arg1);
	m   = GETint(substrsym,arg2);
	n   = GETint(substrsym,arg3);
	while (m > 0 && *str != 0) {		  /* skip the first m chars */
	   --m;
	   ++str;
	}
	if ((m = 0) > n)				 /* negative range? */
	   strbuf[0] = 0;
	else {				    /* copy the substring to strbuf */
	   while (m < n && *str)
	      strbuf[m++] = *str++;
	   strbuf[m] = 0;
	}
	return(mkstr(strbuf));
} /* Lsubstr */

kerncell
Lstrlen ()  /* ---------------------------------------------- (strlen 'str) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(strlensym,1);
	return(mkinum(strlen(GETstr(strlensym,arg))));
} /* Lstrlen */

kerncell
Lstrconc ()  /* ------------------------------------- (strconc 'str1 'str2) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;
   char *str1, *str2;
   int m, n;

	CHECKlargs(strconcsym,2);
	m = strlen(str1 = GETstr(strconcsym,arg1));
	n = strlen(str2 = GETstr(strconcsym,arg2));
	if (m+n > STRBUFSIZE)
	   error(strconcsym,"string overflow",NULL);
	strcpy(strbuf,str1);
	strcpy(strbuf+m,str2);
	strbuf[m + n] = 0;
	return(mkstr(strbuf));
} /* Lstrconc */

kerncell
Lnilstrp ()  /* -------------------------------------------- (nilstr? 'str) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(nilstrpsym,1);
	return(*(GETstr(nilstrpsym,arg)) == 0 ? TTT : NIL);
} /* Lnilstrp */

kerncell
Lstringp ()  /* -------------------------------------------- (string? 'str) */
{
	CHECKlargs(stringpsym,1);
	return(ISstr(ARGnum1) ? TTT : NIL);
} /* Lstringp */
