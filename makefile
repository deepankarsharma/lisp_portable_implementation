OBJ1 =	arith.obj cellt.obj eval.obj flow.obj globals.obj
OBJ2 =	init.obj io.obj iter.obj list.obj logic.obj map.obj
OBJ3 =	misc.obj prop.obj set.obj str.obj sym.obj symt.obj
OBJ4 =	vec.obj
OBJ =	$(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)
CFLAGS = -AL -Ot

all: kern.exe kcomp.exe

kern.exe: $(OBJ) kern.obj link.rsp
	link kern.obj @link.rsp,a.exe;
	exemod a.exe /stack 5000
	exepack a.exe kern.exe
	del a.exe

kcomp.exe: $(OBJ) kcomp.obj link.rsp
	link kcomp.obj @link.rsp,a.exe;
	exemod a.exe /stack 5000
	exepack a.exe kcomp.exe
	del a.exe

link.rsp: makefile
	echo $(OBJ1)+ > link.rsp
	echo $(OBJ2)+ >> link.rsp
	echo $(OBJ3)+ >> link.rsp
	echo $(OBJ4) >> link.rsp

$(OBJ) kcomp.obj kern.obj: kernel.h 
