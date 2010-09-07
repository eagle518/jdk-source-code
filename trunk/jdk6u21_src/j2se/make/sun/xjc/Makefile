#######################################################################
#
# Makefile for building xjc
#

BUILDDIR = ../..
PACKAGE = com.sun.tools.internal.xjc
PRODUCT = sun
PROGRAM = xjc
include $(BUILDDIR)/common/Defs.gmk

#
# This program must contain a manifest that defines the execution level
# needed to follow standard Vista User Access Control Guidelines
# This must be set before Program.gmk is included
#
BUILD_MANIFEST=true

#
# Files. (If java sources need to be compiled)
#
include FILES.gmk
AUTO_FILES_JAVA_DIRS = com/sun/tools/internal/xjc com/sun/tools/internal/xjc/reader/xmlschema

FILES_xsd = $(XSD_FILES:%.xsd=$(CLASSBINDIR)/%.xsd)
FILES_misc = $(MISC_FILES:%.java=$(CLASSBINDIR)/%.java)

##
# Rules.
#

JAVA_ARGS = "{ \"-J-ms8m\", \"com.sun.tools.internal.xjc.Driver\" }"

include $(BUILDDIR)/common/Program.gmk


#
# Resources 
#

LOCALE_SET_DEFINITION = j2sdk
NEW_RESOURCE_BUNDLES_PROPERTIES = $(PROPERTY_FILES)
RESOURCE_SRC_DIR = $(SHARE_SRC)/classes
RESOURCE_DEST_DIR = $(CLASSBINDIR)
include $(BUILDDIR)/common/Resources.gmk


build: $(FILES_xsd) $(FILES_misc)

$(CLASSBINDIR)/$(PKGDIR)/reader/xmlschema/bindinfo/%.xsd: $(SHARE_SRC)/classes/$(PKGDIR)/reader/xmlschema/bindinfo/%.xsd
	$(install-file)

$(CLASSBINDIR)/$(PKGDIR)/runtime/%.java: $(SHARE_SRC)/classes/$(PKGDIR)/runtime/%.java
	$(install-file)


#######################################################################


