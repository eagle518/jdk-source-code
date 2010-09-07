## Makefile for a local directory
## Author: Mike Paleczny, Copyright (C) 1997-1998
#
#  /usr/ccs/bin/make

# #########################################################################

EXEC	= ../adlc
TARGETS	= comments.cpp register.cpp \
	match.cpp encode.cpp \
	predicate.cpp test2.cpp

# #########################################################################

test2.cpp	: test2 $(EXEC)
	$(EXEC) test2

all	: $(TARGETS)


# #########################################################################

comments.cpp : comments
	$(EXEC) comments

register.cpp : register
	$(EXEC) register

# #########################################################################

source.cpp : source $(EXEC)
	$(EXEC) source

op_attrib.cpp : op_attrib $(EXEC)
	$(EXEC) op_attrib

ins_attrib.cpp : ins_attrib $(EXEC)
	$(EXEC) ins_attrib

oper_class.cpp : oper_class $(EXEC)
	$(EXEC) oper_class

match.cpp : match $(EXEC)
	$(EXEC) match

encode.cpp : encode $(EXEC)
	$(EXEC) encode

predicate.cpp : predicate $(EXEC)
	$(EXEC) predicate

test.cpp : test $(EXEC)
	$(EXEC) test

test2.cpp	: test2 $(EXEC)
	$(EXEC) test2

## foo.cpp : foo $(EXEC)
## 	   $(EXEC) foo
## 

# #########################################################################

clean	:
	rm $(TARGETS)

cleanall :
	rm *.cpp *.hpp

# #########################################################################



