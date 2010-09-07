#
# @(#)Makefile	1.39 07/08/15
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR  = ../../../..
PACKAGE   = com.sun.java.swing.plaf.nimbus
PRODUCT   = com
SWING_SRC = $(SHARE_SRC)/classes/javax/swing

include $(BUILDDIR)/common/Defs.gmk
include $(BUILDDIR)/common/Classes.gmk

.PHONY: nimbus.clean

#
# Variables for building the Nimbus Generator
#

# Top directory for the Nimbus source generator tool
NIMBUS_GENERATOR_DIR=$(J2SE_TOPDIR)/make/tools/swing-nimbus

# Source for the Nimbus generator
NIMBUS_GENERATOR_SRC_DIR=${NIMBUS_GENERATOR_DIR}/classes

# Directory of libraries required to build the Nimbus generator
NIMBUS_GENERATOR_LIBS_DIR=${NIMBUS_GENERATOR_DIR}/libs

# Jar containing 1.6 versions of a few key AWT files
AWT16=${NIMBUS_GENERATOR_LIBS_DIR}/awt16.jar

# List of source files for the Nimbus generator
NIMBUS_GENERATOR_SRC=${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/beans/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/swingx/designer/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/swingx/designer/font/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/swingx/designer/jibxhelpers/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/swingx/designer/paint/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/swingx/designer/utils/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/swingx/designer/effects/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/swingx/graphics/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/synthdesigner/synthmodel/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/synthdesigner/synthmodel/jibxhelpers/*.java \
    ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/synthdesigner/generator/*.java

# List of libraries required to build the Nimbus generator
NIMBUS_GENERATOR_LIBS=${NIMBUS_GENERATOR_LIBS_DIR}/bcel.jar${CLASSPATH_SEPARATOR}${NIMBUS_GENERATOR_LIBS_DIR}/xpp3.jar${CLASSPATH_SEPARATOR}${NIMBUS_GENERATOR_LIBS_DIR}/jibx-bind.jar${CLASSPATH_SEPARATOR}${NIMBUS_GENERATOR_LIBS_DIR}/jibx-run.jar${CLASSPATH_SEPARATOR}${NIMBUS_GENERATOR_LIBS_DIR}/xmlpull_1_1_4.jar

# Output dir for the built Nimbus generator tool
NIMBUS_GENERATOR_OUTPUT_DIR=${GENSRCDIR}/nimbus-generator

# Path to the .class file for the Nimbus generator tool
NIMBUS_GENERATOR_CLASS=${NIMBUS_GENERATOR_OUTPUT_DIR}/org/jdesktop/synthdesigner/generator/Generator.class

# Copy the template files to here
NIMBUS_TEMPLATE_DEST=${NIMBUS_GENERATOR_OUTPUT_DIR}/org/jdesktop/synthdesigner/generator/resources

# Need a Java 6 JRE to run JIBX
RUN_JAVA=${JAVA_BOOT}
RUN_JAVAC=${JAVAC_BOOT}

#
# Variables for generating the Nimbus source using the Generator
#

# Relative direcctory to Nimbus sources
NIMBUS_PKG=com/sun/java/swing/plaf/nimbus

# Checked-in (i.e. non-generated) Nimbus sources
NIMBUS_SRC = $(SHARE_SRC)/classes/$(NIMBUS_PKG)

# File input into the generator tool
NIMBUS_SKIN_FILE=$(NIMBUS_SRC)/skin.laf

# Output dir for generated src files
NIMBUS_GENSRC_DIR=${GENSRCDIR}

#
# Rules
#

clean clobber:: nimbus.clean

nimbus.clean:
	@$(ECHO) "Running nimbus.clean target"
	${RM} -r ${NIMBUS_GENSRC_DIR}/${NIMBUS_PKG}	
	${RM} -r ${NIMBUS_GENERATOR_OUTPUT_DIR} 
	${RM} $(TEMPDIR)/NIMBUS_GENSRC_FILES

# build: nimbus_bldsrc
# nimbus_bldsrc: .compile.classlist

# Note: JAVA_SOURCE_LIST is a prereq of .compile.classlist, and
# .compile.classlist is (somehow) run for the build target, therefore the
# nimbus_bldsrc target is unnecessary.
#
# JAVA_SOURCE_LIST is redefined here because otherwise there are problems
# picking up all of the generated Nimbus sources.
$(JAVA_SOURCE_LIST) : $(TEMPDIR)/NIMBUS_GENSRC_FILES
	# Get list of all check-in src *and* generated src
	@$(TOUCH) $(JAVA_SOURCE_LIST)
	@$(LS) $(NIMBUS_SRC)/*.java >> $(JAVA_SOURCE_LIST)
	@$(LS) $(NIMBUS_GENSRC_DIR)/$(NIMBUS_PKG)/*.java >> $(JAVA_SOURCE_LIST)
	@$(CAT) $(JAVA_SOURCE_LIST)


# Target for generating the Nimbus source files
$(TEMPDIR)/NIMBUS_GENSRC_FILES: ${NIMBUS_SKIN_FILE} ${NIMBUS_GENERATOR_CLASS}
    # Run the generator to generate the Nimbus sources
	@$(ECHO) "Generating the Nimbus source files:"
	${RUN_JAVA} -Xbootclasspath/p:${AWT16} \
	  -classpath "${NIMBUS_GENERATOR_LIBS}${CLASSPATH_SEPARATOR}${NIMBUS_GENERATOR_OUTPUT_DIR}" \
	  org.jdesktop.synthdesigner.generator.Generator \
	  -skinFile $(NIMBUS_SKIN_FILE) \
	  -buildDir ${NIMBUS_GENSRC_DIR} -srcDir ${NIMBUS_GENSRC_DIR} \
	  -packagePrefix ${PACKAGE} -lafName Nimbus 
	touch $(TEMPDIR)/NIMBUS_GENSRC_FILES
	@$(ECHO) "Done running nimbus_gensrc"


# Target for building the Nimbus generator
${NIMBUS_GENERATOR_CLASS}:
	@$(ECHO) "Building the Nimbus generator:"
	${MKDIR} -p ${NIMBUS_GENERATOR_OUTPUT_DIR} 
	${RUN_JAVAC} -bootclasspath "${AWT16}${CLASSPATH_SEPARATOR}${BOOTDIR}/jre/lib/rt.jar" \
	 -d ${NIMBUS_GENERATOR_OUTPUT_DIR} -sourcepath ${NIMBUS_GENERATOR_SRC_DIR} \
	 -classpath "${NIMBUS_GENERATOR_LIBS}" ${NIMBUS_GENERATOR_SRC}
	$(MKDIR) -p ${NIMBUS_TEMPLATE_DEST}
	${RM} ${NIMBUS_TEMPLATE_DEST}/*.template 
	${CP} $(SHARE_SRC)/classes/${NIMBUS_PKG}/*.template ${NIMBUS_TEMPLATE_DEST}
	#run JIBX
	${RUN_JAVA} -Xbootclasspath/p:${AWT16} \
	  -classpath "${AWT16}${CLASSPATH_SEPARATOR}${NIMBUS_GENERATOR_LIBS}${CLASSPATH_SEPARATOR}${NIMBUS_GENERATOR_OUTPUT_DIR}" \
	  org.jibx.binding.Compile \
	  ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/swingx/designer/Designer.jibx.xml \
	  ${NIMBUS_GENERATOR_SRC_DIR}/org/jdesktop/synthdesigner/synthmodel/SynthModel.jibx.xml 
	@$(ECHO) "Done running nimbus_generator target"

