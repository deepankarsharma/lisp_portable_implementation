The book is available on archive.org [HERE](https://archive.org/details/lispportableimpl0000hekm)

This source code was taken from the tar file available on archive.org [HERE](https://archive.org/details/kernel-lisp)

The source code of Kernel is broken down into a number of files where each file consists of a set of closely related functions. A summary of the files is given below.

1. **kernel.h** Contains header definitions. These include type declarations, macro-definitions, and external declarations. All global variables that are referred to by more than one file and all C functions that implement Kernel functions (and some lower-level functions) are declared as external in this file. By including this file in every other file, redeclarations of global types, variables, and functions are avoided.

2. **globals.c** All global variables that are declared as external in kernel.h are defined in this file. The isolation of global variables in this manner achieves better consistency.

3. **symt.c** Contains the symbol table management functions and a set of related functions for creating symbols.

4. **cellt.c** Contains the cons-cell table management functions and a set of related functions for creating objects.

5. **eval.c** Contains the evaluation functions of Kernel. Functions that implement the various function types of Kernel appear in this file.

6. **io.c** Contains the I/O management functions of Kernel.

7. **arith.c** Contains the arithmetic functions of Kernel.

8. **str.c** Contains the string manipulation functions of Kernel.

9. **sym.c** Contains the symbol manipulation functions of Kernel.

10. **list.c** Contains the list manipulation functions of Kernel.

11. **set.c** Contains the set manipulation functions of Kernel.

12. **logic.c** Contains the logic and conditional functions of Kernel.

13. **prop.c** Contains the property list manipulation and the association list functions of Kernel.

14. **vec.c** Contains the vector manipulation functions of Kernel.

15. **flow.c** Contains functions for the nonstandard flow of control in Kernel. The error and top level functions of Kernel appear in this file.

16. **iter.c** Contains the iteration functions of Kernel.

17. **map.c** Contains the mapping functions of Kernel.

18. **misc.c** Contains some miscellaneous functions for, for example, defining variables, constants, and functions.

19. **init.c** Contains the initialization function of Kernel.

20. **kern.c** Contains the main driver function of Kernel.

21. **kcomp.c** Contains the Kernel compiler for translating Kernel programs into C.
