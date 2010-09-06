PACKAGE=com/sun/kenc/util

CLASSES=LogWrapperBase IndentingPrintWriter StringUtil
SOURCE=$(CLASSES:%=$(PACKAGE)/%.java)
OBJECT=$(CLASSES:%=$(PACKAGE)/%.class)

MCROOTS= Activation Interceptors IOR Naming ORBUtil POA Util OMG
MCFILES=$(MCROOTS:%=data/%.mc)
MCSOURCE=$(MCROOTS:%=$(PACKAGE)/%SystemException.java)
MCCLASS=$(MCROOTS:%=$(PACKAGE)/%SystemException.class)
MCRESOURCE=$(MCROOTS:%=resources/%SystemException.resource)

JARDIR=lib
UTILJAR=$(JARDIR)/util.jar
JSCHEMEJAR=$(JARDIR)/jscheme.jar
JARS=$(UTILJAR):$(JSCHEMEJAR)

all: mkdir jar resources classes 

mkdir : 
	@-mkdir resources

clean:
	@-rm $(OBJECT) $(MCSOURCE) $(MCOBJECT) $(MCCLASS) $(MCRESOURCE) $(UTILJAR)

test:
	@echo "PACKAGE	    :" $(PACKAGE)
	@echo "CLASSES	    :" $(CLASSES)
	@echo "SOURCE	    :" $(SOURCE)
	@echo "OBJECT	    :" $(OBJECT)
	@echo "MCROOTS	    :" $(MCROOTS)
	@echo "MCFILES	    :" $(MCFILES)
	@echo "MCCLASS	    :" $(MCCLASS)
	@echo "MCRESOURCE   :" $(MCRESOURCE)
	
jar: $(UTILJAR)

$(UTILJAR) : $(OBJECT)
	jar cvf $(JARDIR)/util.jar $(OBJECT)

$(OBJECT) : $(SOURCE)

classes: $(MCCLASS)

$(MCCLASS) : $(MCSOURCE)

$(MCSOURCE) : $(MCFILES)

resources: $(MCRESOURCE)

$(MCRESOURCE) : $(MCFILES)

# This target does not compile, because the generated source code requires
# com.sun.corba.se.impl.util.SUNVMCID, which would normally be built in the
# ee package.  This should not be a problem once everything is merged into the
# workspace.
genclasses: $(MCCLASS)

$(PACKAGE)/%SystemException.java : data/%.mc
	./mc make-class $<
	mv *SystemException.java $(PACKAGE)

resources/%SystemException.resource : data/%.mc
	./mc make-resource $<
	mv *.resource resources

$(PACKAGE)/%.class : $(PACKAGE)/%.java
	javac -classpath $(JARS) $<
