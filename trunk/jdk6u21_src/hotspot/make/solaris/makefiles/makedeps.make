#
# Copyright (c) 1999, 2008, Oracle and/or its affiliates. All rights reserved.
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

include $(GAMMADIR)/make/solaris/makefiles/rules.make

COMPILE.JAVAC.FLAGS += -d $(OUTDIR)

MakeDepsSources=\
	$(GAMMADIR)/src/share/tools/MakeDeps/Database.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/DirectoryTree.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/DirectoryTreeNode.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/FileFormatException.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/FileList.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/FileName.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/Macro.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/MacroDefinitions.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/MakeDeps.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/MetroWerksMacPlatform.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/Platform.java \
	$(GAMMADIR)/src/share/tools/MakeDeps/UnixPlatform.java

MakeDepsOptions=
