#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)Makefile	1.45 10/05/14
#
# Makefile to build the Online installer
#



TEMPDIR=$(TEMP_DIR)/jinstall-jre
TEMPPATCHDIR=$(TEMP_DIR)/ishield/patch



TARGET_NAME=jchksum
TARGET=$(TARGET_NAME).exe

include ./common.gmk

TARGET_CHK=jchksum.exe


all: clean updatefiles create_projectfiles jre

updatefiles: $(OBJDIR) patches_cksums
	$(CP) $(INSTALL_MSSDK)/include/Bits.h $(OBJDIR)/Bits.h
	$(CP) $(INSTALL_MSSDK)/include/Bitsmsg.h $(OBJDIR)/Bitsmsg.h
	$(CP) $(INSTALL_MSSDK)/lib/bits.lib $(OBJDIR)/bits.lib
	if [ -f $(INSTALL_MSSDK)/include/Bits1_5.h ] ; \
	then \
		$(CP) $(INSTALL_MSSDK)/include/Bits1_5.h $(OBJDIR)/Bits1_5.h ; \
	fi
	if [ -f $(INSTALL_MSSDK)/include/Bits2_0.h ] ; \
	then \
		$(CP) $(INSTALL_MSSDK)/include/Bits2_0.h $(OBJDIR)/Bits2_0.h ; \
	fi
	$(CP) $(SRCDIR)jchksum.h $(OBJDIR)/InstallerFiles.h

create_projectfiles: $(OBJDIR)
	$(RM) -r $(TEMP_DIR)/jinstallproj
	$(MKDIR) $(TEMP_DIR)/jinstallproj
	$(CP) JInstall_Win32.sln $(TEMP_DIR)/jinstallproj/

# currently a .vcproj file is only created for Makefile.iftw
	$(MAKE) -f Makefile.iftw create_projectfiles
# The others would be added here and Makefile.* if needed
#	$(MAKE) -f Makefile.jxpi create_projectfiles
#	$(MAKE) -f Makefile.xpi create_projectfiles
#	$(MAKE) -f Makefile.cab create_projectfiles
#	$(MAKE) -f Makefile.jkernel create_projectfiles
#	$(MAKE) -f Makefile.jchrome create_projectfiles
ifneq ($(J4B), true)
#	$(MAKE) -f Makefile.jxpirv create_projectfiles
#	$(MAKE) -f Makefile.iftwrv create_projectfiles
#	$(MAKE) -f Makefile.cabrv create_projectfiles
#	$(MAKE) -f Makefile.jchromerv create_projectfiles
#	$(MAKE) -f Makefile.xpirv create_projectfiles
endif


# It seems that under Cygwin, TARGET_CHK will not overwrite InstallerFiles.h.
jchksum: $(OBJDIR)/$(TARGET_CHK)
	$(RM) -f $(OBJDIR)/InstallerFiles.h
ifdef ALT_CHECKSUM
	$(CP) $(ALT_CHECKSUM) $(OBJDIR)/InstallerFiles.h
else
#Want to add FCS_VERSION to InstallerFiles.h, but only when we are NOT building a plus build, so do it when there is NO ALT_CHECKSUM (which indicates a plus build).
#  For the plus build, FCS_VERSION will already be in InstallerFiles.h since we copy the FCS version of this file in that case
	$(ECHO) '#define FCS_VERSION "$(FULL_VERSION)"' > $(OBJDIR)/InstallerFiles.h
	$(CD) $(ONLINE_TMPDIR); $(OBJDIR)/$(TARGET_CHK) $(FILE_LIST) >> $(OBJDIR)/InstallerFiles.h
endif
	$(ECHO) "InstallerFiles.h content:"
	$(CAT) $(OBJDIR)/InstallerFiles.h

# create static structure in PatchStruct.h, like:
#    static struct patch_info Patches[] = {
#        { "1.6.0_10", "1138312109"},
#        { "1.6.0_11", "0123456789abcdef"}
#    };
patches_cksums:
# delete PatchStruct.h in case there is a problem overwriting with Cygwin
	$(RM) -f $(OBJDIR)/PatchStruct.h
	$(ECHO) "static PATCH_INFO Patches[] = {"                       > $(OBJDIR)/PatchStruct.h
	for jre in $(PATCH_LIST) ; do \
		ck_sum=`$(CAT) $(ALT_BASE_IMAGE_DIR)/cksum_$${jre}` ; \
		$(ECHO) $$jre $$ck_sum | $(NAWK) 'BEGIN { FS=" " } {print "{ \"" substr($$1, 4) "\", \"" substr($$1, 4, 8) "\", \"" $$2 "\" }," }' >> $(OBJDIR)/PatchStruct.h; \
	done;
	$(ECHO) "};"                                                            >> $(OBJDIR)/PatchStruct.h

