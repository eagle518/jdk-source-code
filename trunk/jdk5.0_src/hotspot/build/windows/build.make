# @(#)build.make	1.20 04/05/28 13:52:37
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Note: this makefile is invoked both from build.bat and from the J2SE
# control workspace in exactly the same manner; the required
# environment variables (Variant, WorkSpace, BootStrapDir, [WindbgHome], BuildID)
# are passed in as command line arguments.

# SA components are not built by default. Specify WindbgHome to build.
# Along with VM, Serviceability Agent (SA) is built for SA/JDI binding. 
# JDI binding on SA produces two binaries: 
#  1. sa-jdi.jar       - This is build before building jvm[_g].dll
#                        Please refer to ./makefiles/sa.make
#  2. sawindbg[_g].dll - Native library for SA - This is built after jvm[_g].dll
#                        Building this native library requires WindbgHome.
#                        Please refer to ./vm/release/Makefile, ./vm/debug/Makefile

# Allow control workspace to force Itanium or AMD64 builds with LP64
ARCH_TEXT=
!ifdef LP64
!if "$(LP64)" == "1"
ARCH_TEXT=64-Bit
!if "$(ARCH)" == "amd64"
ARCH=amd64
!else
ARCH=ia64
!endif
!else
!endif
!endif

# If we haven't set an ARCH yet use i486
# create.bat and build.bat will set it, if used.
!ifndef ARCH
ARCH=i486
!endif

# Supply these from the command line or the environment
#  It doesn't make sense to default this one
Variant=
#  It doesn't make sense to default this one
WorkSpace=

variantDir = windows_$(ARCH)_$(Variant)

VARIANT_TEXT=Core
!if "$(Variant)" == "compiler1"
VARIANT_TEXT=Client
!elseif "$(Variant)" == "compiler2"
VARIANT_TEXT=Server
!endif

#########################################################################
# Parameters for VERSIONINFO resource for jvm[_g].dll.
# These can be overridden via the nmake.exe command line.
#
HS_MAJOR_VER=1
HS_MINOR_VER=5
HS_MICRO_VER=0

# Following the Web Start / Plugin model here....
# HS_UPDATE_VER = JDK_UPDATE_VERSION * 10 + EXCEPTION_VERSION
# 
# Here are some examples:
#     1.5.0 -> 1,5,0,0
#     1.5.0_10 -> 1,5,0,100
#     1.4.2 -> 1,4,2,0
#     1.4.2_02 -> 1,4,2,20
#     1.4.2_02a -> 1,4,2,21
#     1.4.2_02b -> 1,4,2,22
HS_UPDATE_VER=0

HS_COMPANY=Sun Microsystems, Inc.
HS_COPYRIGHT=Copyright \xA9 2004
HS_RELEASE=$(HS_MAJOR_VER).$(HS_MINOR_VER).$(HS_MICRO_VER)
HS_RELEASE_SHORT=$(HS_MINOR_VER).$(HS_MICRO_VER)
HS_NAME=Java(TM) 2 Platform Standard Edition $(HS_RELEASE_SHORT)
HS_FILEDESC=Java HotSpot(TM) $(ARCH_TEXT) $(VARIANT_TEXT) VM
HS_VER=$(HS_MAJOR_VER),$(HS_MINOR_VER),$(HS_MICRO_VER),$(HS_UPDATE_VER)
HS_DOTVER=$(HS_MAJOR_VER).$(HS_MINOR_VER).$(HS_MICRO_VER).$(HS_UPDATE_VER)
!if "$(BuildID)" == ""
HS_BUILD_ID=$(HS_MAJOR_VER).$(HS_MINOR_VER).$(HS_MICRO_VER)-internal
!else
HS_BUILD_ID=$(BuildID)
!endif

# End VERSIONINFO parameters
#########################################################################

defaultTarget: product

product: checks $(variantDir) $(variantDir)\local.make
	cd $(variantDir)
	nmake -f $(WorkSpace)\build\windows\makefiles\top.make ARCH=$(ARCH)

develop: checks $(variantDir) $(variantDir)\local.make
	cd $(variantDir)
	nmake -f $(WorkSpace)\build\windows\makefiles\top.make DEVELOP=1 ARCH=$(ARCH)

clean: checkVariant
	- rm -r -f $(variantDir)

$(variantDir):
	mkdir $(variantDir)

$(variantDir)\local.make: checks
	@ echo # Generated file					>  $@
	@ echo Variant=$(Variant)				>> $@
	@ echo WorkSpace=$(WorkSpace)				>> $@
	@ echo BootStrapDir=$(BootStrapDir)			>> $@
	@ if "$(WindbgHome)" NEQ "" echo WindbgHome=$(WindbgHome) >> $@
	@ echo BuildID=$(BuildID)				>> $@
        @ if "$(USERNAME)" NEQ "" echo BuildUser=$(USERNAME)	>> $@
	@ echo HS_VER=$(HS_VER)					>> $@
	@ echo HS_DOTVER=$(HS_DOTVER)				>> $@
	@ echo HS_COMPANY=$(HS_COMPANY)				>> $@
	@ echo HS_FILEDESC=$(HS_FILEDESC)			>> $@
	@ echo HS_COPYRIGHT=$(HS_COPYRIGHT)			>> $@
	@ echo HS_NAME=$(HS_NAME)				>> $@
	@ echo HS_BUILD_ID=$(HS_BUILD_ID)			>> $@

checks: checkVariant checkWorkSpace

checkVariant:
	@ if "$(Variant)"=="" echo Need to specify "Variant=[compiler2|compiler1|core]" && false
	@ if "$(Variant)" NEQ "compiler2" if "$(Variant)" NEQ "compiler1" if "$(Variant)" NEQ "core" if "$(Variant)" NEQ "docs"	\
          echo Need to specify "Variant=[compiler2|compiler1|core|docs]" && false

checkWorkSpace:
	@ if "$(WorkSpace)"=="" echo Need to specify "WorkSpace=..." && false

checkBuildID:
	@ if "$(BuildID)"=="" echo Need to specify "BuildID=..." && false
