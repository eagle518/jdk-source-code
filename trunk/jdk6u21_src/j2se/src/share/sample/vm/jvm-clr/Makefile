all: invoker.dll invoker.class invoked.exe

invoker.dll: invoker.obj Makefile
	cl /LD /o invoker.dll invoker.obj mscoree.lib

invoker.obj: invoker.cpp invokerExp.h invoker.h Makefile
	cl /Foinvoker.obj /DINVOKEREXPORT /c invoker.cpp

invoker.class: invoker.java Makefile
	javac invoker.java

invoked.exe: invoked.cs Makefile
	csc invoked.cs
