#
# @(#)Makefile	1.204 10/04/27
#
# Copyright (c) 2006, 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# Definitions for $(DOCSDIR), $(MKDIR), $(BINDIR), etc.
#
BUILDDIR=..
PRODUCT=docs
include $(BUILDDIR)/common/Defs.gmk

# Get CopyrightLine macro and other shared variables
include $(BUILDDIR)/common/shared/Defs-javadoc.gmk

#
# Variables used by docs target
#
DOCSTMPDIR          = $(TEMPDIR)/doctmp

#
# Different api directories in DOCSDIR
#
CORE_API_DOCSDIR=$(DOCSDIR)/api
JDK_API_DOCSDIR=$(DOCSDIR)/jdk/api
JRE_API_DOCSDIR=$(DOCSDIR)/jre/api
PLATFORM_DOCSDIR=$(DOCSDIR)/platform
# Relative paths from jdk/api or jre/api to the core api/ directory
JDKJRE2CORE_API=../../api

#
# Set heap memory max (Xmx) and initial value (Xms) depending on architecture.
# (Default initial value is 8 -- way too small for the platform APIs)
#
ifeq ($(ARCH_DATA_MODEL), 32)
  ifeq ($(ARCH), sparc)
    JAVADOC_VM_MEMORY_FLAGS = -J-Xms64M -J-Xmx512M
  else
    # Max of 256M is no longer enough.  Increase to 256 + 128
    JAVADOC_VM_MEMORY_FLAGS = -J-Xms32M -J-Xmx384M
  endif
else
  JAVADOC_VM_MEMORY_FLAGS = -J-Xms128M -J-Xmx1024M
endif

COMMON_JAVADOCFLAGS =					\
		-source 1.5				\
		-quiet					\
                -use					\
                -keywords				\
		$(JAVADOC_VM_MEMORY_FLAGS)		\
		$(ADDITIONAL_JAVADOCFLAGS)
ADDITIONAL_JAVADOCFLAGS =

JAVADOCFLAGS  =	$(COMMON_JAVADOCFLAGS)			\
		$(TAGS)					\
		-encoding ISO-8859-1			\
		-splitIndex				\
		-doctitle $(DOCTITLE_SWITCH)		\
		-windowtitle $(WINDOWTITLE_SWITCH)	\
		-header $(HEADER_SWITCH)		\
		$(TOPOPTION) 				\
		-bottom $(JAVADOCBOTTOM_SWITCH)		\
		-overview $(SHARE_SRC)/classes/overview-core.html

DRAFT = '<br><b>DRAFT&nbsp;'$(MILESTONE)-$(BUILD_NUMBER)'</b>'

# Url to root of documents
DOCSDIR_URL = {@docroot}/$(GET2DOCSDIR)

# Url to copyright html file
COPYRIGHT_URL = $(DOCSDIR_URL)/legal/cpyr.html

# Common line for how to submit a bug or rfe
BUG_SUBMIT_LINE = <a href="$(BUG_SUBMIT_URL)">Submit a bug or feature</a>

# For bottom of top page 
BOTTOM_LINE =  For further API reference and developer documentation, \
see <a href="$(DEV_DOCS_URL)">Java SE Developer Documentation</a>. \
That documentation contains more detailed, developer-targeted descriptions, \
with conceptual overviews, definitions of terms, workarounds, \
and working code examples.

# Ignored tags
IGNORED_TAGS =	beaninfo revised since.unbundled spec Note ToDo

# Java language specification cite
JLS3_CITE = <a href="$(JLS3_URL)"> \
		The Java Language Specification, Third Edition</a>
TAG_JLS3 = -tag 'jl3:a:See <cite>$(JLS3_CITE)</cite>:'

TAGS = $(IGNORED_TAGS:%=-tag %:X) $(TAG_JLS3)

ifeq ($(MILESTONE), fcs)
    DOCTITLE_SWITCH = $(JAVADOCTITLE)
    WINDOWTITLE_SWITCH = $(JAVADOCWINDOWTITLE)
    HEADER_SWITCH = $(JAVADOCHEADER)
    TOPOPTION=
    JAVADOCBOTTOM_SWITCH= $(JAVADOCBOTTOM)
else
    #
    # Early access top and bottom text (for snapshots, beta and rc)
    #
    JAVADOCTOP_EARLYACCESS = '<div style="background-color: \#EEEEEE"> \
<div style="padding: 6px; margin-top: 2px; margin-bottom: 6px; \
margin-left: 6px; margin-right: 6px; text-align: justify; \
font-size: 80%; font-family: Helvetica, Arial, sans-serif; \
font-weight: normal;"> \
Please note that the specifications and other information \
contained herein are not final and are subject to change. \
The information is being made available to you solely for purpose of \
evaluation. \
</div> </div>'
    
    JAVADOCBOTTOM_EARLYACCESS = \
