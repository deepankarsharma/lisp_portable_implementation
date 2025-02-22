/* KERNEL, 28/9/87 -- S. Hekmatpour
 * compilation routines:
 * The compiler produces 'C' code which replaces the loading process of
 * a KERNEL file. The produced 'C' code should be compiled and linked
 * with the object code of KERNEL itself.
 */
#include "kernel.h"

typedef struct snode {		/* symbol/string node */
	char *str;			/* string/symbol name */
	kernsym sym;			/* denoting variable */
	struct snode *left, *right;	/* child nodes */
} *stree;
typedef struct inode {		/* integer node */
	int inum;			/* integer value */
	kernsym sym;			/* denoting variable */
	struct inode *left, *right;	/* child nodes */
} *itree;
typedef struct rnode {		/* real node */
	real rnum;			/* real value */
	kernsym sym;			/* denoting variable */
	struct rnode *left, *right;	/* child nodes */
} *rtree;

stree symtree  = NULL;		/* symbol tree */
stree strtree  = NULL;		/* string tree */
itree inumtree = NULL;		/* integer tree */
rtree rnumtree = NULL;		/* real tree */
kernsym lastsym;		/* last symbol */
char *source, *target;		/* source and target file names */

main (argn,argv)  /* ------------------------------------------------- main */
int  argn;
char *argv[];
{
   int	    len;
   char	    *malloc();
   kerncell compile();

	if (argn <= 1) {
	   fprintf(stderr,"no source file\n");
	   exit(1);
	}
	if ((len = strlen(source = argv[1])) < 3 ||	    /* check source */
	    source[len-2] != '.' || source[len-1] != 'k') {
	   fprintf(stderr,"bad source file\n");
	   exit(1);
	}
	if (argn >= 3) {
	   if ((len = strlen(target = argv[2])) < 3 ||	    /* check target */
	       target[len-2] != '.' || target[len-1] != 'c') {
	      fprintf(stderr,"bad target file\n");
	      exit(1);
	   }
	}
	else {						  /* default target */
	   target = malloc(len + 1);
	   strcpy(target,source);
	   target[len-1] = 'c';
	   target[len] = 0;
	}
	initialize();
	if (catch(compile,_toptagsym,1) == NIL) {
	   fprintf(stderr,"compilation aborted\n");
	   exit(1);
	}
	exit(0);
} /* main */

kerncell
compile ()  /* ---------------------------------------------------- compile */
{
   kerncell compaux();

	return(caperr(compaux,NIL,1));
} /* compile */

kerncell
compaux ()  /* -------------------------------------------------- auxiliary */
{
   kerncell schan, tchan;
   register kerncell form, forms = NIL;
   kerncell procform();

	schan = openaux(source,"r");			/* open source file */
	tchan = openaux(target,"w");			/* open target file */
	/* read and eval ann forms in source (for macro expansion): */
	while (eval(readaux(schan->CELLchan,0)) != CONVcell(eofsym))
	   ;
	closechan(schan->CELLchan);			/* close source file */
	schan = openaux(source,"r");		      /* re-open source file */
	/* process each form in source: */
	while ((form = readaux(schan->CELLchan,0)) != CONVcell(eofsym))
	   forms = mkcell(procform(form,1),forms);
	closechan(schan->CELLchan);			/* close source file */
	forms = dreverse(forms);
	/* produce object code: */
	bufprint(PRINC,tchan->CELLchan,
		 "#include \"/user/hekmat/kern/kernel.h\"\n");
	bufprint(PRINC,tchan->CELLchan,"auxiliary ()\n{\n");
	gensyms(symtree,tchan->CELLchan);		/* generate symbols */
	genstrs(strtree,tchan->CELLchan);		/* generate strings */
	geninums(inumtree,tchan->CELLchan);	       /* generate integers */
	genrnums(rnumtree,tchan->CELLchan);		  /* generate reals */
	gencode(forms,tchan->CELLchan);	     /* generate code for the forms */
	bufprint(PRINC,tchan->CELLchan,"}\n");
	bufprint(PRINC,tchan->CELLchan,			   /* main function */
		       "main() {initialize(); auxiliary(); topexec();}\n");
	closechan(tchan->CELLchan);
	return(NIL);
} /* compaux */

kerncell
procform (form,expd)  /* ----------------------------------- process a form */
register kerncell form;
int expd;
{
   kerncell convert(), head;
   stree    addstr();
   itree    addinum();
   rtree    addrnum();

	if (ISsym(form)) {
	   symtree = addstr(symtree,CONVsym(form)->name);
	   return(CONVcell(lastsym));
	}
	if (ISstr(form)) {
	   strtree = addstr(strtree,form->CELLstr);
	   return(CONVcell(lastsym));
	}
	if (ISint(form)) {
	   inumtree = addinum(inumtree,form->CELLinum);
	   return(CONVcell(lastsym));
	}
	if (ISreal(form)) {
	   rnumtree = addrnum(rnumtree,form->CELLrnum);
	   return(CONVcell(lastsym));
	}
	if (form->CELLcar == CONVcell(quotesym))
	   return(convert(form,0));
	if (expd &&
	    ((ISfun(form->CELLcar) &&
	      ((head = CONVsym(form->CELLcar)->bind)->CELLcar)
	       == CONVcell(mlamsym)) ||
	     (ISlist(head = form->CELLcar) &&
	      (head->CELLcar == CONVcell(mlamsym)))))
	   procform(expand(head,form,0),expd);
	return(convert(form,expd));
} /* procform */

