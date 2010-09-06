#
# @(#)jvmti.make	1.10 03/12/23 16:35:19
#
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

# This makefile (jvmti.make) is included from the jvmti.make in the
# build directories.
#
# It knows how to build and run the tools to generate jvmti/jvmdi.

include $(GAMMADIR)/build/linux/makefiles/rules.make

# #########################################################################

JvmtiOutDir  = ../generated/jvmtifiles

JvmtiSrcDir = $(GAMMADIR)/src/share/vm/prims
Src_Dirs += $(JvmtiSrcDir)

# set VPATH so make knows where to look for source files
Src_Dirs_V = ${Src_Dirs}
VPATH    += $(Src_Dirs_V:%=%:)

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

JvmtiGeneratedFiles = $(JvmtiGeneratedNames:%=$(JvmtiOutDir)/%)

XSLT = $(QUIETLY) $(RUN.JAVA) -classpath $(JvmtiOutDir) jvmtiGen

.PHONY: all jvmtidocs clean cleanall

# #########################################################################

all: $(JvmtiGeneratedFiles)

both = $(JvmtiGenClass) $(JvmtiSrcDir)/jvmti.xml $(JvmtiSrcDir)/jvmtiLib.xsl

$(JvmtiGenClass): $(JvmtiGenSource)
	$(QUIETLY) $(COMPILE.JAVAC) -g -d $(JvmtiOutDir) $(JvmtiGenSource)

$(JvmtiEnvFillClass): $(JvmtiEnvFillSource)
	$(QUIETLY) $(COMPILE.JAVAC) -g -d $(JvmtiOutDir) $(JvmtiEnvFillSource)

$(JvmtiOutDir)/jvmtiEnter.cpp: $(both) $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiEnter.xsl -OUT $(JvmtiOutDir)/jvmtiEnter.cpp -PARAM interface jvmti

$(JvmtiOutDir)/jvmtiEnterTrace.cpp: $(both) $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiEnter.xsl -OUT $(JvmtiOutDir)/jvmtiEnterTrace.cpp -PARAM interface jvmti -PARAM trace Trace

$(JvmtiOutDir)/jvmdiEnter.cpp: $(both) $(JvmtiSrcDir)/jvmdiEnter.xsl $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmdiEnter.xsl -OUT $(JvmtiOutDir)/jvmdiEnter.cpp -PARAM interface jvmdi

$(JvmtiOutDir)/jvmdiEnterTrace.cpp: $(both) $(JvmtiSrcDir)/jvmdiEnter.xsl $(JvmtiSrcDir)/jvmtiEnter.xsl
	@echo Generating $@
	$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmdiEnter.xsl -OUT $(JvmtiOutDir)/jvmdiEnterTrace.cpp -PARAM interface jvmdi -PARAM trace Trace

$(JvmtiOutDir)/jvmtiEnvRecommended.cpp: $(both) $(JvmtiSrcDir)/jvmtiEnv.xsl $(JvmtiSrcDir)/jvmtiEnv.cpp $(JvmtiEnvFillClass)
	@echo Generating $@
	$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiEnv.xsl -OUT $(JvmtiOutDir)/jvmtiEnvStub.cpp
	$(QUIETLY) $(RUN.JAVA) -classpath $(JvmtiOutDir) jvmtiEnvFill $(JvmtiSrcDir)/jvmtiEnv.cpp $(JvmtiOutDir)/jvmtiEnvStub.cpp $(JvmtiOutDir)/jvmtiEnvRecommended.cpp

$(JvmtiOutDir)/jvmtiEnv.hpp: $(both) $(JvmtiSrcDir)/jvmtiHpp.xsl
	@echo Generating $@
	$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiHpp.xsl -OUT $(JvmtiOutDir)/jvmtiEnv.hpp

$(JvmtiOutDir)/jvmti.h: $(both) $(JvmtiSrcDir)/jvmtiH.xsl
	@echo Generating $@
	$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmtiH.xsl -OUT $(JvmtiOutDir)/jvmti.h

jvmtidocs:  $(JvmtiOutDir)/jvmti.html 

$(JvmtiOutDir)/jvmti.html: $(both) $(JvmtiSrcDir)/jvmti.xsl
	@echo Generating $@
	$(XSLT) -IN $(JvmtiSrcDir)/jvmti.xml -XSL $(JvmtiSrcDir)/jvmti.xsl -OUT $(JvmtiOutDir)/jvmti.html

# #########################################################################

clean :
	rm $(JvmtiGenClass) $(JvmtiEnvFillClass) $(JvmtiGeneratedFiles)

cleanall :
	rm $(JvmtiGenClass) $(JvmtiEnvFillClass) $(JvmtiGeneratedFiles)

# #########################################################################

