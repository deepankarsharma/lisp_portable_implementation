/* KERNEL, 6/7/87 -- S. Hekmatpour
 * cell table management routines:
 * The cell table is organized as an array of blocks.
 * Blocks are allocated on demand, and are automatically garbage collected
 * once the table is full, or once the dynamic storage is exhausted.
 */
#include "kernel.h"

extern	 kernsym symtab[];
kerncell celltab[CELLTABSIZE];  /* cell table */
int celltabsize = CELLTABSIZE;  /* cell table size */
int celltabidx = 0;		/* current index to cell table */
int blockidx;			/* index to a cell in current block */
int phase1 = 1;			/* block allocation phase */
kerncell blockptr;		/* block pointer */
kerncell inumblock;		/* small inum block */
kerncell freelist = NULL;	/* used only after garbage collection */

char *
new (size)  /* ------------------------------------- allocates 'size' bytes */
int size;
{
   char *res, *malloc();

	if ((res = malloc(size)) == NULL) {
	   phase1 = 0;			/* terminate block allocation phase */
	   celltabsize = celltabidx;		 /* freeze celltab's growth */
	   collectgarb();
	   if ((res = malloc(size)) == NULL)		       /* try again */
	      faterr(err_memory);
	}
	return(res);
} /* new */

initcelltab ()  /* ------------------------------ initialize the cell table */
{
   int	    range = SMALLINTHIGH - SMALLINTLOW;
   char	    *malloc();
   register int blkidx;
   register kerncell blkptr;

	if (range >= BLOCKSIZE)
	   faterr("BLOCKSIZE is too small");
	if ((celltab[celltabidx] = blkptr = inumblock =
	     (kerncell) malloc(sizeof(struct cell) * BLOCKSIZE)) == NULL)
	   faterr(err_memory);
	for (blkidx=0; blkidx <= range; ++blkidx) {	     /* small inums */
	    blkptr->flag = INTOBJ;
	    (blkptr++)->CELLinum = SMALLINTLOW + blkidx;
	}
	blockidx = blkidx;
	blockptr = blkptr + 1;
} /* initcelltab */

kerncell
freshcell ()  /* -------------------------------- returns a fresh cons-cell */
{
   static kerncell freecell;
   char	  *malloc();

	if (phase1) {		/* in this phase storage is still available */
	   if (blockidx++ < BLOCKSIZE) {       /* get it from current block */
	      blockptr->CELLcdr = NIL;
	      return(CELLpush(blockptr++));
	   }
	   if (++celltabidx < celltabsize) {	      /* create a new block */
	      if ((celltab[celltabidx] = blockptr = (kerncell)
		  malloc (sizeof(struct cell) * BLOCKSIZE)) == NULL) {
		 celltabsize = celltabidx;
		 goto phase2;		   /* run out of storage --> phase2 */
	      }
	      blockidx = 1;
	      blockptr->CELLcdr = NIL;
	      return(CELLpush(blockptr++));
	   }
     phase2:			      /* in this phase storage is exhausted */
	   phase1 = 0;
	}
	if (freelist == NULL)
	   if (collectgarb() == NULL)		  /* try garbage collecting */
	      faterr("cons-cell storage exhausted");
	freecell = freelist;
	freelist = freelist->CELLcdr;
	freecell->CELLcdr = NIL;
	return(CELLpush(freecell));
} /* freshcell */

