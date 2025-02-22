/* KERNEL, 10/7/87 -- S. Hekmatpour
 * I/O routines:
 */
#include "kernel.h"
#include <math.h>

#define EOL	'\n'
#define TAB	'\t'
#define SPACE	' '
#define ESCAPE  033

#define LPARENTOK	1	/* ( */
#define RPARENTOK	2	/* ) */
#define LBRACKTOK	3	/* [ */
#define RBRACKTOK	4	/* ] */
#define LBRACETOK	5	/* { */
#define RBRACETOK	6	/* } */
#define QUOTETOK	7	/* ' */
#define BQUOTETOK	8	/* ` */
#define COMMATOK	9	/* , */
#define ATTOK		10	/* @ */
#define HASHTOK		11	/* # */
#define STRTOK		12	/* string token */
#define SYMTOK		13	/* symbol token */
#define EOLTOK		14	/* end-of-line token */
#define EOFTOK		15	/* end-of-file token */
#define INUMTOK		16	/* integer number token */
#define RNUMTOK		17	/* real number token */

#define NEXTtok(chan)		((chan)->tok = nexttok(chan))
#define ISdigit(ch)		((ch) >= '0' && (ch) <= '9')
#define DIGITvalue(ch)		((ch) - '0')

int  inumber = 0;
real rnumber = 0.0;

/* for use by pp: */
int ppcols = MAXCOLS;		/* maximum no. of columns on the screen */
int ppgap  = MAXCOLS;		/* free gap for printing */
int llimit = MAXCOLS - 30;	/* left limit */
int rlimit = MAXCOLS + 15;	/* right limit */

iochan
openchan (file,mode)  /* ------------------------------------- open channel */
FILE *file;
int mode;
{
   register iochan chan;

	chan = CONVchan(new(sizeof(struct channel)));
	chan->buf  = new(CHANBUFSIZE+2);
	chan->ch   = EOL;
	chan->tok  = EOLTOK;
	chan->pos  = chan->len = 0;
	chan->mode = mode;
	chan->file = file;
	return(chan);
} /* openchan */

closechan (chan)  /* ---------------------------------------- close channel */
iochan chan;
{
	if (chan->mode == OUTCHAN && chan->len > 0)
	   bufprint(PRINT,chan,"\n");			/* flush the buffer */
	fclose(chan->file);
	free(chan->buf);
	free(chan);
} /* closechan */

initio ()  /* ------------------------------------- initialize I/O channels */
{
	inchan  = mkchan(_inchan  = openchan(stdin,INCHAN));
	outchan = mkchan(_outchan = openchan(stdout,OUTCHAN));
	errchan = mkchan(_errchan = openchan(stderr,OUTCHAN));
} /* initio */

char
nextch(chan)  /* --------------------- returns the next character from chan */
register iochan chan;
{
   register char ch;

	if (chan->pos < chan->len)		   /* more chars in buffer? */
	   return(chan->ch = chan->buf[chan->pos++]);
	chan->pos = chan->len = 0;
	while ((ch = getc(chan->file)) != EOL && ch != EOF)
	   if (chan->len < CHANBUFSIZE)		  /* store it in the buffer */
	      chan->buf[chan->len++] = ch;
	   else {
	      chan->buf[chan->len] = 0;
	      while ((ch = getc(chan->file)) != EOL && ch != EOF)
		 ;				   /* skip till end of line */
	      error(readsym,"line too long",_mkstr(chan->buf));
	   }
	if (chan->len == 0)				     /* empty line? */
	   return(chan->ch = ch);		   /* ch is one of EOL, EOF */
	chan->buf[chan->len++] = EOL;		/* put a newline at the end */
	chan->buf[chan->len] = 0;		  /* null the end of string */
	return(chan->ch = chan->buf[chan->pos++]);
} /* nextch */

