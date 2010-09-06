# @(#)shared.make	1.6 03/12/23 16:35:35
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

DEFAULTACTIONS=clean post_update create

default:: $(SUBDIRS)

!ifndef DIR
DIR=.
!endif

!ifndef CPP
CPP=cl.exe
!endif


!ifdef SUBDIRS
$(SUBDIRS): FORCE
	@if not exist $@ mkdir $@
	@if not exist $@\local.make echo # Empty > $@\local.make
	@echo nmake $(ACTION) in $(DIR)\$@
	@cd $@ && $(MAKE) /NOLOGO /f $(WorkSpace)\build\windows\$(DIR)\$@\Makefile $(ACTION) DIR=$(DIR)\$@
!endif

# Creates the needed directory
create::
!if "$(DIR)" != "."
	@echo mkdir $(DIR)
!endif

# Epilog to update for generating derived files
post_update::

# Removes scrap files
clean:: FORCE
	-@rm -f *.OLD *.publish

# Remove all scrap files and all generated files
pure:: clean
	-@rm -f *.OLD *.publish

$(DEFAULTACTIONS) $(ACTIONS)::
!ifdef SUBDIRS
	@$(MAKE) -nologo ACTION=$@ DIR=$(DIR)
!endif

FORCE:


