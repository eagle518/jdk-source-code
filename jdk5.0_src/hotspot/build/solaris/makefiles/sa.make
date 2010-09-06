#
# @(#)sa.make	1.18 04/04/28 14:30:10
#
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

# This makefile (sa.make) is included from the sa.make in the
# build directories.

# This makefile is used to build Serviceability Agent java code
# and generate JNI header file for native methods.

include $(GAMMADIR)/build/solaris/makefiles/rules.make
AGENT_DIR = $(GAMMADIR)/agent
include $(GAMMADIR)/build/sa.files
GENERATED = ../generated

# tools.jar is needed by the JDI - SA binding
SA_CLASSPATH = $(BOOT_JAVA_HOME)/lib/tools.jar

# gnumake 3.78.1 does not accept the *s that
# are in AGENT_ALLFILES, so use the shell to expand them
AGENT_ALLFILES := $(shell /usr/bin/test -d $(AGENT_DIR) && /bin/ls $(AGENT_ALLFILES))

# if $(AGENT_DIR) does not exist, we don't build SA.
all: 
	$(QUIETLY) if [ -d $(AGENT_DIR) ] ; then \
 	   $(MAKE) -f sa.make $(GENERATED)/sa-jdi.jar; \
 	fi

$(GENERATED)/sa-jdi.jar: $(AGENT_ALLFILES)
	$(QUIETLY) if [ "$(BOOT_JAVA_HOME)" = "" ]; then \
	  echo "ALT_BOOTDIR, BOOTDIR or JAVA_HOME needs to be defined to build SA"; \
	  exit 1; \
	fi
	$(QUIETLY) echo "Making $@";
	$(QUIETLY) if [ ! -d $(GENERATED)/saclasses ] ; then \
	  mkdir -p $(GENERATED)/saclasses;        \
	fi
	$(QUIETLY) $(COMPILE.JAVAC) -source 1.4 -classpath $(SA_CLASSPATH) -g -d $(GENERATED)/saclasses $(AGENT_ALLFILES)
	$(QUIETLY) $(COMPILE.RMIC)  -classpath $(GENERATED)/saclasses -d $(GENERATED)/saclasses sun.jvm.hotspot.debugger.remote.RemoteDebuggerServer
	$(QUIETLY) $(RUN.JAR) cvf $@ -C $(GENERATED)/saclasses/ .
	$(QUIETLY) $(RUN.JAR) uvf $@ -C $(AGENT_SRC_DIR) META-INF/services/com.sun.jdi.connect.Connector
	$(QUIETLY) $(RUN.JAVAH) -classpath $(GENERATED)/saclasses -d $(GENERATED) -jni sun.jvm.hotspot.debugger.proc.ProcDebuggerLocal

clean:
	rm -rf $(GENERATED)/saclasses
	rm -rf $(GENERATED)/sa-jdi.jar
