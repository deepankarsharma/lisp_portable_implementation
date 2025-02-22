/* KERNEL, 14/8/87 -- S. Hekmatpour
 * global variable definitions:
 */
#include "kernel.h"

char *err_args	  = "incorrect number of arguments";
char *err_pars	  = "bad parameter(s)";
char *err_evalstk = "evaluation stack overflow";
char *err_varstk  = "variable stack overflow";
char *err_argstk  = "argument stack overflow";
char *err_catstk  = "catch stack overflow";
char *err_memory  = "memory space exhausted";
char *err_int	  = "integer number expected";
char *err_real	  = "real number expected";
char *err_num	  = "number expected";
char *err_str	  = "string expected";
char *err_chan1	  = "channel expected";
char *err_chan2	  = "bad channel";
char *err_sym1	  = "symbol expected";
char *err_sym2	  = "non-constant symbol expected";
char *err_pair	  = "non-nil list expected";
char *err_list	  = "list expected";
char *err_var	  = "bad variable";
char *err_dom	  = "bad domain designator";

kerncell catres;			/* catch result pair */
kerncell golabel;			/* label specified by go in prog */
kerncell _tempstr;
kerncell read_and_eval, top_lev_call, top_lev_tags;
kerncell inchan, outchan, errchan;
iochan	 _inchan, _outchan, _errchan;
char	 strbuf[STRBUFSIZE+2];		/* string buffer */
struct	 variable varstk[VARSTKSIZE+1]; /* variable stack */
kerncell evalstk[EVALSTKSIZE+1];	/* evaluation stack (also cell stack) */
kerncell argstk[ARGSTKSIZE+1];		/* argument stack */
int evaltop = -1;		/* top of evaluation stack */
int celltop = EVALSTKSIZE;	/* top of cell stack */
int vartop  = -1;		/* top of variable stack */
int argtop  = -1;		/* top of argument stack */
int _argtop = -1;		/* argtop for the last vlam */
int (* org_interrupt)();	/* original interrupt handler */

kernsym /* internals: */
	_bquotesym, _commasym, _atsym,
	_toptagsym, _errtagsym, _rettagsym, _gotagsym, _tempsym, _cxxrsym;
kernsym /* constants: */
	nil, ttt, eofsym, inchansym, outchansym, errchansym;
kernsym /* unbounds: */
	lamsym, vlamsym, ulamsym, mlamsym;
kernsym /* eval.c: */
	evalsym, callsym, applysym;
kernsym /* io.c: */
	opensym, closesym, flushsym, readsym, printsym, princsym, tabsym,
	terprisym, prlensym, iobufsym, chanpsym, ppsym;
kernsym /* arith.c: */
	plussym, minussym, timessym, divsym, sumsym, prodsym, remsym,
	powsym, incsym, decsym, abssym, negsym, intsym, realsym,
	a_ltsym, a_gtsym, a_lesym, a_gesym, a_eqsym, a_nesym,
	numberpsym, intpsym, realpsym;
kernsym /* str.c: */
	s_ltsym, s_gtsym, s_eqsym, strcmpsym, nthcharsym, substrsym,
	strlensym, strconcsym, nilstrpsym, stringpsym;
kernsym /* sym.c: */
	symnamesym, synonymsym, gensymsym, concatsym, bindingsym,
	symbolpsym, boundpsym;
kernsym /* list.c: */
	carsym, cdrsym, nthelemsym, nthpairsym, rplacasym, rplacdsym,
	lastelemsym, lastpairsym, conssym, listsym, lengthsym, concsym,
	dconcsym, removesym, dremovesym, substsym, dsubstsym, reversesym,
	dreversesym, membersym, memqsym, equalsym, nequalsym, eqsym, neqsym,
	atompsym, listpsym, pairpsym, nullpsym;
kernsym /* set.c: */
	convsetsym, dconvsetsym, convlistsym, dconvlistsym,
	esetsym, isetsym, unionsym, intsecsym, diffsym, subsetsym;
kernsym /* logic.c: */
	notsym, andsym, orsym, condsym, implysym, equivsym,
	allsym, existsym, onesym;
kernsym /* prop.c: */
	putpropsym, rempropsym, getsym, plistsym, setplistsym,
	assocsym, assqsym;
kernsym /* vec.c: */
	vectorsym, storesym, dimensionsym, vectorpsym;
kernsym /* flow.c: */
	catchsym, throwsym, caperrsym, errorsym, toplevelsym,
	resetsym, exitsym;
kernsym /* iter.c: */
	progsym, gosym, returnsym, dosym;
kernsym /* map.c: */
	mapcarsym, mapasym, mapcdrsym, mapdsym;
kernsym /* misc.c: */
	voidsym, quotesym, kwotesym, defsym, funsym, argsym, letsym,
	setsym, setqsym, constsym, sssym, loadsym, shellsym;