nexttok (chan)  /* -------------- fetch and return the next token from chan */
register iochan chan;
{
    start:
	while (chan->ch == SPACE || chan->ch == TAB)	     /* skip blanks */
	   nextch(chan);
	switch (chan->ch) {
	  case '(':  nextch(chan); return(LPARENTOK);
	  case ')':  nextch(chan); return(RPARENTOK);
	  case '[':  nextch(chan); return(LBRACKTOK);
	  case ']':  nextch(chan); return(RBRACKTOK);
	  case '{':  nextch(chan); return(LBRACETOK);
	  case '}':  nextch(chan); return(RBRACETOK);
	  case '\'': nextch(chan); return(QUOTETOK);
	  case '`':  nextch(chan); return(BQUOTETOK);
	  case ',':  nextch(chan); return(COMMATOK);
	  case '@':  nextch(chan); return(ATTOK);
	  case '#':  nextch(chan); return(HASHTOK);
	  case ';':  chan->pos = chan->len = 0;		 /* ingore comments */
		     nextch(chan);
		     goto start;
	  case '"':
	     {  register int i = 0;	      /* string is stored in strbuf */
		while (nextch(chan) != '"' &&
		       chan->ch != EOL && chan->ch != EOF)
		   strbuf[i++] = (chan->ch == '\\' ? nextch(chan) : chan->ch);
		strbuf[i] = 0;
		if (chan->ch == EOL || chan->ch == EOF)
		   error(readsym,"broken string",_mkstr(strbuf));
		nextch(chan);
		return(STRTOK);
	     }
	  case '|':
	     {  register int i = 0;	/* strange atom is stored in strbuf */
		strbuf[i++] = chan->ch;
		while (nextch(chan) != '|' &&
		       chan->ch != EOL && chan->ch != EOF)
		   strbuf[i++] = (chan->ch == '\\' ? nextch(chan) : chan->ch);
		strbuf[i++] = '|';
		strbuf[i] = 0;
		if (chan->ch == EOL || chan->ch == EOF)
		   error(readsym,"broken atom",_mkstr(strbuf));
		nextch(chan);
		return(SYMTOK);
	     }
	  case EOL:  return(EOLTOK);		 /* end-of-line is reported */
	  case EOF:  return(EOFTOK);		 /* end-of-file is reported */
	  case ESCAPE: nextch(chan);			  /* ignore escapes */
		       goto start;
	  default:
	     {  register int i = 0;   /* nums and syms are stored in strbuf */
		strbuf[i++] = chan->ch;
		while (nextch(chan) != '('  && chan->ch != ')'
		       && chan->ch != '['   && chan->ch != ']'
		       && chan->ch != '{'   && chan->ch != '}'
		       && chan->ch != SPACE && chan->ch != TAB
		       && chan->ch != EOL   && chan->ch != EOF)
		   strbuf[i++] = chan->ch;
		strbuf[i] = 0;
		return(atomkind(strbuf));
	     }
	} /* switch */
} /* nexttok */

skipeoltok (chan,flag)  /* ------- skip eol token and return the next token */
register iochan chan;
int	 flag;
{
	if (flag)
	   NEXTtok(chan);
	while (chan->tok == EOLTOK) {			      /* skip eol's */
	   nextch(chan);
	   NEXTtok(chan);
	}
	return(chan->tok);
} /* skipeoltok */

atomkind (name)  /* ----------- work out whether name is a number or symbol */
register char *name;
{
   int	  sign = 1, frac = 0, places = 0;
   double pow();

	if (isnum(name)) {
	   inumber = 0;
	   rnumber = 0;
	   if (*name == '+' || *name == '-')		  /* signed number? */
	      sign = (*name++ == '+' ? 1 : -1);
	   while (*name && *name != '.') {
	      inumber = 10*inumber + DIGITvalue(*name);
	      ++name;
	   }
	   if (*name == '.') {
	      ++name;
	      while (*name && ISdigit(*name)) {	       /* work out fraction */
		 frac = 10*frac + DIGITvalue(*name);
		 ++name;
		 ++places;
	      }
	      rnumber = (float) (sign*(inumber+((double) frac) *
					       pow(10.0,- (double) places)));
	      return(RNUMTOK);				     /* real number */
	   }
	   inumber *= sign;
	   return(INUMTOK);				  /* integer number */
	}
	return(SYMTOK);						  /* symbol */
} /* atomkind */