'<font size="-1">\
$(BUG_SUBMIT_LINE)<p>\
$(call CopyrightLine,$(COPYRIGHT_URL),$(FIRST_COPYRIGHT_YEAR),)\
</font>'

    DOCTITLE_SWITCH = $(JAVADOCTITLE)$(DRAFT)
    WINDOWTITLE_SWITCH = $(JAVADOCWINDOWTITLE)" $(BUILD_NUMBER)"
    HEADER_SWITCH = $(JAVADOCHEADER)$(DRAFT)
    JAVADOCBOTTOM_SWITCH= $(JAVADOCBOTTOM_EARLYACCESS)
    TOPOPTION= -top $(JAVADOCTOP_EARLYACCESS)
endif

JAVADOCTITLE  = 'Java$(TRADEMARK) Platform, \
Standard Edition $(JDK_MINOR_VERSION)<br>API Specification'

JAVADOCWINDOWTITLE  = 'Java Platform SE $(JDK_MINOR_VERSION)'
JAVADOCHEADER = '<b>Java$(TRADEMARK)&nbsp;Platform<br>Standard&nbsp;Ed.&nbsp;$(JDK_MINOR_VERSION)</b>'
JAVADOCBOTTOM = \
'<font size="-1">\
$(BUG_SUBMIT_LINE)<br>$(BOTTOM_LINE)<p>\
$(call CopyrightLine,$(COPYRIGHT_URL),$(FIRST_COPYRIGHT_YEAR),)\
</font>'

#
# Variables used by domapidocs target
#

DOMAPI_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)		      \
		    -encoding ascii			      \
                    -splitIndex                               \
                    -doctitle $(DOMAPI_JAVADOCTITLE)          \
                    -windowtitle $(DOMAPI_JAVADOCWINDOWTITLE) \
                    -header $(DOMAPI_JAVADOCHEADER)           \
                    -bottom $(DOMAPI_JAVADOCBOTTOM)           \
                    -group $(DOMAPI_GROUPNAME) $(DOMAPI_REGEXP)
DOMAPI_JAVADOCTITLE  = 'Common DOM API'
DOMAPI_JAVADOCWINDOWTITLE  = 'Common DOM API'
DOMAPI_JAVADOCHEADER = '<b>Common DOM API</b>'
DOMAPI_JAVADOCBOTTOM = \
'<font size="-1">\
$(BUG_SUBMIT_LINE)<br>$(JAVA_TRADEMARK_LINE)<br>\
$(call CopyrightLine,,$(DOMAPI_FIRST_COPYRIGHT_YEAR),$(COMPANY_ADDRESS))\
</font>'
DOMAPI_GROUPNAME = "Packages"
DOMAPI_REGEXP = "com.sun.java.browser.dom:org.w3c.dom*"
DOMAPI_DESTDIR = $(JRE_API_DOCSDIR)/plugin/dom
DOMAPI2CORE_API = ../../$(JDKJRE2CORE_API)
# DOMAPI_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by mirrordocs target
#

MIRROR_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)		        \
                    -encoding ascii			        \
                    -doctitle $(MIRROR_JAVADOCTITLE)            \
                    -windowtitle $(MIRROR_JAVADOCWINDOWTITLE)   \
                    -header $(MIRROR_JAVADOCHEADER)             \
                    -bottom $(MIRROR_JAVADOCBOTTOM)             \
                    -group $(MIRROR_GROUPNAME) $(MIRROR_REGEXP) \
                    -overview $(MIRROR_OVERVIEW)
MIRROR_JAVADOCTITLE = 'Mirror API'
MIRROR_JAVADOCWINDOWTITLE = 'Mirror API'
MIRROR_JAVADOCHEADER = '<b>Mirror API</b>'
MIRROR_JAVADOCBOTTOM = \
'<font size="-1">\
$(BUG_SUBMIT_LINE)<br>\
$(call CopyrightLine,,$(MIRROR_FIRST_COPYRIGHT_YEAR),)\
</font>'
MIRROR_GROUPNAME = "Packages"
MIRROR_OVERVIEW = $(SHARE_SRC)/classes/com/sun/mirror/overview.html
MIRROR_REGEXP = "com.sun.mirror.*"
MIRROR_DESTDIR = $(JDK_API_DOCSDIR)/apt/mirror
MIRROR2CORE_API = ../../$(JDKJRE2CORE_API)
MIRROR_LINKOPT = -linkoffline $(MIRROR2CORE_API) $(CORE_API_DOCSDIR)/
# MIRROR_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by docletapidocs target
#

DOCLETAPI_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)			 \
		    -breakiterator				 \
		    -encoding ascii			         \
                    -doctitle $(DOCLETAPI_JAVADOCTITLE)          \
                    -windowtitle $(DOCLETAPI_JAVADOCWINDOWTITLE) \
                    -header $(DOCLETAPI_JAVADOCHEADER)           \
                    -bottom $(DOCLETAPI_JAVADOCBOTTOM)           \
                    -group $(DOCLETAPI_GROUPNAME) $(DOCLETAPI_REGEXP)
DOCLETAPI_JAVADOCTITLE  = 'Doclet API'
DOCLETAPI_JAVADOCWINDOWTITLE  = 'Doclet API'
DOCLETAPI_JAVADOCHEADER = '<b>Doclet API</b>'
DOCLETAPI_JAVADOCBOTTOM = \
'<font size="-1">\
$(BUG_SUBMIT_LINE)<br>$(JAVA_TRADEMARK_LINE)<br>\
$(call CopyrightLine,,$(DOCLETAPI_FIRST_COPYRIGHT_YEAR),$(COMPANY_ADDRESS))\
</font>'
DOCLETAPI_GROUPNAME = "Packages"
DOCLETAPI_REGEXP = "com.sun.javadoc"
DOCLETAPI_DESTDIR = $(JDK_API_DOCSDIR)/javadoc/doclet
DOCLETAPI2CORE_API = ../../$(JDKJRE2CORE_API)
DOCLETAPI_LINKOPT = -linkoffline $(DOCLETAPI2CORE_API) $(CORE_API_DOCSDIR)/
# DOCLETAPI_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by tagletapidocs target
#

TAGLETAPI_DESTDIR = $(JDK_API_DOCSDIR)/javadoc/taglet
TAGLETAPI2CORE_API = ../../$(JDKJRE2CORE_API)
TAGLETAPI_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)			\
		    -encoding ascii			        \
                    -nonavbar                                   \
                    -noindex                                    \
                    -bottom $(TAGLETAPI_JAVADOCBOTTOM)
TAGLETAPI_JAVADOCBOTTOM = \
'<font size="-1">\
$(BUG_SUBMIT_LINE)<br>$(JAVA_TRADEMARK_LINE)<br>\
$(call CopyrightLine,,$(TAGLETAPI_FIRST_COPYRIGHT_YEAR),$(COMPANY_ADDRESS))\
</font>'
# TAGLETAPI_FILE is located in NON_CORE_PKGS.gmk

#
# Variables used by jdi target
#

JPDA_SOURCEPATH  = $(TOPDIR)/src/share/classes
JDI_DOCDIR = $(JDK_API_DOCSDIR)/jpda/jdi
JDI2CORE_API = ../../$(JDKJRE2CORE_API)
JDI_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)			 	\
		   -encoding ascii			         	\
                   -nodeprecatedlist                             	\
                   -d $(JDI_DOCDIR)		                  	\
                   -sourcepath $(JPDA_SOURCEPATH)                	\
                   -windowtitle $(JDI_WINDOWTITLE)               	\
                   -doctitle $(JDI_DOCTITLE)                     	\
                   -header $(JDI_HEADER)                         	\
		   -bottom $(JDI_JAVADOCBOTTOM)			        \
                   -linkoffline $(JDI2CORE_API) $(CORE_API_DOCSDIR)/    \
                   -overview $(JPDA_SOURCEPATH)/jdi-overview.html
JDI_WINDOWTITLE = "Java Debug Interface"
JDI_DOCTITLE    = "Java$(TRADEMARK) Debug Interface"
JDI_HEADER      = "Java Debug Interface"
JDI_JAVADOCBOTTOM = \
'<font size="-1"><p>\
$(call CopyrightLine,,$(JDI_FIRST_COPYRIGHT_YEAR),)\
</font>'
# JDI_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by JAAS target
#
# NOTE: Quotes are required around sourcepath argument only on Windows.  Otherwise,
#       you get "No packages or classes specified." due to $(CLASSPATH_SEPARATOR)
#       being interpreted as an end of command (newline)

JAAS_SOURCEPATH = $(TOPDIR)/src/share/classes$(CLASSPATH_SEPARATOR)$(TOPDIR)/src/solaris/classes$(CLASSPATH_SEPARATOR)$(TOPDIR)/src/windows/classes$(CLASSPATH_SEPARATOR)$(TOPDIR)/src/linux/classes

JAAS_DOCDIR	 = $(JRE_API_DOCSDIR)/security/jaas/spec
JAAS2CORE_API    = ../../../$(JDKJRE2CORE_API)
JAAS_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)			 \
		   -encoding ascii			         \
                   -nodeprecatedlist                             \
                   -d $(JAAS_DOCDIR)				 \
                   -sourcepath $(JAAS_SOURCEPATH)                \
                   -windowtitle $(JAAS_WINDOWTITLE)              \
                   -doctitle $(JAAS_DOCTITLE)                    \
                   -header $(JAAS_JAVADOCHEADER)                 \
		   -bottom $(JAAS_JAVADOCBOTTOM)		 \
                   -linkoffline $(JAAS2CORE_API) $(CORE_API_DOCSDIR)/	 \
                   -overview $(TOPDIR)/src/share/classes/com/sun/security/auth/jaas-overview.html
JAAS_WINDOWTITLE = "Java Authentication and Authorization Service "
JAAS_DOCTITLE    = "Java$(TRADEMARK) Authentication and Authorization Service"
JAAS_JAVADOCHEADER      = "JAAS"
JAAS_JAVADOCBOTTOM = \
'<font size="-1"><p>\
$(call CopyrightLine,,$(JAAS_FIRST_COPYRIGHT_YEAR),)\
</font>'
# JAAS_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by JGSS target
#

JGSS_SOURCEPATH  = $(TOPDIR)/src/share/classes
JGSS_DOCDIR	 = $(JRE_API_DOCSDIR)/security/jgss/spec
JGSS2CORE_API    = ../../../$(JDKJRE2CORE_API)
JGSS_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)			\
		   -encoding ascii			        \
                   -nodeprecatedlist                            \
                   -d $(JGSS_DOCDIR)				\
                   -sourcepath $(JGSS_SOURCEPATH)               \
                   -windowtitle $(JGSS_WINDOWTITLE)             \
                   -doctitle $(JGSS_DOCTITLE)                   \
                   -header $(JGSS_JAVADOCHEADER)                \
		   -bottom $(JGSS_JAVADOCBOTTOM)		\
                   -linkoffline $(JGSS2CORE_API) $(CORE_API_DOCSDIR)/ \
                   -overview \
		   $(JGSS_SOURCEPATH)/com/sun/security/jgss/jgss-overview.html

JGSS_WINDOWTITLE = "Java GSS-API Utilities "
JGSS_DOCTITLE    = "Java$(TRADEMARK) GSS-API Utilities"
JGSS_JAVADOCHEADER      = "Java GSS-API Utilities"
JGSS_JAVADOCBOTTOM = \
'<font size="-1"><p>\
$(call CopyrightLine,,$(JGSS_FIRST_COPYRIGHT_YEAR),)\
</font>'
# JGSS_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by SMARTCARDIO target
#

SMARTCARDIO_SOURCEPATH  = $(TOPDIR)/src/share/classes
SMARTCARDIO_DOCDIR	= $(JRE_API_DOCSDIR)/security/smartcardio/spec
SMARTCARDIO2CORE_API    = ../../../$(JDKJRE2CORE_API)
SMARTCARDIO_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)		       \
		   -encoding ascii			               \
                   -nodeprecatedlist                                   \
                   -d $(SMARTCARDIO_DOCDIR)			       \
                   -sourcepath $(SMARTCARDIO_SOURCEPATH)               \
                   -windowtitle $(SMARTCARDIO_WINDOWTITLE)             \
                   -doctitle $(SMARTCARDIO_DOCTITLE)                   \
                   -header $(SMARTCARDIO_JAVADOCHEADER)                \
		   -bottom $(SMARTCARDIO_JAVADOCBOTTOM)		       \
                   -linkoffline $(SMARTCARDIO2CORE_API) $(CORE_API_DOCSDIR)/

SMARTCARDIO_WINDOWTITLE = "Java Smart Card I/O"
SMARTCARDIO_DOCTITLE    = "Java$(TRADEMARK) Smart Card I/O"
SMARTCARDIO_JAVADOCHEADER      = "Java Smart Card I/O"
SMARTCARDIO_JAVADOCBOTTOM = \
'<font size="-1"><p>\
$(call CopyrightLine,,$(SMARTCARDIO_FIRST_COPYRIGHT_YEAR),)\
</font>'
# SMARTCARDIO_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by HTTPSERVER target
#

HTTPSERVER_SOURCEPATH  = $(TOPDIR)/src/share/classes
HTTPSERVER_DOCDIR	 = $(JRE_API_DOCSDIR)/net/httpserver/spec
HTTPSERVER2CORE_API    = ../../../$(JDKJRE2CORE_API)
HTTPSERVER_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)			\
		   -encoding ascii			        	\
                   -nodeprecatedlist                            	\
                   -d $(HTTPSERVER_DOCDIR)				\
                   -sourcepath $(HTTPSERVER_SOURCEPATH)               	\
                   -windowtitle $(HTTPSERVER_WINDOWTITLE)             	\
                   -doctitle $(HTTPSERVER_DOCTITLE)                   	\
                   -header $(HTTPSERVER_JAVADOCHEADER)                	\
		   -bottom $(HTTPSERVER_JAVADOCBOTTOM)		        \
                   -linkoffline $(HTTPSERVER2CORE_API) $(CORE_API_DOCSDIR)/

HTTPSERVER_WINDOWTITLE = "Java HTTP Server"
HTTPSERVER_DOCTITLE    = "Java$(TRADEMARK) HTTP Server"
HTTPSERVER_JAVADOCHEADER      = "Java HTTP Server"
HTTPSERVER_JAVADOCBOTTOM = \
'<font size="-1"><p>\
$(call CopyrightLine,,$(HTTPSERVER_FIRST_COPYRIGHT_YEAR),)\
</font>'
# HTTPSERVER_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by jdwp target
#
JDWP_SPEC       = $(JPDA_SOURCEPATH)/com/sun/tools/jdwpgen/jdwp.spec
JDWP_DOCDIR     = $(PLATFORM_DOCSDIR)/jpda/jdwp

#
# Variables used by jvmti target
#
JVMTI_HTML        = $(HOTSPOT_DOCS_IMPORT_PATH)/platform/jvmti/jvmti.html
JVMTI_DOCDIR      = $(PLATFORM_DOCSDIR)/jvmti

