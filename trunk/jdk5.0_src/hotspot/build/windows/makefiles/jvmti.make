# 
# @(#)jvmti.make	1.9 03/12/23 16:35:34
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

# This makefile (jvmti.make) is included from the jvmti.make in the
# build directories.
#
# It knows how to build and run the tools to generate jvmti/jvmdi.

!include $(WorkSpace)/build/windows/makefiles/rules.make

# #########################################################################

JvmtiSrcDir = $(WorkSpace)/src/share/vm/prims

JvmtiGeneratedNames = \
        jvmdiEnter.cpp \
        jvmdiEnterTrace.cpp \
        jvmtiEnv.hpp \
        jvmtiEnter.cpp \
        jvmtiEnterTrace.cpp \
        jvmtiEnvRecommended.cpp\
        jvmti.h \

JvmtiEnvFillSource = $(JvmtiSrcDir)/jvmtiEnvFill.java
JvmtiEnvFillClass = $(JvmtiOutDir)/jvmtiEnvFill.class

JvmtiGenSource = $(JvmtiSrcDir)/jvmtiGen.java
JvmtiGenClass = $(JvmtiOutDir)/jvmtiGen.class

#Note: JvmtiGeneratedFiles must be kept in sync with JvmtiGeneratedNames by hand.
#Should be equivalent #to "JvmtiGeneratedFiles = $(JvmtiGeneratedNames:%=$(JvmtiOutDir)/%)"
JvmtiGeneratedFiles = \
        $(JvmtiOutDir)/jvmdiEnter.cpp \
        $(JvmtiOutDir)/jvmdiEnterTrace.cpp \
        $(JvmtiOutDir)/jvmtiEnv.hpp \
        $(JvmtiOutDir)/jvmtiEnter.cpp \
        $(JvmtiOutDir)/jvmtiEnterTrace.cpp \
        $(JvmtiOutDir)/jvmtiEnvRecommended.cpp\
        $(JvmtiOutDir)/jvmti.h \

XSLT = $(RUN_JAVA) -classpath $(JvmtiOutDir) jvmtiGen

# #########################################################################

both = $(JvmtiGenClass) $(JvmtiSrcDir)/jvmti.xml $(JvmtiSrcDir)/jvmtiLib.xsl

default::
        @if not exist $(JvmtiOutDir) mkdir $(JvmtiOutDir)

$(JvmtiGenClass): $(JvmtiGenSource)
	$(COMPILE_JAVAC) -g -d $(JvmtiOutDir) $(JvmtiGenSource)

$(JvmtiEnvFillClass): $(JvmtiEnvFillSource)
	@$(COMPILE_JAVAC) -g -d $(JvmtiOutDir) $(JvmtiEnvFillSource)

$(JvmtiOutDir)/jvmtiEnter.cpp: $(both) $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiEnter.xsl -OUT $(JvmtiOutDir)/jvmtiEnter.cpp -PARAM interface jvmti

$(JvmtiOutDir)/jvmtiEnterTrace.cpp: $(both) $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiEnter.xsl -OUT $(JvmtiOutDir)/jvmtiEnterTrace.cpp -PARAM interface jvmti -PARAM trace Trace

$(JvmtiOutDir)/jvmdiEnter.cpp: $(both) $(JvmtiSrcDir)/jvmdiEnter.xsl $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmdiEnter.xsl -OUT $(JvmtiOutDir)/jvmdiEnter.cpp -PARAM interface jvmdi

$(JvmtiOutDir)/jvmdiEnterTrace.cpp: $(both) $(JvmtiSrcDir)/jvmdiEnter.xsl $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	@$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmdiEnter.xsl -OUT $(JvmtiOutDir)/jvmdiEnterTrace.cpp -PARAM interface jvmdi -PARAM trace Trace

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