isnum (name)  /* --------------------------------- is name a number string? */
register char *name;
{
   int decpoint = 0;

	if (*name == '+' || *name == '-')
	   ++name;
	if (*name == 0)			      /* empty name can't be number */
	   return (0);
	while (*name && (ISdigit(*name) || *name == '.')) {
	   if (*name == '.') {		 /* at most 1 decimal point allowed */
	      if (decpoint)
		 return(0);
	      decpoint = 1;
	   }
	   ++name;		       /* skip all digits and decimal point */
	}
	return(*name == 0);		      /* there must be nothing left */
} /* isnum */

kerncell
readaux (chan,bq)  /* ---------------------- read an s-expression from chan */
iochan chan;
int    bq;		     /* non-zero when in a back-quoted s-expression */
{
   int	    save_celltop = celltop;		  /* save top of cell stack */
   kerncell obj;

	if (chan == _inchan && _outchan->len > 0) {
	   fprintf(_outchan->file,"%s",_outchan->buf);	    /* flush output */
	   _outchan->len = 0;
	}
	obj = readaux1(chan,bq);
	celltop = save_celltop;		       /* restore top of cell stack */
	return(CELLpush(obj));
} /* readaux */

kerncell
readaux1 (chan,bq)  /* -------- read an s-expression: for internal use ONLY */
register iochan chan;
int	 bq;		     /* non-zero when in a back-quoted s-expression */
{
   kerncell obj;

    start:
	skipeoltok(chan,0);
	switch (chan->tok) {
	  case SYMTOK:
		obj = CONVcell(mksym(strbuf));
		NEXTtok(chan);
		break;
	  case INUMTOK:
		obj = mkinum(inumber);
		NEXTtok(chan);
		break;
	  case RNUMTOK:
		obj = mkrnum(rnumber);
		NEXTtok(chan);
		break;
	  case STRTOK:
		obj = mkstr(strbuf);
		NEXTtok(chan);
		break;
	  case LPARENTOK:
	  case LBRACKTOK:
	     {  /* NOTE: ) matches ( only, and ] matches [ only */
		int right = (chan->tok == LPARENTOK ? RPARENTOK : RBRACKTOK);
		register kerncell list;
		if (skipeoltok(chan,1) == right) {
		   NEXTtok(chan);
		   return(NIL);					      /* () */
		}
		obj = list = mkcell(readaux1(chan,bq),nil);	 /* (* ...) */
		while (skipeoltok(chan,0),
		       chan->tok != RPARENTOK && chan->tok != RBRACKTOK
		       && chan->tok != EOFTOK) {
		   list->CELLcdr = mkcell(readaux1(chan,bq),nil);
		   list = list->CELLcdr;
		}
		if (chan->tok == EOFTOK)
		   error(readsym,"unexpected end of file",NULL);
		if (chan->tok != right) {
		   if (chan->tok == RPARENTOK)
		      error(readsym,"[ ... ) is not allowed",NULL);
		   else
		      error(readsym,"( ... ] is not allowed",NULL);
		}
		NEXTtok(chan);
		break;
	     }
	  case LBRACETOK:
	     {  register kerncell set;
		if (skipeoltok(chan,1) == RBRACETOK) {
		   NEXTtok(chan);
		   return(NIL);					      /* {} */
		}
		obj = set = mkset(readaux1(chan,bq),nil);	 /* (* ...) */
		while (skipeoltok(chan,0),
		       chan->tok != RBRACETOK && chan->tok != EOFTOK) {
		   set->CELLcdr = mkset(readaux1(chan,bq),nil);
		   set = set->CELLcdr;
		}
		if (chan->tok == EOFTOK)
		   error(readsym,"unexpected end of file",NULL);
		obj = remrep(obj);
		NEXTtok(chan);
		break;
	     }
	  case QUOTETOK:
		NEXTtok(chan);
		obj = mkcell(quotesym,mkcell(readaux1(chan,bq),nil));
		break;
	  case BQUOTETOK:
		NEXTtok(chan);
		obj = transform(readaux1(chan,1));
		break;
	  case COMMATOK:
		NEXTtok(chan);
		if (!bq)
		   error(readsym,"',' outside a back-quoted s-expresion",NULL);
		obj = mkcell(_commasym,readaux1(chan,bq));
		break;
	  case ATTOK:
		NEXTtok(chan);
		if (!bq)
		   error(readsym,"'@' outside a back-quoted s-expression",NULL);
		obj = mkcell(_atsym,readaux1(chan,bq));
		break;
	  case HASHTOK:
		NEXTtok(chan);
		obj = eval(readaux1(chan,bq));
		break;
	  case EOLTOK:
		chan->ch = SPACE;
		NEXTtok(chan);
		goto start;
	  case EOFTOK:
		return(CONVcell(eofsym));
	  case RPARENTOK:
		NEXTtok(chan);
		error(readsym,"unexpected ')'",NULL);
	  case RBRACKTOK:
		NEXTtok(chan);
		error(readsym,"unexpected ']'",NULL);
	  case RBRACETOK:
		NEXTtok(chan);
		error(readsym,"unexpected '}'",NULL);
	  default:
		NEXTtok(chan);
		return(NIL);
	} /* switch */
	return(obj);
} /* readaux1 */