kerncell
collectgarb ()  /* ---------------------------------------- collect garbage */
{
   register int i, cidx;
   register kernsym  entry;
   register kerncell blockptr;

	/* mark: */
	for (i=0; i < HASHTABSIZE; ++i) {
	    entry = symtab[i];
	    while (entry) {		       /* mark every symbol's refs. */
	       if (ISnotbinary(entry))
		  mark(entry->bind);			   /* mark bindings */
	       mark(entry->prop);		     /* mark property lists */
	       entry = entry->link;
	    }
	}
	for (i=0; i <= vartop; ++i)   /* mark bindings of vars on var stack */
	    mark(varstk[i].bind);
	for (i=0; i <= argtop; ++i) {		  /* mark args on arg stack */
	    if (CONVint(argstk[i]) > ARGSTKSIZE)	  /* ignore indices */
	       mark(argstk[i]);
	}
	for (i=celltop; i < EVALSTKSIZE; ++i)	/* mark cells on cell stack */
	    mark(evalstk[i]);

	/* sweep: */
	blockptr = celltab[0];
	for (i=0; i < BLOCKSIZE; ++i)		   /* unmark small integers */
	    (blockptr++)->flag &= MASK7;
	for (cidx=1; cidx < celltabsize; ++cidx) {		   /* sweep */
	    blockptr = celltab[cidx];
	    for (i=0; i < BLOCKSIZE; ++i) {
		if (ISmarked(blockptr))			    /* cell in use? */
		   (blockptr++)->flag &= MASK7;		       /* unmark it */
		else {					 /* cell not in use */
		   blockptr->CELLcdr = freelist;		 /* free it */
		   freelist = blockptr;
		   switch (blockptr->flag) {
		     case STROBJ:
				free(blockptr->CELLstr);
				break;
		     case CHANOBJ:
				closechan(blockptr->CELLchan);
				break;
		     case VECTOROBJ:
				free(blockptr->CELLvec);
				break;
		   }
		   ++blockptr;
		}
	    }
	}
	return(freelist);
} /* collectgarb */

mark (obj)  /* --------------------------------- mark cells that are in use */
register kerncell obj;
{
	if (ISsym(obj) || ISmarked(obj))	 /* symbols need no marking */
	   return;
	switch (obj->flag) {
	  case VECTOROBJ:
	     {  register int dim = obj->CELLdim->CELLinum;
		register kerncell *vec = obj->CELLvec;
		obj->flag |= MARK;			     /* mark vector */
		obj->CELLdim->flag |= MARK;	   /* mark vector dimension */
		while (dim--)			    /* mark vector elements */
		   mark(*vec++);
		return;
	     }
	  case LISTOBJ:
	  case SETOBJ:			       /* sets are treated as lists */
		while (ISlist(obj)) {
		   obj->flag |= MARK;			  /* mark this cell */
		   mark(obj->CELLcar);		       /* mark list element */
		   obj = obj->CELLcdr;
		}
		if (obj != NIL)				    /* dotted pair? */
		   mark(obj);
		return;
	  default:
		obj->flag |= MARK;		  /* mark elementary object */
		return;
	}
} /* mark */

kerncell
mkinum(inum)  /* ----------------------------------- make an integer object */
int inum;
{
   kerncell obj;

	if (inum >= SMALLINTLOW && inum <= SMALLINTHIGH)
	   return(inumblock + inum - SMALLINTLOW);
	obj = freshcell();
	obj->flag = INTOBJ;
	obj->CELLinum = inum;
	return(obj);
} /* mkinum */

kerncell
mkrnum(rnum)  /* --------------------------------------- make a real object */
real rnum;
{
   kerncell obj = freshcell();

	obj->flag = REALOBJ;
	obj->CELLrnum = rnum;
	return(obj);
} /* mkrnum */

kerncell
mkstr(str)  /* --------------------------------------- make a string object */
char *str;
{
   kerncell obj = freshcell();
   char	    *newstr;
   int	    len = strlen(str);

	newstr = new(len + 1);
	strcpy(newstr,str);
	*(newstr + len) = 0;
	obj->flag = STROBJ;
	obj->CELLstr = newstr;
	return(obj);
} /* mkstr */

kerncell
_mkstr (str)  /* --------------------------- make a temporary string object */
char *str;
{
	_tempstr->CELLstr = str;
	return(_tempstr);
} /* _mkstr */

kerncell
mkchan (chan)  /* ----------------------------------- make a channel object */
iochan chan;
{
   kerncell obj = freshcell();

	obj->flag = CHANOBJ;
	obj->CELLchan = chan;
	return(obj);
} /* mkchan */

kerncell
mkcell (head, tail)  /* ------------------------------ make a new cons-cell */
kerncell head, tail;
{
   kerncell obj = freshcell();

	obj->flag = LISTOBJ;
	obj->CELLcar = head;
	obj->CELLcdr = tail;
	return(obj);
} /* mkcell */

kerncell
mkset (head, tail)  /* ------------------------------- make a new cons-cell */
kerncell head, tail;
{
   kerncell obj = freshcell();

	obj->flag = SETOBJ;
	obj->CELLcar = head;
	obj->CELLcdr = tail;
	return(obj);
} /* mkset */