#
# Variables used by mgmt target
#
MGMT_DOCDIR	   = $(JRE_API_DOCSDIR)/management/
MGMT2CORE_API      = ../../$(JDKJRE2CORE_API)
MGMT_EXT_DIR	   = $(MGMT_DOCDIR)/extension
MGMT_SOURCEPATH    = $(TOPDIR)/src/share/classes
JVM_MANAGEMENT_MIB = $(MGMT_SOURCEPATH)/sun/management/snmp/JVM-MANAGEMENT-MIB.mib
MGMT_JAVADOCFLAGS  = $(COMMON_JAVADOCFLAGS)			   \
		     -encoding ascii			           \
                     -nodeprecatedlist                             \
                     -d $(MGMT_EXT_DIR) 		  	   \
                     -sourcepath $(MGMT_SOURCEPATH)                \
                     -windowtitle $(MGMT_WINDOWTITLE)              \
                     -doctitle $(MGMT_DOCTITLE)                    \
                     -header $(MGMT_HEADER)                        \
		     -bottom $(MGMT_JAVADOCBOTTOM)		   \
		     -linkoffline $(MGMT2CORE_API) $(CORE_API_DOCSDIR)/  \
                     -overview $(MGMT_SOURCEPATH)/com/sun/management/mgmt-overview.html
MGMT_WINDOWTITLE = "Monitoring and Management Interface for the Java Platform"
MGMT_DOCTITLE    = "Monitoring and Management Interface for the Java$(TRADEMARK) Platform"
MGMT_HEADER      = "Monitoring and Management Interface for the Java Platform"
MGMT_JAVADOCBOTTOM = \
'<font size="-1"><p>\
$(call CopyrightLine,,$(MGMT_FIRST_COPYRIGHT_YEAR),)\
</font>'
# MGMT_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by attach target
#

ATTACH_SOURCEPATH  = $(TOPDIR)/src/share/classes
ATTACH_DOCDIR      = $(JDK_API_DOCSDIR)/attach/spec
ATTACH2CORE_API    = ../../$(JDKJRE2CORE_API)
ATTACH_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)                        	\
		   -encoding ascii                               	\
                   -nodeprecatedlist                             	\
		   -d $(ATTACH_DOCDIR)					\
                   -sourcepath $(ATTACH_SOURCEPATH)                	\
                   -windowtitle $(ATTACH_WINDOWTITLE)               	\
                   -doctitle $(ATTACH_DOCTITLE)                     	\
                   -header $(ATTACH_HEADER)                         	\
		   -bottom $(ATTACH_JAVADOCBOTTOM)		        \
		   -linkoffline $(ATTACH2CORE_API) $(CORE_API_DOCSDIR)/
ATTACH_WINDOWTITLE = "Attach API"
ATTACH_DOCTITLE    = "Attach API"
ATTACH_HEADER      = "Attach API"
ATTACH_JAVADOCBOTTOM = \
'<font size="-1"><p>\
$(call CopyrightLine,,$(ATTACH_FIRST_COPYRIGHT_YEAR),)\
</font>'
# ATTACH_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by jconsole target
#

JCONSOLE_SOURCEPATH  = $(TOPDIR)/src/share/classes
JCONSOLE_DOCDIR      = $(JDK_API_DOCSDIR)/jconsole/spec
JCONSOLE2CORE_API    = ../../$(JDKJRE2CORE_API)
JCONSOLE_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)                        	\
		   -encoding ascii                               	\
                   -nodeprecatedlist                             	\
		   -d $(JCONSOLE_DOCDIR)				\
                   -sourcepath $(JCONSOLE_SOURCEPATH)                	\
                   -windowtitle $(JCONSOLE_WINDOWTITLE)               	\
                   -doctitle $(JCONSOLE_DOCTITLE)                     	\
                   -header $(JCONSOLE_HEADER)                         	\
		   -bottom $(JCONSOLE_JAVADOCBOTTOM)		        \
		   -linkoffline $(JCONSOLE2CORE_API) $(CORE_API_DOCSDIR)/
JCONSOLE_WINDOWTITLE = "JConsole API"
JCONSOLE_DOCTITLE    = "JConsole API"
JCONSOLE_HEADER      = "JConsole API"
JCONSOLE_JAVADOCBOTTOM = \
'<font size="-1"><p>\
$(call CopyrightLine,,$(JCONSOLE_FIRST_COPYRIGHT_YEAR),)\
</font>'
# JCONSOLE_PKGS is located in NON_CORE_PKGS.gmk

#
# Variables used by treeapidocs target
#

TREEAPI_JAVADOCFLAGS = $(COMMON_JAVADOCFLAGS)		        	\
                    -encoding ascii			        	\
                    -doctitle $(TREEAPI_JAVADOCTITLE)            	\
                    -windowtitle $(TREEAPI_JAVADOCWINDOWTITLE)   	\
                    -header $(TREEAPI_JAVADOCHEADER)             	\
                    -bottom $(TREEAPI_JAVADOCBOTTOM)             	\
                    -group $(TREEAPI_GROUPNAME) $(TREEAPI_REGEXP) 