hasmacro (expr)  /* -------- returns non-zero when expr contains ',' or '@' */
register kerncell expr;
{
	if (!ISlist(expr))
	   return(0);
	if (expr->CELLcar == CONVcell(_commasym) ||
	    expr->CELLcar == CONVcell(_atsym))
	   return(1);
	while (ISlist(expr)) {
	   if (hasmacro(expr->CELLcar))
	      return(1);
	   expr = expr->CELLcdr;
	}
	return(0);
} /* hasmacro */

kerncell
transform (list)  /* ------------------ transform back-quoted s-expressions */
kerncell list;
{
   kerncell obj;

	if (list == NIL)
	   return(NIL);
	if (!hasmacro(list))
	   return(mkcell(quotesym,mkcell(list,nil)));
	if (!ISlist(obj = list->CELLcar)) {
	   if (obj == CONVcell(_commasym) || obj == CONVcell(_atsym))
	      return(eval(transform(list->CELLcdr)));
	   return(mkcell(conssym,
			 mkcell(mkcell(quotesym,mkcell(obj,nil)),
				mkcell(transform(list->CELLcdr),nil))));
	}
	if (obj->CELLcar == CONVcell(_commasym))
	   return(mkcell(conssym,
			 mkcell(eval(transform(obj->CELLcdr)),
				mkcell(transform(list->CELLcdr),nil))));
	if (obj->CELLcar == CONVcell(_atsym))
	   return(mkcell(concsym,
			 mkcell(eval(transform(obj->CELLcdr)),
				mkcell(transform(list->CELLcdr),nil))));
	return(mkcell(conssym,
		      mkcell(transform(obj),
			     mkcell(transform(list->CELLcdr),nil))));
} /* transform */

