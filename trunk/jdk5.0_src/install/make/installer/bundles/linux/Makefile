#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
# @(#)Makefile	1.25 04/06/11
#
#
# Most frequently used targets:
#
#    all            -- builds
#    clobber        -- clobber the builds
#

INSTALL_BUILDDIR=../../..

include $(INSTALL_BUILDDIR)/common/Defs.gmk
include $(INSTALL_BUILDDIR)/common/Pack.gmk

all: sanity-bundles post-sanity-bundles bundle-jre j2re-rpm bundle-jdk j2sdk-rpm bundle-jdk-debug

clean::  bundle-jdk-debug-clobber bundle-jdk-clobber j2sdk-rpm-clobber bundle-jre-clobber j2re-rpm-clobber

j2re-rpm:
	@#
	@# Generate self extracting rpm file ...
	@#
	$(RM) $(TEMP_DIR)/jre-rpm.spec $(RPM_MACROFILE) 
	$(RM) -r $(TEMP_DIR)/BUILD $(TEMP_DIR)/RPMS
	$(MKDIR) -p $(TEMP_DIR)/BUILD $(TEMP_DIR)/RPMS/$(ARCH)
	@#
	@# rpm removes the input tar file built by the 'binaries' target, which 
	@# we want to keep, so we need to make a copy that rpm can devour...
	@#
	$(CD) $(OUTPUTDIR) ; $(TAR) -cf $(JRE_RPM_TARFILE) $(JRE_TEMPORARY_DIR) 
	@#
	@#subtle: override the sourcedir so that we don't have to copy
	@#        large binaries around.
	$(ECHO) "%_sourcedir	$(ABS_OUTPUTDIR)" >> $(RPM_MACROFILE)
	$(ECHO) "%_builddir	$(TEMP_DIR)/BUILD" >> $(RPM_MACROFILE)
	$(ECHO) "%_rpmdir	$(ABS_OUTPUTDIR)" >> $(RPM_MACROFILE)
	@#
	@# generate the jre-rpm.spec file
	@#
	$(ECHO) " "  >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define jre_prefix	/usr/java" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define jdk_prefix	/usr/java" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define jre_manpath	/usr/local/man/man1" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define jre_tarfile	$(JRE_RPM_TARFILE)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define arch	$(NATIVE_ARCH)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define jre_name	$(JRE_NAME)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define jdk_name	$(JDK_NAME)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define jre_version	$(JDK_VERSION)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define packed_jars	\"$(JRE_PACKED_JARS)\"" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define javaws_bin     $(JRE_JAVAWS)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%define mime_type      application/x-java-jnlp-file" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) " " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Summary: Java(TM) 2 Platform Standard Edition Runtime Environment" >> \
		$(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Name:	%{jre_name}" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Version:	%{jre_version}" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Release:	$(MILESTONE)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Copyright:	Sun Microsystems Binary Code License (BCL)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Group:	Development/Tools" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Provides:	j2re = %{jre_version}, jaxp_parser_impl, xml-commons-apis" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Requires:	glibc >= 2.1.2-11, sh-utils >= 2.0-1, fileutils >= 4.0-8, gawk >= 3.0.4-1, textutils >= 2.0-2" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "AutoReqProv:	no" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Url:	http://java.sun.com/" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Vendor:	Sun Microsystems, Inc." >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Source:	$(JRE_RPM_TARFILE)" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Packager:	Java Software <jre-comments@java.sun.com>" >> \
		$(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Buildroot:	$(TEMP_DIR)/rpm-root" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "Prefix: %{jre_prefix}" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) " " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) "%description" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "The Java 2 Platform Standard " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "Edition Runtime Environment " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "(JRE) contains\neverything " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "necessary to run applets and " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "applications designed for the " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "Java\nplatform. This includes " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "the Java virtual machine, plus " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "the Java platform\nclasses " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "and supporting files.\n\n" >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "The JRE is freely " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "redistributable, per the terms " >> $(TEMP_DIR)/jre-rpm.spec
	$(ECHO) -en "of the included license.\n\n" >> $(TEMP_DIR)/jre-rpm.spec
	@#
	@# add the rest of the template to the spec file...
	@#
	$(CAT) rpm-jre.spec >> $(TEMP_DIR)/jre-rpm.spec
	@#
	@# rpm wants to put the .rpm file the _rpmdir/{arch} directory.
	@#
	-$(MKDIR) $(OUTPUTDIR)/$(ARCH)
	@# on RH8 rpm builds as user 'rpm'
	$(CHMOD) 777  $(OUTPUTDIR)/$(ARCH)
	@# 
	@# generate the rpm file...
	@#
	$(RPM) -bb --rmsource --target $(NATIVE_ARCH)-Sun-Linux $(TEMP_DIR)/jre-rpm.spec
	@#
	@# rename the generated .rpm file to the name we want 
	@#
	-$(MV) $(OUTPUTDIR)/$(NATIVE_ARCH)/$(TEMP_JRE_RPMFILE_NAME) \
		$(OUTPUTDIR)/$(NATIVE_ARCH)/$(JRE_SHORT_BUNDLE_NAME).rpm
	@#
	@# zip it up for inclusion in self-extracting zip...
	@#
	$(RM) $(OUTPUTDIR)/$(NATIVE_ARCH)/temp.rpm $(TEMP_DIR)/temp.bin
	$(CD) $(OUTPUTDIR)/$(NATIVE_ARCH) ; \
		$(ZIPEXE) -qryn nothing temp.rpm $(JRE_SHORT_BUNDLE_NAME).rpm
	@# 
	@# combine the self-extractor and .rpm.zip into a single binary...
	@#
	$(CAT) $(UNZIPSFX) $(OUTPUTDIR)/$(NATIVE_ARCH)/temp.rpm > $(TEMP_DIR)/temp.bin
	$(SFX2SH) $(TEMP_DIR)/temp.bin \
	          $(JRE_IMAGE_DIR)/LICENSE \
		  $(OUTPUTDIR)/$(NATIVE_ARCH)/$(JRE_SHORT_BUNDLE_NAME)-rpm.sh \
		  $(JRE_NODOTVERSION) \
		  $(OUTPUTDIR)/$(JRE_TEMPORARY_DIR) \
		  "$(JRE_PACKED_JARS)" \
		  $(PLATFORM) \
		  " " \
		  "RPM"
	$(MV) $(OUTPUTDIR)/$(NATIVE_ARCH)/$(JRE_SHORT_BUNDLE_NAME).rpm $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).rpm
	$(MV) $(OUTPUTDIR)/$(NATIVE_ARCH)/$(JRE_SHORT_BUNDLE_NAME)-rpm.sh $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME)-rpm.sh
	@# 
	@# remove the intermediate files...
	@#
	$(RM) $(TEMP_DIR)/temp.bin
	$(RM) -r $(OUTPUTDIR)/$(NATIVE_ARCH)
	$(RM) -r $(OUTPUTDIR)/$(JRE_TEMPORARY_DIR)
	$(RM) $(OUTPUTDIR)/$(JRE_BUNDLE_NAME).bin $(OUTPUTDIR)/$(JRE_BUNDLE_NAME).zip

j2sdk-rpm:
	@# 
	@# Generate the self-extracting rpm file...
	@#
	$(RM) $(TEMP_DIR)/jdk-rpm.spec $(RPM_MACROFILE) 
	$(RM) -r $(TEMP_DIR)/BUILD $(TEMP_DIR)/RPMS
	$(MKDIR) -p $(TEMP_DIR)/BUILD $(TEMP_DIR)/RPMS/$(ARCH)
	@#
	@# rpm removes the input tar file built by the 'binaries' target, which 
	@# we want to keep, so we need to make a copy that rpm can devour...
	@#
	$(CD) $(OUTPUTDIR) ; $(TAR) -cf $(JDK_RPM_TARFILE) $(JDK_TEMPORARY_DIR) 
	@#
	@#subtle: override the sourcedir so that we don't have to copy
	@#        large binaries around.
	@#
	$(ECHO) "%_sourcedir	$(ABS_OUTPUTDIR)" >> $(RPM_MACROFILE)
	$(ECHO) "%_builddir	$(TEMP_DIR)/BUILD" >> $(RPM_MACROFILE)
	$(ECHO) "%_rpmdir	$(ABS_OUTPUTDIR)" >> $(RPM_MACROFILE)
	@#
	@# generate the jdk-rpm.spec file
	@#
	$(ECHO) " "  >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define jdk_prefix	/usr/java" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define jre_prefix	/usr/java" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define jdk_manpath	/usr/local/man/man1" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define jdk_tarfile	$(JDK_BUNDLE_NAME).tar" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define arch	$(NATIVE_ARCH)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define jdk_name	$(JDK_NAME)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define jre_name	$(JRE_NAME)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define jdk_version	$(JDK_VERSION)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define jre_version	$(JDK_VERSION)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define packed_jars	\"$(JDK_PACKED_JARS)\"" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define javaws_bin     $(JDK_JAVAWS)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%define mime_type      application/x-java-jnlp-file" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) " " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Summary: Java(TM) 2 Platform Standard Edition Development Kit" >> \
		$(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Name:	%{jdk_name}" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Version:	%{jdk_version}" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Release:	$(MILESTONE)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Copyright:	Sun Microsystems Binary Code License (BCL)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Group:	Development/Tools" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Provides:	%{jre_name} = %{jre_version}, j2sdk = %{jdk_version}, j2re = %{jre_version}, jaxp_parser_impl, xml-commons-apis" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Requires:	glibc >= 2.1.2-11, sh-utils >= 2.0-1, fileutils >= 4.0-8, gawk >= 3.0.4-1, textutils >= 2.0-2" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "AutoReqProv:	no" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Epoch:         2000" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Url:	http://java.sun.com/" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Vendor:	Sun Microsystems, Inc." >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Source:	$(JDK_RPM_TARFILE)" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Packager:	Java Software <j2se-comments@java.sun.com>" >> \
		$(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Buildroot:	$(TEMP_DIR)/rpm-root" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "Prefix: %{jdk_prefix}" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) " " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) "%description" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "The Java 2 Platform Standard " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "Edition Development Kit (JDK) " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "includes both the\nruntime " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "environment (Java virtual " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "machine, the Java platform " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "classes and\nsupporting " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "files) and development tools " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "(compilers, debuggers, tool " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "libraries\nand other " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "tools).\n\n" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "The JDK is a development " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "environment for building " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "applications, applets and\n" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "components that can be " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "deployed with the Java 2 " >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "Platform Standard Edition\n" >> $(TEMP_DIR)/jdk-rpm.spec
	$(ECHO) -en "Runtime Environment.\n\n" >> $(TEMP_DIR)/jdk-rpm.spec

	@#
	@# add the rest of the template to the spec file...
	@#
	$(CAT) rpm-jdk.spec >> $(TEMP_DIR)/jdk-rpm.spec
	@#
	@# rpm wants to put the .rpm file the _rpmdir/{arch} directory.
	@#
	-$(MKDIR) $(OUTPUTDIR)/$(ARCH)
	@# on RH8 rpm builds as user 'rpm'
	$(CHMOD) 777  $(OUTPUTDIR)/$(ARCH)
	@# 
	@# generate the rpm file...
	@#
	$(RPM) -bb --rmsource --target $(NATIVE_ARCH)-Sun-Linux $(TEMP_DIR)/jdk-rpm.spec
	@#
	@# rename the generated .rpm file to the name we want
	@#
	@#
	-$(MV) $(OUTPUTDIR)/$(NATIVE_ARCH)/$(TEMP_JDK_RPMFILE_NAME) \
		$(OUTPUTDIR)/$(NATIVE_ARCH)/$(JDK_SHORT_BUNDLE_NAME).rpm
	@#
	@# zip it up for inclusion in self-extracting zip...
	@#
	$(RM) $(OUTPUTDIR)/$(NATIVE_ARCH)/temp.rpm $(TEMP_DIR)/temp.bin
	$(CD) $(OUTPUTDIR)/$(NATIVE_ARCH) ; \
		$(ZIPEXE) -qryn nothing temp.rpm $(JDK_SHORT_BUNDLE_NAME).rpm
	@#
	@# combine the self-extractor and .rpm.zip into a single binary...
	@#
	$(CAT) $(UNZIPSFX) $(OUTPUTDIR)/$(NATIVE_ARCH)/temp.rpm > $(TEMP_DIR)/temp.bin
	$(SFX2SH) $(TEMP_DIR)/temp.bin \
	          $(JDK_IMAGE_DIR)/LICENSE \
		  $(OUTPUTDIR)/$(NATIVE_ARCH)/$(JDK_SHORT_BUNDLE_NAME)-rpm.sh \
		  $(JRE_NODOTVERSION) \
		  $(OUTPUTDIR)/$(JDK_TEMPORARY_DIR) \
		  "$(JRE_PACKED_JARS)" \
		  $(PLATFORM) \
		  " " \
		  "RPM"
	$(MV) $(OUTPUTDIR)/$(NATIVE_ARCH)/$(JDK_SHORT_BUNDLE_NAME).rpm $(BIN_BUNDLEDIR)/$(JDK_BUNDLE_NAME).rpm
	$(MV) $(OUTPUTDIR)/$(NATIVE_ARCH)/$(JDK_SHORT_BUNDLE_NAME)-rpm.sh $(BIN_BUNDLEDIR)/$(JDK_BUNDLE_NAME)-rpm.sh
	@#
	@# remove the intermediate files...
	@#
	$(RM) $(TEMP_DIR)/temp.bin
	$(RM) -r $(OUTPUTDIR)/$(NATIVE_ARCH)
	$(RM) -r $(OUTPUTDIR)/$(JDK_TEMPORARY_DIR)
	$(RM) $(OUTPUTDIR)/$(JDK_BUNDLE_NAME).bin $(OUTPUTDIR)/$(JDK_BUNDLE_NAME).zip

j2re-rpm-clobber:
	$(RM) $(TEMP_DIR)/jre-rpm.spec $(RPM_MACROFILE) 
	$(RM) -r $(TEMP_DIR)/BUILD $(TEMP_DIR)/RPMS
	$(RM) $(OUTPUTDIR)/$(JRE_RPM_TARFILE) 
	$(RM) $(OUTPUTDIR)/$(ARCH)/$(JRE_SHORT_BUNDLE_NAME).rpm
	$(RM) $(OUTPUTDIR)/$(ARCH)/temp.rpm $(TEMP_DIR)/temp.bin
	$(RM) $(OUTPUTDIR)/$(ARCH)/$(JRE_SHORT_BUNDLE_NAME)-rpm.sh
	$(RM) $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME).rpm
	$(RM) $(BIN_BUNDLEDIR)/$(JRE_BUNDLE_NAME)-rpm.sh
	$(RM) -r $(OUTPUTDIR)/$(ARCH)

j2sdk-rpm-clobber:
	$(RM) $(TEMP_DIR)/jdk-rpm.spec $(RPM_MACROFILE) 
	$(RM) -r $(TEMP_DIR)/BUILD $(TEMP_DIR)/RPMS
	$(RM) $(OUTPUTDIR)/$(JDK_RPM_TARFILE) 
	$(RM) $(OUTPUTDIR)/$(ARCH)/$(JDK_SHORT_BUNDLE_NAME).rpm
	$(RM) $(OUTPUTDIR)/$(ARCH)/$(JDK_SHORT_BUNDLE_NAME)-rpm.sh
	$(RM) $(BIN_BUNDLEDIR)/$(JDK_BUNDLE_NAME).rpm
	$(RM) $(BIN_BUNDLEDIR)/$(JDK_BUNDLE_NAME)-rpm.sh
	$(RM) $(OUTPUTDIR)/$(ARCH)/temp.rpm $(TEMP_DIR)/temp.bin
	$(RM) -r $(OUTPUTDIR)/$(ARCH)

$(BIN_BUNDLEDIR):
	$(MKDIR) -p $@

RPM_MACROFILE = $(HOME)/.rpmmacros

include $(INSTALL_BUILDDIR)/common/Sanity.gmk
include $(INSTALL_BUILDDIR)/common/Bundles.gmk

.PHONY: all clean bundle-jre j2re-rpm bundle-jdk j2sdk-rpm bundle-jdk-debug \
	bundle-jdk-debug-clobber bundle-jdk-clobber j2sdk-rpm-clobber \
	bundle-jre-clobber j2re-rpm-clobber
