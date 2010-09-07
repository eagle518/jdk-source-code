#
# @(#)README.txt	1.3 03/23/10
#
# Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

Instructions on adding a java.lang.management demo.

Basically you want to mimic the java.lang.management demo "FullThreadDump".

* Create and populate a source directory at src/demo/management
  (This should include a small README.txt document on what this demo is)

* Make sure the appropriate "demo" copyright notice is added to all the
  source files.

* Edit src/share/demo/management/index.html and add in reference to this demo.

* Create make directory at make/mkdemo/management
  (Use the Demo.gmk file like make/mkdemo/management/FullThreadDump/Makefile)

* Edit make/mkdemo/management/Makefile and add in the new demo

* Create test directory at test/demo/management, create at least one test
  (Use test/demo/management/FullThreadDump as a template)

* Don't forget to SCCS in all the new files

* Build and create images (cd make && gnumake && gnumake images)
  (Do this on Solaris, Linux, and at least one Windows platform)

* Verify that browsing build/*/j2sdk-images/demo/management looks right

* Run the tests: cd test/demo/management && runregress .
  (Do this on Solaris, Linux, and at least one Windows platform)

Contact: jk-svc-group@sun.com for more information or help.