printaux (flag,expr,chan,max)  /* ------------------------------- auxiliary */
int	 flag;
register kerncell expr;
iochan	 chan;
int	 max;		/* max specifies an upper bound when flag is LENGTH */
{
	if (ISsym(expr))			       /* is expr a symbol? */
	   return(bufprint((flag == PRINC && *CONVsym(expr)->name == '|'
			    ? STRIP : flag),
			   chan,"%s",CONVsym(expr)->name));
	switch (expr->flag) {
	  case INTOBJ:
		return(bufprint(flag,chan,"%1d",expr->CELLinum));
	  case REALOBJ:
		return(bufprint(flag,chan,"%f",expr->CELLrnum));
	  case STROBJ:
		return(bufprint(flag,chan,
				(flag == PRINC ? "%s" : "\"%s\""),
				expr->CELLstr));
	  case CHANOBJ:
		return(bufprint(flag,chan,"<channel:%1d>",expr->CELLchan));
	  case VECTOROBJ:
		return(bufprint(flag,chan,"vector[%1d]",
					  expr->CELLdim->CELLinum));
	  case LISTOBJ:
		if (expr->CELLcar == CONVcell(quotesym)) {
		   bufprint(flag,chan,"'");
		   return(1 + printaux(flag,expr->CELLcdr->CELLcar,chan,max));
		}
	  case SETOBJ:				  /* handles lists and sets */
	     {  int size;
		int oflag = expr->flag;
		size = bufprint(flag,chan,(oflag == LISTOBJ ? "(" : "{"));
		do {
		   if (flag == LENGTH && size > max)
		      return(size);
		   size += printaux(flag,expr->CELLcar,chan,max);
		   if ((expr = expr->CELLcdr) != NIL) {
		      if (expr->flag != oflag) {
			 if (flag == LENGTH && size > max)
			    return(size);
			 size += bufprint(flag,chan," . ");
			 size += printaux(flag,expr,chan,max);
			 break;
		      }
		      else
			 size += bufprint(flag,chan," ");
		   }
		} while (expr != NIL);
		size += bufprint(flag,chan,(oflag == LISTOBJ ? ")" : "}"));
		return(size);
	     }
	  default:
		return(bufprint(flag,chan,"<@:%1d>",expr->CELLcar));
	} /* switch */
} /* printaux */

bufprint (flag,chan,format,arg)  /* ------------------------ buffered print */
int    flag;
iochan chan;
char   *format;
word   arg;
{
   static char outputbuf[CHANBUFSIZE+2];
   char *outbuf = outputbuf;

	sprintf(outbuf,format,arg);
	if (flag == LENGTH)
	   return(strlen(outputbuf));
	else if (flag == STRIP) {		/* strip |symbol| to symbol */
	   ++outbuf;
	   *(outbuf + strlen(outbuf) - 1) = 0;
	}
	if (chan->len > 0)
	   --(chan->len);		   /* get rid of the last null char */
	do {
	   *(chan->buf + chan->len++) = *outbuf;
	   if (*outbuf == EOL || chan->len > CHANBUFSIZE) {
	      *(chan->buf + chan->len) = 0;
	      fprintf(chan->file,"%s",chan->buf);
	      chan->len = 0;
	      if (!*(outbuf + 1))
		 break;
	   }
	} while (*outbuf++);
	return(strlen(outputbuf));
} /* bufprint */

kerncell
Lopen ()  /* ------------------------------------------- (open 'name 'mode) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2 = ARGnum2;

	CHECKlargs(opensym,2);
	return(openaux(GETstr(opensym,arg1),
		       GETstr(opensym,arg2)));
} /* Lopen */

kerncell
openaux (name,mode)  /* ------------------------------------ open a channel */
char *name, *mode;
{
   FILE *file, *fopen();

	if ((file = fopen(name,mode)) == NULL)
	   error(opensym,"can't open file",_mkstr(name));
	return(mkchan(openchan(file,
			       (*mode == 'r'
				? (*++mode != 0 ? INOUTCHAN : INCHAN)
				: OUTCHAN))));
} /* openaux */

kerncell
Lclose ()  /* ----------------------------------------------- (close 'chan) */
{
   kerncell arg = ARGnum1;

	CHECKlargs(closesym,1);
	closechan(GETchan(closesym,arg));
	arg->flag = VOID;		      /* arg is no longer a channel */
	return(TTT);
} /* Lclose */

