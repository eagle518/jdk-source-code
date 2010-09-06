#
# @(#)Makefile	1.24 03/10/27
#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

DEPLOY_BUILDDIR = ..
JAVAWS_TOPDIR = ../..
DEPLOY_TOPDIR = ../..
include ../common/Defs.gmk

# debug
include ../common/JPDA.gmk

# File groupings
include FILES_java.gmk
include FILES_c.gmk
include FILES_resource.gmk

# Main Rules
ifeq ($(ARCH_DATA_MODEL),32)

dummy := $(shell $(MKDIR) -p $(OUTPUTDIR) $(TMPDIR))

ifndef EXTERNALSANITYCONTROL
OPTIMIZED += sanity-comprehensive
DEBUG += sanity-comprehensive
endif

OPTIMIZED += jump
DEBUG += jump

all:: deploy-jar optimized

clean::
	-$(RM) -r $(GENSRCBASE) $(CLASSDIR) $(CLASSDIR_DBG) $(I18NDESTDIR) \
		$(SERVLET_DESTDIR) $(OBJDIR) $(OBJDIR_DBG) $(JARDIFF_DESTDIR) \
		$(DEPLOYCLASSDESTDIR) $(DEPLOY_COMMON_JAR) $(DEPLOYOBJDIR) $(DEPLOYOBJDIR_DBG) \
		$(DEPLOYDBGCLASSDESTDIR) $(DEPLOY_COMMON_DBG_JAR) $(DEPLOY_COMMON_LIB) $(DEPLOY_COMMON_LIB_DBG)
	-$(RM) -r $(JAVAWSDEMODIR) $(PROTO_DIR) $(PROTO_DIR_DBG) $(IMAGE_DIR) \
		$(BUNDLE_DIR)/jnlp*.*
#Since deploy.jar/libdeploy.so is not built against J2SE rules, we need to remove it manually
	-$(RM) -r $(DEPLOY_OUTPUT_JAR) $(DEPLOY_OUTPUT_DBG_JAR) $(DEPLOY_OUTPUT_LIB) $(DEPLOY_OUTPUT_LIB_DBG)

else # ARCH_DATA_MODEL

all::
clean::

endif # ARCH_DATA_MODEL

include ../common/Sanity.gmk
include ../common/Rules.gmk
include ../common/Release.gmk

# for deploy.jar
include ../common/FILES_c.gmk
include ../common/FILES_java.gmk
include ../common/DeployRules.gmk

ifeq ($(PLATFORM), windows)
   include ../common/DeployRules-windows.gmk
   DEPLOY_COMMON_LIB = $(DEPLOY_COMMON_DLL)
   DEPLOY_COMMON_LIB_DBG = $(DEPLOY_COMMON_DLL_DBG)	
else
   include ../common/DeployRules-solaris.gmk
   DEPLOY_COMMON_LIB = $(DEPLOY_COMMON_SO)
   DEPLOY_COMMON_LIB_DBG = $(DEPLOY_COMMON_SO_DBG)
endif

# for zh_HK RFE 4463546 
include ../common/Resources.gmk

.PHONY: all clean
