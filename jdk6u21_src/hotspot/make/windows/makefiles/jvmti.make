#
# Copyright (c) 2003, 2008, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#  
#

# This makefile (jvmti.make) is included from the jvmti.make in the
# build directories.
#
# It knows how to build and run the tools to generate jvmti.

!include $(WorkSpace)/make/windows/makefiles/rules.make

# #########################################################################

JvmtiSrcDir = $(WorkSpace)/src/share/vm/prims
InterpreterSrcDir = $(WorkSpace)/src/share/vm/interpreter

JvmtiGeneratedNames = \
        jvmtiEnv.hpp \
        jvmtiEnter.cpp \
        jvmtiEnterTrace.cpp \
        jvmtiEnvRecommended.cpp \
        bytecodeInterpreterWithChecks.cpp \
        jvmti.h \

JvmtiEnvFillSource = $(JvmtiSrcDir)/jvmtiEnvFill.java
JvmtiEnvFillClass = $(JvmtiOutDir)/jvmtiEnvFill.class

JvmtiGenSource = $(JvmtiSrcDir)/jvmtiGen.java
JvmtiGenClass = $(JvmtiOutDir)/jvmtiGen.class

#Note: JvmtiGeneratedFiles must be kept in sync with JvmtiGeneratedNames by hand.
#Should be equivalent #to "JvmtiGeneratedFiles = $(JvmtiGeneratedNames:%=$(JvmtiOutDir)/%)"
JvmtiGeneratedFiles = \
        $(JvmtiOutDir)/jvmtiEnv.hpp \
        $(JvmtiOutDir)/jvmtiEnter.cpp \
        $(JvmtiOutDir)/jvmtiEnterTrace.cpp \
        $(JvmtiOutDir)/jvmtiEnvRecommended.cpp\
        $(JvmtiOutDir)/bytecodeInterpreterWithChecks.cpp\
        $(JvmtiOutDir)/jvmti.h \

XSLT = $(RUN_JAVA) -classpath $(JvmtiOutDir) jvmtiGen

# #########################################################################

both = $(JvmtiGenClass) $(JvmtiSrcDir)/jvmti.xml $(JvmtiSrcDir)/jvmtiLib.xsl

default::
        @if not exist $(JvmtiOutDir) mkdir $(JvmtiOutDir)

$(JvmtiGenClass): $(JvmtiGenSource)
	$(COMPILE_JAVAC) -d $(JvmtiOutDir) $(JvmtiGenSource)

$(JvmtiEnvFillClass): $(JvmtiEnvFillSource)
	@$(COMPILE_JAVAC) -d $(JvmtiOutDir) $(JvmtiEnvFillSource)

$(JvmtiOutDir)/jvmtiEnter.cpp: $(both) $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiEnter.xsl -OUT $(JvmtiOutDir)/jvmtiEnter.cpp -PARAM interface jvmti

$(JvmtiOutDir)/bytecodeInterpreterWithChecks.cpp: $(JvmtiGenClass) $(InterpreterSrcDir)/bytecodeInterpreter.cpp $(InterpreterSrcDir)/bytecodeInterpreterWithChecks.xml $(InterpreterSrcDir)/bytecodeInterpreterWithChecks.xsl
	@echo Generating $@
	@$(XSLT) -IN $(InterpreterSrcDir)/bytecodeInterpreterWithChecks.xml -XSL $(InterpreterSrcDir)/bytecodeInterpreterWithChecks.xsl -OUT $(JvmtiOutDir)/bytecodeInterpreterWithChecks.cpp

$(JvmtiOutDir)/jvmtiEnterTrace.cpp: $(both) $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiEnter.xsl -OUT $(JvmtiOutDir)/jvmtiEnterTrace.cpp -PARAM interface jvmti -PARAM trace Trace

$(JvmtiOutDir)/jvmtiEnvRecommended.cpp: $(both) $(JvmtiSrcDir)/jvmtiEnv.xsl $(JvmtiSrcDir)/jvmtiEnv.cpp $(JvmtiEnvFillClass)
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiEnv.xsl -OUT $(JvmtiOutDir)/jvmtiEnvStub.cpp
	@$(RUN_JAVA) -classpath $(JvmtiOutDir) jvmtiEnvFill $(JvmtiSrcDir)/jvmtiEnv.cpp $(JvmtiOutDir)/jvmtiEnvStub.cpp $(JvmtiOutDir)/jvmtiEnvRecommended.cpp

$(JvmtiOutDir)/jvmtiEnv.hpp: $(both) $(JvmtiSrcDir)/jvmtiHpp.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiHpp.xsl -OUT $(JvmtiOutDir)/jvmtiEnv.hpp

$(JvmtiOutDir)/jvmti.h: $(both) $(JvmtiSrcDir)/jvmtiH.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiH.xsl -OUT $(JvmtiOutDir)/jvmti.h

jvmtidocs:  $(JvmtiOutDir)/jvmti.html

$(JvmtiOutDir)/jvmti.html: $(both) $(JvmtiSrcDir)/jvmti.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmti.xsl -OUT $(JvmtiOutDir)/jvmti.html

# #########################################################################

cleanall :
	rm $(JvmtiGenClass) $(JvmtiEnvFillClass) $(JvmtiGeneratedFiles)

# #########################################################################

.PHONY: jvmtidocs cleanall
