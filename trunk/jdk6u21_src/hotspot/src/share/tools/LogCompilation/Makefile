#
# Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
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
PKGLIST = \
com.sun.hotspot.tools.compiler
#END PKGLIST

FILELIST = com/sun/hotspot/tools/compiler/*.java

ifneq "x$(ALT_BOOTDIR)" "x"
  BOOTDIR := $(ALT_BOOTDIR)
endif

ifeq "x$(BOOTDIR)" "x"
  JDK_HOME := $(shell dirname $(shell which java))/..
else
  JDK_HOME := $(BOOTDIR)
endif

isUnix := $(shell test -r c:/; echo $$?)

ifeq "$(isUnix)" "1"
    CPS := :
else
    CPS := ";"
endif

SRC_DIR    = src
BUILD_DIR  = build
OUTPUT_DIR = $(BUILD_DIR)/classes

# gnumake 3.78.1 does not accept the *s, 
# so use the shell to expand them
ALLFILES := $(patsubst %,$(SRC_DIR)/%,$(FILELIST))
ALLFILES := $(shell /bin/ls $(ALLFILES))

JAVAC = $(JDK_HOME)/bin/javac
JAR = $(JDK_HOME)/bin/jar

# Tagging it on because there's no reason not to run it
all: logc.jar

logc.jar: filelist manifest.mf
	@mkdir -p $(OUTPUT_DIR)
	$(JAVAC) -source 1.5 -deprecation -sourcepath $(SRC_DIR) -d $(OUTPUT_DIR) @filelist
	$(JAR) cvfm logc.jar manifest.mf -C $(OUTPUT_DIR) com

.PHONY: filelist
filelist: $(ALLFILES)
	@rm -f $@
	@echo $(ALLFILES) > $@

clean::
	rm -rf filelist logc.jar
	rm -rf $(BUILD_DIR)