jre: optimized debug 



debug:

$(OBJDIR)/$(TARGET_CHK): $(FILES_ochk)
ifeq ($(ARCH), amd64)
	$(LINK) -out:$(OBJDIR)/$(TARGET_CHK) advapi32.lib wintrust.lib bufferoverflowu.lib $(FILES_ochk)
else
	$(LINK) -out:$(OBJDIR)/$(TARGET_CHK) advapi32.lib wintrust.lib $(FILES_ochk)
endif


optimized: $(OBJDIR)
	$(MAKE) jchksum VARIANT=OPT
	$(MAKE) -f Makefile.jxpi all
	$(MAKE) -f Makefile.xpi all
	$(MAKE) -f Makefile.iftw all
	$(MAKE) -f Makefile.cab all
	$(MAKE) -f Makefile.jkernel all
	$(MAKE) -f Makefile.jchrome all
ifneq ($(J4B), true)
	$(MAKE) -f Makefile.jxpirv all
	$(MAKE) -f Makefile.iftwrv all
	$(MAKE) -f Makefile.cabrv all
	$(MAKE) -f Makefile.jchromerv all
	$(MAKE) -f Makefile.xpirv all
endif


	$(CP) -f $(TEMP_DIR)/jinstall-iftw/$(OBJECT_SUFFIX)/jinstall_iftw.exe $(TEMPPATCHDIR)/online-final/$(JRE_SHORT_BUNDLE_NAME)-iftw.exe
	$(CP) -f $(TEMP_DIR)/jinstall-jchrome/$(OBJECT_SUFFIX)/jinstall_jchrome.exe $(TEMPPATCHDIR)/online-final/$(JCHROME_BUNDLE)
	$(CP) -f $(TEMP_DIR)/jinstall-cab/$(OBJECT_SUFFIX)/jinstall_cab.exe $(TEMPPATCHDIR)/online-final/$(JCAB_BUNDLE)
	$(CP) -f $(TEMP_DIR)/jinstall-jxpi/$(OBJECT_SUFFIX)/jinstall_jxpi.exe $(TEMPPATCHDIR)/online-final/$(JXPI_BUNDLE)
	$(CP) -f $(TEMP_DIR)/jinstall-jkernel/$(OBJECT_SUFFIX)/jinstall_jkernel.exe $(TEMPPATCHDIR)/online-final/$(JRE_SHORT_BUNDLE_NAME)-iftw-k.exe
	$(CP) -f $(TEMP_DIR)/jinstall-xpi/$(OBJECT_SUFFIX)/jinstall_xpi.exe $(TEMPPATCHDIR)/online-final/$(XPI_BUNDLE)
ifneq ($(J4B), true)
	$(CP) -f $(TEMP_DIR)/jinstall-xpi-rv/$(OBJECT_SUFFIX)/jinstall_xpi_rv.exe $(TEMPPATCHDIR)/online-final/$(XPI_RV_BUNDLE)
	$(CP) -f $(TEMP_DIR)/jinstall-jxpi-rv/$(OBJECT_SUFFIX)/jinstall_jxpi_rv.exe $(TEMPPATCHDIR)/online-final/$(JXPI_RV_BUNDLE)
	$(CP) -f $(TEMP_DIR)/jinstall-iftw-rv/$(OBJECT_SUFFIX)/jinstall_iftw_rv.exe $(TEMPPATCHDIR)/online-final/$(JRE_SHORT_BUNDLE_NAME)-iftw-rv.exe
	$(CP) -f $(TEMP_DIR)/jinstall-cab-rv/$(OBJECT_SUFFIX)/jinstall_cab_rv.exe $(TEMPPATCHDIR)/online-final/$(JCAB_RV_BUNDLE)
	$(CP) -f $(TEMP_DIR)/jinstall-jchrome-rv/$(OBJECT_SUFFIX)/jinstall_jchrome_rv.exe $(TEMPPATCHDIR)/online-final/$(JCHROME_RV_BUNDLE)
endif
clean::
	$(RM) -r $(TEMPDIR)
	$(RM) -r $(TEMP_DIR)/jinstallproj


#
# Phonies to avoid accidents.
#
.PHONY: all create_projectfiles optimized debug clobber clean
