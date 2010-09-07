#
# Copyright (c) 2005, 2009, Oracle and/or its affiliates. All rights reserved.
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

SASRCDIR = $(AGENT_DIR)/src/os/$(Platform_os_family)

SASRCFILES = $(SASRCDIR)/salibelf.c                   \
             $(SASRCDIR)/symtab.c                     \
             $(SASRCDIR)/libproc_impl.c               \
             $(SASRCDIR)/ps_proc.c                    \
             $(SASRCDIR)/ps_core.c                    \
             $(SASRCDIR)/LinuxDebuggerLocal.c

SAMAPFILE = $(SASRCDIR)/mapfile

DEST_SAPROC = $(JDK_LIBDIR)/$(LIBSAPROC)

# DEBUG_BINARIES overrides everything, use full -g debug information
ifeq ($(DEBUG_BINARIES), true)
  SA_DEBUG_CFLAGS = -g
endif

# if $(AGENT_DIR) does not exist, we don't build SA
# also, we don't build SA on Itanium or zero.

checkAndBuildSA:
	$(QUIETLY) if [ -d $(AGENT_DIR) -a "$(SRCARCH)" != "ia64" -a "$(SRCARCH)" != "zero" ] ; then \
	   $(MAKE) -f vm.make $(LIBSAPROC); \
	fi

SA_LFLAGS = $(MAPFLAG:FILENAME=$(SAMAPFILE)) $(LDFLAGS_HASH_STYLE)

$(LIBSAPROC): $(SASRCFILES) $(SAMAPFILE)
	$(QUIETLY) if [ "$(BOOT_JAVA_HOME)" = "" ]; then \
	  echo "ALT_BOOTDIR, BOOTDIR or JAVA_HOME needs to be defined to build SA"; \
	  exit 1; \
	fi
	@echo Making SA debugger back-end...
	$(QUIETLY) $(CC) -D$(BUILDARCH) -D_GNU_SOURCE                   \
                   $(SYMFLAG) $(ARCHFLAG) $(SHARED_FLAG) $(PICFLAG)     \
	           -I$(SASRCDIR)                                        \
	           -I$(GENERATED)                                       \
	           -I$(BOOT_JAVA_HOME)/include                          \
	           -I$(BOOT_JAVA_HOME)/include/$(Platform_os_family)    \
	           $(SASRCFILES)                                        \
	           $(SA_LFLAGS)                                         \
	           $(SA_DEBUG_CFLAGS)                                   \
	           -o $@                                                \
	           -lthread_db
	$(QUIETLY) [ -f $(LIBSAPROC_G) ] || { ln -s $@ $(LIBSAPROC_G); }

install_saproc: checkAndBuildSA
	$(QUIETLY) if [ -e $(LIBSAPROC) ] ; then             \
	  echo "Copying $(LIBSAPROC) to $(DEST_SAPROC)";     \
	  cp -f $(LIBSAPROC) $(DEST_SAPROC) && echo "Done";  \
	fi

.PHONY: checkAndBuildSA install_saproc
