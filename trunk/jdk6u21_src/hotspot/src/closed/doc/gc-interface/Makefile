# 
# 
# Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

MACH=`/usr/bin/mach`
LATEX=/net/suntools.eng/export/tools/$(MACH)/bin/latex
DVIPS=/net/suntools.eng/export/tools/$(MACH)/bin/dvips
TEXFILE=hs-gen

all: $(TEXFILE).ps

$(TEXFILE).ps: $(TEXFILE).tex
	$(LATEX) $(TEXFILE)
	$(LATEX) $(TEXFILE)
	$(DVIPS) $(TEXFILE).dvi -o $(TEXFILE).ps
	$(RM)    $(TEXFILE).log $(TEXFILE).aux $(TEXFILE).dvi

clean:
	$(RM) $(TEXFILE).log $(TEXFILE).aux $(TEXFILE).dvi $(TEXFILE).ps
