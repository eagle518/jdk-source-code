#
# @(#)Makefile	1.13 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building RMI stubs of JMX Remote
#

BUILDDIR = ../../..
PACKAGE = javax.management
PRODUCT = sun
include $(BUILDDIR)/common/Defs.gmk

#
# Files to compile
#

# NO java files to compile: already compiled in javax/management/Makefile

# Note : some targets are double colon rules and some single colon rules
# within common included gmk files : that is why the following for loop
# has been duplicated.
SUBDIRS = snmp

all build:
	$(SUBDIRS-loop)

clean clobber::
	$(SUBDIRS-loop)

#
# Rules
#
include $(BUILDDIR)/common/Classes.gmk

build: stubs

#
# Clean up internal-use-only package.
#
clean ::
	$(RM) -r $(CLASSDESTDIR)/com/sun/jmx \
                 $(CLASSDESTDIR)/com/sun/management/jmx \
                 $(CLASSDESTDIR)/org/omg/stub/javax/management/remote/rmi \


#
# The remainder of this makefile contains defs and rules
# for generating RMI stubs.
# With inspiration from com/sun/jndi/rmi/registry
#

#
# Full class names of implementations requiring stubs
#
REMOTE_impls = \
    javax.management.remote.rmi.RMIConnectionImpl \
    javax.management.remote.rmi.RMIServerImpl \


REMOTE_files = $(subst .,/,$(REMOTE_impls))
FILES_stubs  = $(REMOTE_files:%=$(CLASSDESTDIR)/%_Stub.class)
FILES_ties   = $(subst javax/management/remote/rmi/,javax/management/remote/rmi/_,$(REMOTE_files:%=$(CLASSDESTDIR)/%_Tie.class))


#
# Compile stubs and skeletons for remote implementations
#
# Keep generated RMI/JRMP Stub source files and copy them to GENSRCDIR
# so that javadoc can include them in the API (4997471)
#
# For RMI/IIOP call rmic a second time with -standardPackage option
# so that *_tie classes are generated in package without the prefix
# org.omg.stub (6375696)
# 
$(CLASSDESTDIR)/%_Stub.class: $(CLASSDESTDIR)/%.class
	$(prep-target)
	$(RMIC) -classpath "$(CLASSDESTDIR)"    \
                -d $(CLASSDESTDIR)              \
                -v1.2                           \
                -keepgenerated                  \
                $(subst /,.,$(<:$(CLASSDESTDIR)/%.class=%))
	$(MKDIR) -p $(@D:$(CLASSDESTDIR)/%=$(GENSRCDIR)/%)
	$(CP)       $(@:%.class=%.java)                                     \
                    $(@D:$(CLASSDESTDIR)/%=$(GENSRCDIR)/%)
	$(RMIC) -classpath "$(CLASSDESTDIR)"    \
                -d $(CLASSDESTDIR)              \
                -iiop -v1.2                     \
                $(subst /,.,$(<:$(CLASSDESTDIR)/%.class=%))
	$(RMIC) $(HOTSPOT_INTERPRETER_FLAG) -classpath "$(CLASSDESTDIR)"    \
                -d $(CLASSDESTDIR)              \
                -iiop -v1.2                     \
                -standardPackage                \
                $(subst /,.,$(<:$(CLASSDESTDIR)/%.class=%))
	@$(java-vm-cleanup)

stubs: $(FILES_stubs) 
