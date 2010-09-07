#
# Copyright (c) 2006, 2007, Oracle and/or its affiliates. All rights reserved.
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

# A list of object files built without the platform specific PIC flags, e.g.
# -fPIC on linux. Performance measurements show that by compiling GC related 
# code, we could significantly reduce the GC pause time on 32 bit Linux/Unix
# platforms. See 6454213 for more details.
include $(GAMMADIR)/make/scm.make

ifneq ($(OSNAME), windows)
  ifndef LP64
    NONPIC_DIRS  = memory oops gc_implementation gc_interface 
    NONPIC_DIRS  := $(foreach dir,$(NONPIC_DIRS), $(GAMMADIR)/src/share/vm/$(dir))
    # Look for source files under NONPIC_DIRS
    NONPIC_FILES := $(foreach dir,$(NONPIC_DIRS),\
                      $(shell find $(dir) \( $(SCM_DIRS) \) -prune -o \
		      -name '*.cpp' -print))
    NONPIC_OBJ_FILES := $(notdir $(subst .cpp,.o,$(NONPIC_FILES)))
  endif
endif