#
#	            -overview $(TREEAPI_OVERVIEW)
#
TREEAPI_JAVADOCTITLE = 'Compiler Tree API'
TREEAPI_JAVADOCWINDOWTITLE = 'Compiler Tree API'
TREEAPI_JAVADOCHEADER = \
'<b>Compiler Tree API</b>'
TREEAPI_JAVADOCBOTTOM = \
'<font size="-1">\
$(BUG_SUBMIT_LINE)<br>\
$(call CopyrightLine,,$(TREEAPI_FIRST_COPYRIGHT_YEAR),)\
</font>'
TREEAPI_GROUPNAME = "Packages"
TREEAPI_OVERVIEW = $(SHARE_SRC)/classes/com/sun/source/overview.html
TREEAPI_REGEXP = "com.sun.source.*"
TREEAPI_DESTDIR = $(JDK_API_DOCSDIR)/javac/tree
TREEAPI2CORE_API = ../../$(JDKJRE2CORE_API)
TREEAPI_LINKOPT = -linkoffline $(TREEAPI2CORE_API) $(CORE_API_DOCSDIR)/
# TREEAPI_PKGS is located in NON_CORE_PKGS.gmk

#
# Path where javadoc should find source files for release docs
#
RELEASEDOCS_SRCPATH = $(SHARE_SRC)/classes$(CLASSPATH_SEPARATOR)$(PLATFORM_SRC)/classes$(CLASSPATH_SEPARATOR)$(GENSRCDIR)$(CLASSPATH_SEPARATOR)$(SHARE_SRC)/doc/stub

#
# CORE_PKGS environment variable has been moved to the following file
#
include CORE_PKGS.gmk

#
# Load environment variables for API package names that are not part of
# the Java SE platform
#
include NON_CORE_PKGS.gmk

# Targets for all APIs other than the core platform APIs
ALL_OTHER_TARGETS = \
    mirrordocs \
    docletapidocs \
    tagletapidocs \
    domapidocs \
    jpdadocs \
    jaasdocs \
    jgssdocs \
    smartcardiodocs \
    httpserverdocs  \
    mgmtdocs \
    attachdocs \
    jconsoledocs \
    treeapidocs

.PHONY: all docs
all docs: coredocs otherdocs

.PHONY: otherdocs
otherdocs: $(ALL_OTHER_TARGETS)

#################################################################
# Production Targets -- USE THESE TARGETS WHEN:
#    a) You're generating docs outside of release engineering's
#       standard control build.
#    b) The docs will be pushed to the web and/or included in
#       the downloaded doc bundle.
#
# See:  Notes.html#releaseTargets
# Note: Spaces preceed ifdef/ifndef indents. Tabs preceed target commands (!)
#
.PHONY: sanitycheckcoredocs
sanitycheckcoredocs:
	@$(ECHO) ""
	@$(ECHO) "Building core api docs with these values:"
	@$(ECHO) "    BUILD_NUMBER = $(BUILD_NUMBER)"
	@$(ECHO) "    MILESTONE = $(MILESTONE)"
	@$(ECHO) ""
  ifeq ($(BUILD_NUMBER), b00)
	@$(ECHO) "ERROR: Build number must be defined"
	@$(ECHO) "MILESTONE is set to $(MILESTONE)"
	@$(ECHO) ""
	exit 1
  endif

# Maximize performance and ensure that build number & milestone are set.
.PHONY: rel-coredocs
rel-coredocs: sanitycheckcoredocs
	@# ######## release version of core packages ########
	$(MAKE) JAVADOC_VM_MEMORY_FLAGS="-J-Xms256M -J-Xmx512M" coredocs

.PHONY: rel-docs
rel-docs: rel-coredocs $(ALL_OTHER_TARGETS)
#
# end of production targets
#############################################################

.PHONY: coredocs
coredocs: GET2DOCSDIR=..
coredocs: 
	@# ######## core packages #######################
	$(RM) -r $(CORE_API_DOCSDIR)
	$(MKDIR) -p $(CORE_API_DOCSDIR)
	$(JAVADOC) $(JAVADOCFLAGS)                     \
                   -d $(CORE_API_DOCSDIR)              \
                   -sourcepath $(RELEASEDOCS_SRCPATH)  \
                   $(CORE_PKGS)

.PHONY: mirrordocs
mirrordocs: GET2DOCSDIR=$(MIRROR2CORE_API)/..
mirrordocs: 
	@# ######## mirror api for apt ##################
	$(RM) -r $(MIRROR_DESTDIR)
	$(MKDIR) -p $(MIRROR_DESTDIR)
	$(JAVADOC) $(MIRROR_JAVADOCFLAGS)             \
		   -d $(MIRROR_DESTDIR)               \
		   -sourcepath $(RELEASEDOCS_SRCPATH) \
		   $(MIRROR_LINKOPT)                  \
		   $(MIRROR_PKGS)

.PHONY: docletapidocs
docletapidocs: GET2DOCSDIR=$(DOCLETAPI2CORE_API)/..
docletapidocs: 
	@# ######## doclet api ############################
	$(RM) -r $(DOCLETAPI_DESTDIR)
	$(MKDIR) -p $(DOCLETAPI_DESTDIR)
	$(JAVADOC) $(DOCLETAPI_JAVADOCFLAGS)                 \
                   -d $(DOCLETAPI_DESTDIR)                   \
                   -sourcepath $(RELEASEDOCS_SRCPATH)        \
                   $(DOCLETAPI_LINKOPT)                      \
                   $(DOCLETAPI_PKGS)

.PHONY: tagletapidocs
tagletapidocs: GET2DOCSDIR=$(TAGLETAPI2CORE_API)/..
tagletapidocs: 
	@# ######## taglet api ############################
	$(RM) -r $(TAGLETAPI_DESTDIR)
	$(MKDIR) -p $(TAGLETAPI_DESTDIR)
	$(RM) -r $(DOCSTMPDIR)
	$(MKDIR) -p $(DOCSTMPDIR)
	$(JAVADOC) $(TAGLETAPI_JAVADOCFLAGS)                 	\
                   -d $(DOCSTMPDIR)                          	\
                   -linkoffline $(TAGLETAPI2CORE_API) $(CORE_API_DOCSDIR)/ \
                   $(SHARE_SRC)/classes/$(TAGLETAPI_FILE)
	cp -r $(DOCSTMPDIR)/com $(TAGLETAPI_DESTDIR)
	cp $(DOCSTMPDIR)/stylesheet.css $(TAGLETAPI_DESTDIR)
	$(RM) -r $(DOCSTMPDIR)

.PHONY: domapidocs
domapidocs: GET2DOCSDIR=$(DOMAPI2CORE_API)/..
domapidocs: 
	@# ######## dom api ############################
	$(RM) -r $(DOMAPI_DESTDIR)
	$(MKDIR) -p $(DOMAPI_DESTDIR)
	$(JAVADOC) $(DOMAPI_JAVADOCFLAGS)		     	\
                   -d $(DOMAPI_DESTDIR)                         \
                   -sourcepath $(RELEASEDOCS_SRCPATH)        	\
                   -linkoffline $(DOMAPI2CORE_API) $(CORE_API_DOCSDIR)/ \
                   $(DOMAPI_PKGS)

.PHONY: jpdadocs
jpdadocs: jdidocs jdwpdocs jvmtidocs

.PHONY: jdidocs
jdidocs: GET2DOCSDIR=$(JDI2CORE_API)/..
jdidocs: 
	@# ######## jdi #################################
	$(RM) -r $(JDI_DOCDIR)
	$(MKDIR) -p $(JDI_DOCDIR)
	$(JAVADOC) $(JDI_JAVADOCFLAGS)                       \
                   $(JDI_PKGS)

.PHONY: jdwpdocs
jdwpdocs:
	@# ######## jdwp #################################
	$(CD) $(TOPDIR)/make/jpda/jdwpgen && $(MAKE) all
	$(RM) -r $(JDWP_DOCDIR)
	$(MKDIR) -p $(JDWP_DOCDIR)
	$(JAVA) -classpath $(CLASSBINDIR)                   \
                com.sun.tools.jdwpgen.Main                  \
                $(JDWP_SPEC)                                \
                -doc $(JDWP_DOCDIR)/jdwp-protocol.html

.PHONY: jvmtidocs
jvmtidocs:
	@# ######## jvmti #################################
	@if [ -f $(JVMTI_HTML) ] ; then 		\
	  $(RM) -r $(JVMTI_DOCDIR); 			\
	  $(MKDIR) -p $(JVMTI_DOCDIR); 			\
	  $(ECHO) $(CP) $(JVMTI_HTML) $(JVMTI_DOCDIR); 	\
	  $(CP) $(JVMTI_HTML) $(JVMTI_DOCDIR); 		\
	else 						\
	  $(ECHO) "WARNING: Generated jvmti file does not exist: $(JVMTI_HTML)"; \
	fi

.PHONY: jaasdocs
jaasdocs: GET2DOCSDIR=$(JAAS2CORE_API)/..
jaasdocs: 
	@# ######## api-jaas ############################
	$(RM) -r $(JAAS_DOCDIR)
	$(MKDIR) -p $(JAAS_DOCDIR)
	$(JAVADOC) $(JAAS_JAVADOCFLAGS)                     \
		   $(JAAS_PKGS)

.PHONY: jgssdocs
jgssdocs: GET2DOCSDIR=$(JGSS2CORE_API)/..
jgssdocs: 
	@# ######## api-jgss ############################
	$(RM) -r $(JGSS_DOCDIR)
	$(MKDIR) -p $(JGSS_DOCDIR)
	$(JAVADOC) $(JGSS_JAVADOCFLAGS)                     \
		   $(JGSS_PKGS)

.PHONY: smartcardiodocs
smartcardiodocs: GET2DOCSDIR=$(SMARTCARDIO2CORE_API)/..
smartcardiodocs: 
	@# ######## api-smartcardio ############################
	$(RM) -r $(SMARTCARDIO_DOCDIR)
	$(MKDIR) -p $(SMARTCARDIO_DOCDIR)
	$(JAVADOC) $(SMARTCARDIO_JAVADOCFLAGS)                     \
		   $(SMARTCARDIO_PKGS)