kerncell
Vflush ()  /* --------------------------------------------- (flush ['chan]) */
{
   kerncell arg;
   iochan chan;

	CHECKvargs2(flushsym,1);
	chan = (ARGidx1 == argtop
		? _outchan
		: (arg = ARGnum1, GETchan(flushsym,arg)));
	if (chan->len == 0)
	   return(TTT);
	if (chan->mode == OUTCHAN || chan->mode == INOUTCHAN)
	   bufprint(PRINT,chan,"\n");
	else
	   chan->len = 0;
	return(TTT);
} /* Vflush */

kerncell
Vread ()  /* ----------------------------------------------- (read ['chan]) */
{
   kerncell arg;

	CHECKvargs2(readsym,1);
	if (argtop == ARGidx1)
	   return(readaux(_inchan,0));
	else {
	   if (!ISchan(arg = ARGnum1) || arg->CELLchan->mode == OUTCHAN)
	      error(readsym,err_chan2,arg);
	   return(readaux(arg->CELLchan,0));
	}
} /* Vread */

kerncell
Vprint ()  /* --------------------------------------- (print 'expr ['chan]) */
{
   kerncell arg2;

	CHECKvargs(printsym,1,2);
	if (argtop - ARGidx1 == 1)
	   printaux(PRINT,ARGnum1,_outchan);
	else {
	   if (!ISchan(arg2 = ARGnum2) || arg2->CELLchan->mode == INCHAN)
	      error(printsym,err_chan2,arg2);
	   printaux(PRINT,ARGnum1,arg2->CELLchan);
	}
	return(TTT);
} /* Vprint */

kerncell
Vprinc ()  /* --------------------------------------- (princ 'expr ['chan]) */
{
   kerncell arg2;

	if (argtop - ARGidx1 == 1)
	   printaux(PRINC,ARGnum1,_outchan);
	else {
	   if (!ISchan(arg2 = ARGnum2) || arg2->CELLchan->mode == INCHAN)
	      error(princsym,err_chan2,arg2);
	   printaux(PRINC,ARGnum1,arg2->CELLchan);
	}
	return(TTT);
} /* Vprinc */

kerncell
Vtab ()  /* ----------------------------------------- (tab 'column ['chan]) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2;
   iochan   chan;

	CHECKvargs(tabsym,1,2);
	if (argtop - ARGidx1 == 1)
	   chan = _outchan;
	else {
	   if (!ISchan(arg2 = ARGnum2) || arg2->CELLchan->mode == INCHAN)
	      error(tabsym,err_chan2,arg2);
	   chan = arg2->CELLchan;
	}
	arg1 = ARGnum1;
	tab(GETint(tabsym,arg1),chan);
	return(TTT);
} /* Vtab */

tab (column,chan)  /* ------------------------------------------------- tab */
int column;
iochan chan;
{
	if (column > CHANBUFSIZE)
	   column = CHANBUFSIZE;
	if (chan->len > column)
	   bufprint(PRINT,chan,"\n");
	if (column < 0)
	   return;
	while (chan->len < column)
	   *(chan->buf + chan->len++) = SPACE;
	*(chan->buf + chan->len) = 0;
} /* tab */

kerncell
Vterpri ()  /* ------------------------------------------- (terpri ['chan]) */
{
   kerncell arg;

	CHECKvargs2(terprisym,1);
	if (argtop == ARGidx1)
	   bufprint(PRINT,_outchan,"\n");
	else {
	   if (!ISchan(arg = ARGnum1) || arg->CELLchan->mode == INCHAN)
	      error(terprisym,err_chan2,arg);
	   bufprint(PRINT,arg->CELLchan,"\n");
	}
	return(TTT);
} /* Vterpri */

kerncell
Vprlen ()  /* ---------------------------------------- (prlen 'expr ['max]) */
{
   kerncell arg2;
   int	    max;

	CHECKvargs(prlensym,1,2);
	max = (argtop - ARGidx1 == 1
	       ? MAXCOLS
	       : (arg2 = ARGnum2, GETint(prlensym,arg2)));
	return(mkinum(printaux(LENGTH,ARGnum1,_outchan,max)));
} /* Vprlen */

