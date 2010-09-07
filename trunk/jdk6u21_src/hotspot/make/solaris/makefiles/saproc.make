#
# Copyright (c) 2005, 2008, Oracle and/or its affiliates. All rights reserved.
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

# Rules to build serviceability agent library, used by vm.make

# libsaproc[_g].so: serviceability agent

SAPROC = saproc
LIBSAPROC = lib$(SAPROC).so

SAPROC_G = $(SAPROC)$(G_SUFFIX)
LIBSAPROC_G = lib$(SAPROC_G).so

AGENT_DIR = $(GAMMADIR)/agent

SASRCDIR = $(AGENT_DIR)/src/os/$(Platform_os_family)/proc

SASRCFILES = $(SASRCDIR)/saproc.cpp

SAMAPFILE = $(SASRCDIR)/mapfile

DEST_SAPROC = $(JDK_LIBDIR)/$(LIBSAPROC)

# if $(AGENT_DIR) does not exist, we don't build SA

checkAndBuildSA:
	$(QUIETLY) if [ -d $(AGENT_DIR) ] ; then \
	   $(MAKE) -f vm.make $(LIBSAPROC); \
	fi

SA_LFLAGS = $(MAPFLAG:FILENAME=$(SAMAPFILE))

ifdef USE_GCC
SA_LFLAGS += -D_REENTRANT
else
SA_LFLAGS += -mt -xnolib -norunpath
endif

$(LIBSAPROC): $(SASRCFILES) $(SAMAPFILE)
	$(QUIETLY) if [ "$(BOOT_JAVA_HOME)" = "" ]; then \
	  echo "ALT_BOOTDIR, BOOTDIR or JAVA_HOME needs to be defined to build SA"; \
	  exit 1; \
	fi
	@echo Making SA debugger back-end...
	$(QUIETLY) $(CPP)                                               \
                   $(SYMFLAG) $(ARCHFLAG) $(SHARED_FLAG) $(PICFLAG)     \
	           -I$(SASRCDIR)                                        \
	           -I$(GENERATED)                                       \
	           -I$(BOOT_JAVA_HOME)/include                          \
	           -I$(BOOT_JAVA_HOME)/include/$(Platform_os_family)    \
	           $(SASRCFILES)                                        \
	           $(SA_LFLAGS)                                         \
	           -o $@                                                \
	           -ldl -ldemangle -lthread -lc
	[ -f $(LIBSAPROC_G) ] || { ln -s $@ $(LIBSAPROC_G); }

install_saproc: checkAndBuildSA
	$(QUIETLY) if [ -f $(LIBSAPROC) ] ; then             \
	  echo "Copying $(LIBSAPROC) to $(DEST_SAPROC)";     \
	  cp -f $(LIBSAPROC) $(DEST_SAPROC) && echo "Done";  \
	fi

.PHONY: checkAndBuildSA install_saproc
