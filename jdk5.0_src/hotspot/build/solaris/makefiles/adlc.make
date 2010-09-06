# 
# @(#)adlc.make	1.45 03/12/23 16:35:23
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
#

# This makefile (adlc.make) is included from the adlc.make in the
# build directories.
# It knows how to compile, link, and run the adlc.

include $(GAMMADIR)/build/solaris/makefiles/rules.make

# #########################################################################

# OUTDIR must be the same as AD_Dir = $(GENERATED)/adfiles in top.make:
OUTDIR  = ../generated/adfiles

ARCH = $(Platform_arch)
SRCARCH = $(ARCH)
OS = $(Platform_os_family)

SOURCE.AD = $(OUTDIR)/$(OS)_$(ARCH).ad 

SOURCES.AD = $(GAMMADIR)/src/cpu/$(ARCH)/vm/$(SRCARCH).ad \
	     $(GAMMADIR)/src/os_cpu/$(OS)_$(ARCH)/vm/$(OS)_$(ARCH).ad 

Src_Dirs += $(GAMMADIR)/src/share/vm/adlc

EXEC	= $(OUTDIR)/adlc

# set VPATH so make knows where to look for source files
Src_Dirs_V = ${Src_Dirs} $(GENERATED)/incls
VPATH    += $(Src_Dirs_V:%=%:)

# set INCLUDES for C preprocessor
Src_Dirs_I = ${Src_Dirs} $(GENERATED)
INCLUDES += $(Src_Dirs_I:%=-I%)

# Force assertions on.
SYSDEFS		+= -DASSERT
CPPFLAGS	= $(SYSDEFS) $(INCLUDES)

# We need libCstd.so for adlc 
CFLAGS += -library=Cstd
LFLAGS += -library=Cstd

# CFLAGS_WARN holds compiler options to suppress/enable warnings.
CFLAGS		+= $(CFLAGS_WARN)

OBJECTNAMES = \
	adlparse.o \
	archDesc.o \
	arena.o \
	dfa.o \
	dict2.o \
	filebuff.o \
	forms.o \
	formsopt.o \
	formssel.o \
	main.o \
	adlc-opcodes.o \
	output_c.o \
	output_h.o \

OBJECTS = $(OBJECTNAMES:%=$(OUTDIR)/%)

GENERATEDNAMES = \
        ad_$(ARCH).cpp \
        ad_$(ARCH).hpp \
        ad_$(ARCH)_clone.cpp \
        ad_$(ARCH)_expand.cpp \
        ad_$(ARCH)_format.cpp \
        ad_$(ARCH)_gen.cpp \
        ad_$(ARCH)_misc.cpp \
        ad_$(ARCH)_peephole.cpp \
        ad_$(ARCH)_pipeline.cpp \
        adGlobals_$(ARCH).hpp \
        dfa_$(ARCH).cpp \

GENERATEDFILES = $(GENERATEDNAMES:%=$(OUTDIR)/%)
        

# #########################################################################

all: $(EXEC)

$(EXEC) : $(OBJECTS)
	@echo Making adlc
ifeq ("${Platform_arch}", "i486")
	$(QUIETLY) $(LINK_NOPROF.CC) -o $(EXEC) $(OBJECTS)
else
	$(QUIETLY) $(LINK.CC) -o $(EXEC) $(OBJECTS)
endif

# Random dependencies:
$(OBJECTS): opcodes.hpp classes.hpp adlc.hpp adlcVMDeps.hpp adlparse.hpp archDesc.hpp arena.hpp dict2.hpp filebuff.hpp forms.hpp formsopt.hpp formssel.hpp

# The source files refer to ostream.h, which sparcworks calls iostream.h
$(OBJECTS): ostream.h

ostream.h :
	@echo >$@ '#include <iostream.h>'

dump:
	: OUTDIR=$(OUTDIR)
	: OBJECTS=$(OBJECTS)
	: products = $(GENERATEDFILES)

all: $(GENERATEDFILES)

$(GENERATEDFILES): refresh_adfiles

# Get a unique temporary directory name, so multiple makes can run in parallel.
# Note that product files are updated via "mv", which is atomic.
TEMPDIR := $(OUTDIR)/mktmp$(shell echo $$$$)

ADLCFLAGS = -q -T

ifdef LP64
ADLCFLAGS += -D_LP64
else
ADLCFLAGS += -U_LP64
endif

#
# adlc_updater is a simple sh script, under sccs control. It is
# used to selectively update generated adlc files. This should
# provide a nice compilation speed improvement.
#
ADLC_UPDATER_DIRECTORY = $(GAMMADIR)/build/solaris
ADLC_UPDATER = adlc_updater

# This action refreshes all generated adlc files simultaneously.
# The way it works is this:
# 1) create a scratch directory to work in.
# 2) if the current working directory does not have $(ADLC_UPDATER), copy it.
# 3) run the compiled adlc executable. This will create new adlc files in the scratch directory.
# 4) call $(ADLC_UPDATER) on each generated adlc file. It will selectively update changed or missing files.
# 5) If we actually updated any files, echo a notice.
#
refresh_adfiles: $(EXEC) $(SOURCE.AD)
	@rm -rf $(TEMPDIR); mkdir $(TEMPDIR)
	$(QUIETLY) [ -f $(ADLC_UPDATER) ] || ( cp $(ADLC_UPDATER_DIRECTORY)/$(ADLC_UPDATER) . ; chmod +x $(ADLC_UPDATER) )
	$(QUIETLY) $(EXEC) $(ADLCFLAGS) $(SOURCE.AD) \
 -c$(TEMPDIR)/ad_$(ARCH).cpp -h$(TEMPDIR)/ad_$(ARCH).hpp -a$(TEMPDIR)/dfa_$(ARCH).cpp -v$(TEMPDIR)/adGlobals_$(ARCH).hpp \
	    || { rm -rf $(TEMPDIR); exit 1; }
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH).cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH).hpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH)_clone.cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH)_expand.cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH)_format.cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH)_gen.cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH)_misc.cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH)_peephole.cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) ad_$(ARCH)_pipeline.cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) adGlobals_$(ARCH).hpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) ./$(ADLC_UPDATER) dfa_$(ARCH).cpp $(TEMPDIR) $(OUTDIR)
	$(QUIETLY) [ -f $(TEMPDIR)/made-change ] \
		|| echo "Rescanned $(SOURCE.AD) but encountered no changes."
	$(QUIETLY) rm -rf $(TEMPDIR)


# #########################################################################

$(SOURCE.AD): $(SOURCES.AD)
	$(QUIETLY) cat $(SOURCES.AD) > $(SOURCE.AD)

$(OUTDIR)/%.o: %.cpp
	@echo Compiling $<
	$(QUIETLY) $(COMPILE.CC) -o $@ $<
	$(COMPILE_DONE_MSG)

# Some object files are given a prefix, to disambiguate
# them from objects of the same name built for the VM.
$(OUTDIR)/adlc-%.o: %.cpp
	@echo Compiling $<
	$(QUIETLY) $(COMPILE.CC) -o $@ $<
	$(COMPILE_DONE_MSG)

# #########################################################################

clean	:
	rm $(OBJECTS)

cleanall :
	rm $(OBJECTS) $(EXEC)

# #########################################################################

.PHONY: all dump refresh_adfiles clean cleanall
