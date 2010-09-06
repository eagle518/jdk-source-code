#
# @(#)Makefile	1.46 04/04/06
#
# Copyright 2004 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#


include ./Makefile.master

GNUMAKE=	gnumake

# Note:  have to build the "rt" package after common, but before the
# other packages. See Makefile.targ.
SUBDIRS-all=	common .WAIT $(SOLARIS_PACKAGE_LIST_BUILD)

SUBDIRS-ext=

SUBDIRS=	$(SUBDIRS-all) $(SUBDIRS-ext)

all :=		TARGET= install
ext :=		TARGET= install
world :=	TARGET= install
packages :=	TARGET= install
bundle :=	TARGET= install
clean :=	TARGET= clean
clobber :=	TARGET= clobber

.PARALLEL:	$(SUBDIRS) 

all:		$(SUBDIRS-all)

ext:		$(SUBDIRS-ext)

packages \
world:		$(SUBDIRS-all) $(SUBDIRS-ext) verify

# Make sure that no files get into more than one package
verify: FRC
	for i in $(SOLARIS_PACKAGE_LIST_BUILD) ; do \
	    if [ -d $(PKG_ARCHIVE)/$$i/reloc ]; then \
		$(CD) $(PKG_ARCHIVE)/$$i/reloc ; $(FIND) * -type f >> $(TMP_DIR)/list ; \
	    else  \
		$(TOUCH) $(TMP_DIR)/list ; \
	    fi \
	done;
	$(SORT) $(TMP_DIR)/list > $(TMP_DIR)/list2
	$(UNIQ) -d $(TMP_DIR)/list2 > $(TMP_DIR)/list3
	for i in `$(CAT) $(TMP_DIR)/list3` ; do \
		$(ECHO) $$i "detected in more than one package" ; \
		$(RM) $(TMP_DIR)/list $(TMP_DIR)/list2 $(TMP_DIR)/list3 ; \
		exit 1 ; \
	done;
	$(RM) $(TMP_DIR)/list $(TMP_DIR)/list2 $(TMP_DIR)/list3

clean \
clobber:	$(SUBDIRS)

$(SUBDIRS):	FRC
		@cd $@; pwd; $(MAKE) -i MAN_PAGE_FILES="$(MAN_PAGE_FILES)" \
		JDK_DOCS="$(JDK_DOCS)" UNIXCOMMAND_PATH=$(UNIXCOMMAND_PATH) $(TARGET)

FRC:
