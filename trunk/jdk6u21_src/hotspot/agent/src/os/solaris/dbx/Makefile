#
# Copyright (c) 2000, 2003, Oracle and/or its affiliates. All rights reserved.
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


# Targets are:
#   32bit:  Build the 32 bit version in ./32bit
#   64bit:  Build the 64 bit version in ./64bit
#   helloWorld:  Build the helloWorld test program
#   all:    Build all of the above.  This is the default.
#
# NOTE: This makefile uses IOBuf.cpp, IOBuf.hpp, Buffer.cpp, and
#           Buffer.hpp from the src/os/win32/agent directory.

.PHONY: 32bit 64bit

ARCH_ORIG = $(shell uname -p)

# C++    := /java/devtools/$(ARCH_ORIG)/SUNWspro/SC6.1/bin/CC

C++    := CC
RM     := /usr/bin/rm
MKDIRS := /usr/bin/mkdir -p


WIN32_DIR := ../../win32
ARCH     := $(subst i386,i486,$(ARCH_ORIG))
# INCLUDES := -I/net/sparcworks.eng/export/set/sparcworks5/dbx_62_intg/dev/src/dbx -I$(WIN32_DIR)
INCLUDES := -I. -I$(WIN32_DIR)
CFLAGS_32bit := -xarch=v8
CFLAGS_64bit := -xarch=v9
CFLAGS   := -PIC -xO3 $(INCLUDES)
LIBS     := -lsocket -lnsl -lrtld_db
LDFLAGS  := -G

ifneq "$(ARCH)" "i486"
    CFLAGS += $(CFLAGS_$(VERSION))
    LDFLAGS += $(CFLAGS_$(VERSION))
endif

# We use IOBuf.hpp, IOBuf.cpp, Buffer.hpp, and Buffer.cpp from the win32 dir.
vpath %.cpp .:$(WIN32_DIR)
vpath %.hpp .:$(WIN32_DIR)

OBJS = $(VERSION)/svc_agent_dbx.o $(VERSION)/IOBuf.o $(VERSION)/Buffer.o



# The default is to make both 32 bit and 64 bit versions.
all:: 32bit 64bit

32bit 64bit:: 
	$(MKDIRS) $@
	$(MAKE) $@/libsvc_agent_dbx.so  helloWorld VERSION=$@

$(VERSION)/IOBuf.o: IOBuf.hpp
$(VERSION)/Buffer.o: Buffer.hpp
$(VERSION)/svc_agent_dbx.o: svc_agent_dbx.hpp

$(VERSION)/%.o: %.cpp
	$(C++) $(CFLAGS) -c $< -o $@

$(VERSION)/libsvc_agent_dbx.so:: $(OBJS)
	$(C++) $(LDFLAGS) -o $(VERSION)/libsvc_agent_dbx.so $(OBJS) $(LIBS)

# Would be nice to move this into a shared directory
helloWorld:: helloWorld.cpp
	$(C++) -g $< -o $@

clean::
	$(RM) -rf 32bit 64bit *.o helloWorld
