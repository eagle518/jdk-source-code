all: jinvoker.dll invoked.class invoker.exe

jinvoker.dll: jinvoker.obj Makefile
	cl /LD /o jinvoker.dll jinvoker.obj

jinvoker.obj: jinvoker.cpp jinvokerExp.h Makefile
	cl /Fojinvoker.obj /DJINVOKEREEXPORT /c jinvoker.cpp

invoked.class: invoked.java Makefile
	javac invoked.java

invoker.exe: invoker.cs Makefile
	csc /unsafe invoker.cs