kerncell
Viobuf ()  /* --------------------------------------------- (iobuf ['chan]) */
{
   kerncell arg;

	CHECKvargs2(iobufsym,1);
	return(mkinum(ARGidx1 == argtop
		      ? _outchan->len
		      : (arg = ARGnum1, GETchan(iobufsym,arg)->len)));
} /* Viobuf */

kerncell
Lchanp ()  /* ----------------------------------------------- (chan? 'expr) */
{
	CHECKlargs(chanpsym,1);
	return(ISchan(ARGnum1) ? TTT : NIL);
} /* Lchanp */

kerncell
Vpp ()  /* --------------------------------------------- (pp 'expr ['chan]) */
{
   kerncell arg1 = ARGnum1;
   kerncell arg2;

	CHECKvargs(ppsym,1,2);
	ppgap = MAXCOLS;
	pp((ISfun(arg1) ? CONVsym(arg1)->bind : arg1),
	   (argtop - ARGidx1 == 1
	    ? _outchan
	    : (!ISchan(arg2 = ARGnum2) || arg2->CELLchan->mode == INCHAN
	       ? CONVchan(error(ppsym,err_chan2,arg2))
	       : arg2->CELLchan)),
	   0,0);
	return(TTT);
} /* Vpp */

pp (expr,chan,lmar,rmar)  /* --------- pretty print expr within the margins */
register kerncell expr;
iochan	 chan;
int	 lmar, rmar;
{
   int flag = expr->flag;
   int lmar1;

	if (lmar > llimit && printaux(LENGTH,expr,chan,rlimit) > rlimit) {
	   bufprint(PRINT,chan,"\n; <<=== continued left ===<<");
	   pp(expr,chan,4,0);
	   bufprint(PRINT,chan,"\n; >>=== continued right ===>>\n");
	   return;
	}
	tab(lmar,chan);
	if (!ISlist(expr)) {
	   printaux(PRINT,expr,chan);
	   return;
	}
	bufprint(PRINT,chan,(flag == LISTOBJ ? "(" : "{"));
	if (printlen(expr,chan,rmar) < ppgap)
	   do {
	      pp(expr->CELLcar,chan,chan->len,rmar);
	      if ((expr = expr->CELLcdr) != NIL) {
		 if (expr->flag != flag) {
		    bufprint(PRINT,chan," . ");		     /* dotted pair */
		    pp(expr,chan,chan->len,rmar);
		    break;
		 }
		 else
		    bufprint(PRINT,chan," ");
	      }
	   } while (expr != NIL);
	else {
	   if (!ISlist(expr->CELLcar) && ISlist(expr->CELLcdr->CELLcdr)) {
	      pp(expr->CELLcar,chan,chan->len,rmar);
	      bufprint(PRINT,chan," ");
	      expr = expr->CELLcdr;
	   }
	   lmar1 = chan->len;			      /* freeze left margin */
	   do {
	      pp(expr->CELLcar,chan,lmar1,
		 (expr->CELLcdr == NIL ? rmar + 1 : rmar));
	      if ((expr = expr->CELLcdr) != NIL) {
		 if (expr->flag != flag) {
		    bufprint(PRINT,chan," . ");		     /* dotted pair */
		    pp(expr,chan,lmar1,rmar);
		    break;
		 }
		 else
		    bufprint(PRINT,chan," ");
	      }
	   } while (expr != NIL);
	}
	bufprint(PRINT,chan,(flag == LISTOBJ ? ")" : "}"));
} /* pp */

printlen (expr,chan,rmar)  /* ------------------------------ length of expr */
kerncell expr;
iochan	 chan;
int	 rmar;
{
   int len;

	ppgap = ppcols - chan->len;
	len = printaux(LENGTH,expr,chan,ppgap);
	return(rmar + (len > ppgap ? ppgap : len));
} /* printlen */
