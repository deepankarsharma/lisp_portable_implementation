/* KERNEL, 6/7/87 -- sh
 * symbol table management routines:
 * The symbol table is organized as a hash and link structure.
 * Entries in each linked list are kept alphabetically sorted.
 */
#include "kernel.h"
kernsym symtab [HASHTABSIZE];	 /* symbol table */

initsymtab ()  /* ----------------------------- initialize the symbol table */
{
   register int i;

	for (i=0; i < HASHTABSIZE; ++i)
	    symtab[i] = NULL;
} /* initsymtab */

int
hash (name)  /* --------------------------------------------- hash function */
register char *name;
{
   register int hashaddr = 0;

	while (*name)
	      hashaddr += *name++;
	return (hashaddr % HASHTABSIZE);
} /* hash */

kernsym
addsym (name)  /* -------------------------- add symbol to the symbol table */
char *name;
{
   int	    hashaddr = hash (name);
   int	    len = strlen(name);
   register kernsym newsym, sym;

	newsym = CONVsym(new(sizeof(struct symbol)));
	newsym->flag = UNBOUND;
	newsym->name = new(len + 1);
	strcpy(newsym->name,name);
	*(newsym->name + len) = 0;
	if ((sym = symtab[hashaddr]) == NULL ||  /* insert in front of list */
	    strcmp(name, sym->name)  < 0) {
	   symtab[hashaddr] = newsym;
	   newsym->link = sym;
	} else if (sym->link == NULL) {	       /* append to the end of list */
	   sym->link = newsym;
	   newsym->link = NULL;
	} else {					  /* insert in list */
	   while (strcmp(name,sym->link->name) > 0 &&
		  (sym = sym->link)->link)
		 ;
	   newsym->link = sym->link;
	   sym->link = newsym;
	}
	newsym->prop = NIL;
	return(newsym);
} /* addsym */

kernsym
findsym (name)  /* ---------------------- find a symbol in the symbol table */
char *name;
{
   register kernsym sym = symtab[hash(name)];
   int	    cmp;

	while (sym != NULL && (cmp = strcmp(name,sym->name)) > 0)
	   sym = sym->link;
	if (sym == NULL || cmp < 0)			       /* not found */
	   return(NULL);
	return(sym);						   /* found */
} /* findsym */

kernsym
mksym (name)  /* ------------------------------------- make a symbol object */
register char *name;
{
   kernsym sym = findsym(name);

	if (sym == NULL && ISunbound(sym = addsym(name)) && *name == 'c') {
	   while (*++name == 'a' || *name == 'd')     /* look for cxxr form */
	      ;
	   if (*name == 'r' && *++name == 0) {
	      sym->flag = LBINARY;
	      sym->bind = CONVcell(Lcxxr);	  /* see evalcall in eval.c */
	   }
	}
	return(sym);
} /* mksym */

kernsym
_mksym (name)  /* --------------------------------- make a temporary symbol */
char *name;
{
	_tempsym->name = name;
	return(_tempsym);
} /* _mksym */

kernsym
newsym (name, flag, bind)  /* -------------------- make a new symbol object */
char *name;	  /* assumes that symbol is not already in the symbol table */
byte flag;
kerncell bind;
{
   kernsym sym;

	sym = addsym(name);
	sym->flag = flag;
	sym->bind = bind;
	return(sym);
} /* newsym */