.PHONY: httpserverdocs
httpserverdocs: GET2DOCSDIR=$(HTTPSERVER2CORE_API)/..
httpserverdocs: 
	@# ######## api-httpserver #######################
	$(RM) -r $(HTTPSERVER_DOCDIR)
	$(MKDIR) -p $(HTTPSERVER_DOCDIR)
	$(JAVADOC) $(HTTPSERVER_JAVADOCFLAGS)                     \
		   $(HTTPSERVER_PKGS)

.PHONY: mgmtdocs
mgmtdocs: GET2DOCSDIR=$(MGMT2CORE_API)/..
mgmtdocs: 
	@# ######## api-management ############################
	$(RM) -r $(MGMT_DOCDIR)
	$(MKDIR) -p $(MGMT_DOCDIR) $(MGMT_EXT_DIR)
	$(CP) $(JVM_MANAGEMENT_MIB) $(MGMT_DOCDIR)
	$(JAVADOC) $(MGMT_JAVADOCFLAGS)                     \
		   $(MGMT_PKGS)

.PHONY: attachdocs
attachdocs: GET2DOCSDIR=$(ATTACH2CORE_API)/..
attachdocs: 
	@# ######## api-attach ############################
	$(RM) -r $(ATTACH_DOCDIR)
	$(MKDIR) -p $(ATTACH_DOCDIR)
	$(JAVADOC) $(ATTACH_JAVADOCFLAGS)                     \
		   $(ATTACH_PKGS)

.PHONY: jconsoledocs
jconsoledocs: GET2DOCSDIR=$(JCONSOLE2CORE_API)/..
jconsoledocs: 
	@# ######## api-jconsole ############################
	$(RM) -r $(JCONSOLE_DOCDIR)
	$(MKDIR) -p $(JCONSOLE_DOCDIR)
	$(JAVADOC) $(JCONSOLE_JAVADOCFLAGS)                     \
		   $(JCONSOLE_PKGS)

.PHONY: treeapidocs
treeapidocs: GET2DOCSDIR=$(TREEAPI2CORE_API)/..
treeapidocs: 
	@# ######## tree api for javac ##################
	$(RM) -r $(TREEAPI_DESTDIR)
	$(MKDIR) -p $(TREEAPI_DESTDIR)
	$(JAVADOC) $(TREEAPI_JAVADOCFLAGS)             	\
		   -d $(TREEAPI_DESTDIR)               	\
		   -sourcepath $(RELEASEDOCS_SRCPATH) 	\
		   $(TREEAPI_LINKOPT)                  	\
		   $(TREEAPI_PKGS)

# DEBUG TARGET
# List the values defined in the makefile hierarchy, to make sure everything
# is set properly, and to help identify values we can use instead of making new ones.
# (Most of them come from common/shared/Defs.gmk)
#
#  Notes:
#    * BUILD_NUMBER defaults to b00 if not set on command line with BUILD_NUMBER=<value>
#    * MILESTONE defaults to internal unless set to beta, rc, or fcs on command line
#
.PHONY: echovalues
echovalues:
	@$(ECHO) ""
	@$(ECHO) --------------Shared---------------------------
	@$(ECHO) BUILD_NUMBER = $(BUILD_NUMBER)
	@$(ECHO) FULL_VERSION = $(FULL_VERSION)
	@$(ECHO) JDK_VERSION = $(JDK_VERSION)
	@$(ECHO) JDK_MAJOR_VERSION = $(JDK_MAJOR_VERSION)
	@$(ECHO) JDK_MINOR_VERSION = $(JDK_MINOR_VERSION)
	@$(ECHO) JDK_MICRO_VERSION = $(JDK_MICRO_VERSION)
	@$(ECHO) JDK_UPDATE_VERSION = $(JDK_UPDATE_VERSION)
	@$(ECHO) JDK_MKTG_VERSION = $(JDK_MKTG_VERSION)
	@$(ECHO) JDK_UNDERSCORE_VERSION = $(JDK_UNDERSCORE_VERSION)
	@$(ECHO) JDK_MKTG_UNDERSCORE_VERSION = $(JDK_MKTG_UNDERSCORE_VERSION)
	@$(ECHO) MARKETING_NUMBER = $(MARKETING_NUMBER)
	@$(ECHO) MARKET_NAME = $(MARKET_NAME)
	@$(ECHO) MILESTONE = $(MILESTONE)
	@$(ECHO) RELEASE = $(RELEASE)
	@$(ECHO) USER_RELEASE_SUFFIX = $(USER_RELEASE_SUFFIX)
	@$(ECHO) --------------Shared---------------------------
	@$(ECHO) ""
	@$(ECHO) --------------common/Defs---------------------------
	@$(ECHO) "RELEASEDOCS_SRCPATH"
	@$(ECHO) "   SHARE_SRC/classes: $(SHARE_SRC)/classes"
	@$(ECHO) "   PLATFORM_SRC/classes: $(PLATFORM_SRC)/classes"
	@$(ECHO) "   GENSRCDIR: $(GENSRCDIR)"
	@$(ECHO) "   SHARE_SRC/doc/stub: $(SHARE_SRC)/doc/stub"
	@$(ECHO) --------------common/Defs---------------------------
	@$(ECHO) ""
