#
# @(#)Makefile	1.14 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.

SOURCEPATH=src
CLASSES=build
DIST=dist
RESOURCES=resources

J2DBENCH_CLASSES = \
	$(CLASSES)/j2dbench/J2DBench.class \
	$(CLASSES)/j2dbench/Destinations.class \
	$(CLASSES)/j2dbench/Group.class \
	$(CLASSES)/j2dbench/Result.class \
	$(CLASSES)/j2dbench/Modifier.class \
	$(CLASSES)/j2dbench/Node.class \
	$(CLASSES)/j2dbench/Option.class \
	$(CLASSES)/j2dbench/ResultSet.class \
	$(CLASSES)/j2dbench/TestEnvironment.class \
	$(CLASSES)/j2dbench/Test.class \
	$(CLASSES)/j2dbench/ui/EnableButton.class \
	$(CLASSES)/j2dbench/ui/CompactLayout.class \
	$(CLASSES)/j2dbench/tests/ImageTests.class \
	$(CLASSES)/j2dbench/tests/RenderTests.class \
	$(CLASSES)/j2dbench/tests/MiscTests.class \
	$(CLASSES)/j2dbench/tests/iio/IIOTests.class \
	$(CLASSES)/j2dbench/tests/iio/InputTests.class \
	$(CLASSES)/j2dbench/tests/iio/InputImageTests.class \
	$(CLASSES)/j2dbench/tests/iio/InputStreamTests.class \
	$(CLASSES)/j2dbench/tests/iio/OutputTests.class \
	$(CLASSES)/j2dbench/tests/iio/OutputImageTests.class \
	$(CLASSES)/j2dbench/tests/iio/OutputStreamTests.class \
	$(CLASSES)/j2dbench/tests/text/TextConstructionTests.class \
	$(CLASSES)/j2dbench/tests/text/TextMeasureTests.class \
	$(CLASSES)/j2dbench/tests/text/TextRenderTests.class \
	$(CLASSES)/j2dbench/tests/text/TextTests.class \
	$(CLASSES)/j2dbench/tests/GraphicsTests.class

J2DANALYZER_CLASSES = \
	$(CLASSES)/j2dbench/report/IIOComparator.class \
	$(CLASSES)/j2dbench/report/HTMLSeriesReporter.class \
	$(CLASSES)/j2dbench/report/J2DAnalyzer.class \
	$(CLASSES)/j2dbench/report/XMLHTMLReporter.class

J2DBENCH_RESOURCES = \
	$(CLASSES)/j2dbench/tests/iio/images \
	$(CLASSES)/j2dbench/tests/text/textdata

all: mkdirs J2DBench.jar J2DAnalyzer.jar

run: mkdirs J2DBench.jar
	java -jar J2DBench.jar

analyze: mkdirs J2DAnalyzer.jar
	java -jar J2DAnalyzer.jar

J2DBench.jar: \
	$(J2DBENCH_CLASSES) $(J2DBENCH_RESOURCES) \
	$(CLASSES)/j2dbench.manifest
	jar cvmf $(CLASSES)/j2dbench.manifest $(DIST)/J2DBench.jar -C $(CLASSES) j2dbench

J2DAnalyzer.jar: $(J2DANALYZER_CLASSES) $(CLASSES)/j2danalyzer.manifest
	jar cvmf $(CLASSES)/j2danalyzer.manifest \
		$(DIST)/J2DAnalyzer.jar -C $(CLASSES) j2dbench/report

$(CLASSES)/j2dbench/tests/iio/images: $(RESOURCES)/images
	cp -r $< $@
	rm -rf $@/SCCS

$(CLASSES)/j2dbench/tests/text/textdata: $(RESOURCES)/textdata
	cp -r $< $@
	rm -rf $@/SCCS

$(CLASSES)/j2dbench.manifest:
	echo "Main-Class: j2dbench.J2DBench" > $@

$(CLASSES)/j2danalyzer.manifest:
	echo "Main-Class: j2dbench.report.J2DAnalyzer" > $@

$(DIST):
	mkdir $(DIST)
$(CLASSES):
	mkdir $(CLASSES)

mkdirs: $(DIST) $(CLASSES)

$(CLASSES)/j2dbench/%.class: $(SOURCEPATH)/j2dbench/%.java
	javac -source 1.2 -target 1.2 -d $(CLASSES) -sourcepath $(SOURCEPATH) $<

clean:
	rm -rf $(CLASSES)
	rm -rf $(DIST)
