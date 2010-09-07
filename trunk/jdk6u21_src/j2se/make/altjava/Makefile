#
# @(#)Makefile	1.3 10/03/23
#
# Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Makefile for building alt-rt.jar
#

BUILDDIR = ..

PRODUCT = altjava

include $(BUILDDIR)/common/Defs.gmk

JAR_FILE = $(LIBDIR)/alt-rt.jar

ALTCLASSES_SRCDIR = $(SHARE_SRC)/altclasses
ALTCLASSES_BINDIR = $(OUTPUTDIR)/altclasses

ALTCLASSES_src = \
	java/lang/StringValue.java \
	java/util/HashMap.java \
	java/util/LinkedHashMap.java \
	java/util/TreeMap.java

ALTCLASSES_java=$(ALTCLASSES_src:.java=$(ALTCLASSES_SRCDIR)/%.java)
ALTCLASSES_classes=$(ALTCLASSES_src:%.java=$(ALTCLASSES_BINDIR)/%.class)

JAVA_SOURCE_LIST=$(TEMPDIR)/.classes.list

$(ALTCLASSES_BINDIR)/%.class: $(ALTCLASSES_SRCDIR)/%.java
	@$(ECHO) "$?" >> $(JAVA_SOURCE_LIST)

$(JAVA_SOURCE_LIST) : $(ALTCLASSES_classes)
	@$(TOUCH) $@

.delete.classlist:
	@$(RM) $(JAVA_SOURCE_LIST)

.compile.classlist : $(JAVA_SOURCE_LIST)
	@$(MKDIR) -p $(ALTCLASSES_BINDIR)
	@if [ `$(CAT) $(JAVA_SOURCE_LIST) | $(WC) -l` -ge 1 ] ; then \
	  $(ECHO) "# Java sources to be compiled: (listed in file $(JAVA_SOURCE_LIST))"; \
	  $(CAT) $(JAVA_SOURCE_LIST); \
	  $(ECHO) "# Running javac:"; \
	  $(ECHO) $(JAVAC) -d $(ALTCLASSES_BINDIR) @$(JAVA_SOURCE_LIST); \
	  $(JAVAC) -d $(ALTCLASSES_BINDIR) @$(JAVA_SOURCE_LIST); \
	fi

.build.jarfile:
	@if [ `$(CAT) $(JAVA_SOURCE_LIST) | $(WC) -l` -ge 1 ] ; then \
	  $(ECHO) "# Creating or updating $(JAR_FILE)"; \
	  $(JAR) cvf $(JAR_FILE) -C $(ALTCLASSES_BINDIR) . $(JAR_JFLAGS); \
	fi

# Compile all updated sources in batch
classes : .delete.classlist .compile.classlist .build.jarfile

all build: classes

clean clobber: .delete.classlist
	$(RM) -r $(ALTCLASSES_BINDIR) $(JAR_FILE)

.PHONY: all build clean clobber \
	.delete.classlist .compile.classlist .build.jarfile classes