kerncell
convert (form,expd)  /* ------------------------------------ convert a form */
kerncell form;
int expd;
{
	if (form == NIL)
	   return(NIL);
	return(mkcell(procform(form->CELLcar,expd),
		      convert(form->CELLcdr,expd)));
} /* convert */

stree
addstr (tree,str)  /* ------------- add string/symbol to string/symbol tree */
stree tree;
char *str;
{
   kernsym gensym();
   char	   *malloc();
   int	   cmp;

	if (tree == NULL) {
	   if ((tree = (struct snode *) malloc(sizeof(struct snode))) == NULL) {
	      fprintf(stderr,"insufficient memory\n");
	      exit(1);
	   }
	   tree->str = str;
	   tree->sym = lastsym = gensym();
	   tree->left = tree->right = NULL;
	}
	else if ((cmp = strcmp(str,tree->str)) < 0)
	   tree->left = addstr(tree->left,str);
	else if (cmp > 0)
	   tree->right = addstr(tree->right,str);
	else
	   lastsym = tree->sym;
	return(tree);
} /* addstr */

itree
addinum (tree,inum)  /* ----------------------- add integer to integer tree */
itree tree;
int   inum;
{
   kernsym gensym();
   char	   *malloc();

	if (tree == NULL) {
	   if ((tree = (struct inode *) malloc(sizeof(struct inode))) == NULL) {
	      fprintf(stderr,"insufficient memory\n");
	      exit(1);
	   }
	   tree->inum = inum;
	   tree->sym = lastsym = gensym();
	   tree->left = tree->right = NULL;
	}
	else if (inum < tree->inum)
	   tree->left = addinum(tree->left,inum);
	else if (inum > tree->inum)
	   tree->right = addinum(tree->right,inum);
	else
	   lastsym = tree->sym;
	return(tree);
} /* addinum */

rtree
addrnum (tree,rnum)  /* ----------------------------- add real to real tree */
rtree tree;
real  rnum;
{
   kernsym gensym();
   char	   *malloc();

	if (tree == NULL) {
	   if ((tree = (struct rnode *) malloc(sizeof(struct rnode))) == NULL) {
	      fprintf(stderr,"insufficient memory\n");
	      exit(1);
	   }
	   tree->rnum = rnum;
	   tree->sym = lastsym = gensym();
	   tree->left = tree->right = NULL;
	}
	else if (rnum < tree->rnum)
	   tree->left = addrnum(tree->left,rnum);
	else if (rnum > tree->rnum)
	   tree->right = addrnum(tree->right,rnum);
	else
	   lastsym = tree->sym;
	return(tree);
} /* addrnum */

kernsym
gensym ()  /* --------------------------------------- generate a new symbol */
{
   static int num = 0;

	sprintf(strbuf,"s%04d",num++);
	return(mksym(strbuf));
} /* gensym */

gencode (forms,chan)  /* -------------------------- generate code for forms */
kerncell forms;
iochan chan;
{
	while (ISlist(forms)) {
	   bufprint(PRINC,chan,"eval(");
	   gencells(forms->CELLcar,chan);
	   bufprint(PRINC,chan,");\n");
	   forms = forms->CELLcdr;
	}
} /* gencode */

gencells (form,chan)  /* -------------------------- generate cells for form */
kerncell form;
iochan chan;
{
	if (ISlist(form)) {
	   bufprint(PRINC,chan,"mkcell(");
	   gencells(form->CELLcar,chan);
	   bufprint(PRINC,chan,",");
	   gencells(form->CELLcdr,chan);
	   bufprint(PRINC,chan,")");
	}
	else
	   printaux(PRINC,form,chan);
} /* gencells */

gensyms (tree,chan)  /* ------------------ generate code for making symbols */
stree tree;
iochan chan;
{
	if (tree != NULL) {
	   bufprint(PRINC,chan,"kernsym %s = ",tree->sym->name);
	   bufprint(PRINC,chan,"mksym(\"%s\");\n",tree->str);
	   gensyms(tree->left,chan);
	   gensyms(tree->right,chan);
	}
} /* gensyms */

genstrs (tree,chan)  /* ------------------ generate code for making strings */
stree tree;
iochan chan;
{
	if (tree != NULL) {
	   bufprint(PRINC,chan,"kerncell %s = ",tree->sym->name);
	   bufprint(PRINC,chan,"mkstr(\"%s\");\n",tree->str);
	   genstrs(tree->left,chan);
	   genstrs(tree->right,chan);
	}
} /* genstrs */

geninums (tree,chan)  /* ---------------- generate code for making integers */
itree tree;
iochan chan;
{
	if (tree != NULL) {
	   bufprint(PRINC,chan,"kerncell %s = ",tree->sym->name);
	   bufprint(PRINC,chan,"mkinum(%1d);\n",tree->inum);
	   geninums(tree->left,chan);
	   geninums(tree->right,chan);
	}
} /* geninums */

genrnums (tree,chan)  /* ------------------- generate code for making reals */
rtree tree;
iochan chan;
{
	if (tree != NULL) {
	   bufprint(PRINC,chan,"kerncell %s = ",tree->sym->name);
	   bufprint(PRINC,chan,"mkrnum(%f);\n",tree->rnum);
	   genrnums(tree->left,chan);
	   genrnums(tree->right,chan);
	}
} /* genrnums */
