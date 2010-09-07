#
# @(#)README.txt	1.5 03/23/10
#
# Copyright (c) 2004, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

Instructions on adding a jvmti demo agent.

Basically you want to mimic the jvmti demo agent "mtrace".

* Create and populate a source directory at src/demo/jvmti
  (Try and re-use code in agent_util area like src/demo/jvmti/mtrace)
  (This should include a small README.txt document on what this demo is)

* Make sure the appropriate "demo" copyright notice is added to all the
  source files.

* Edit src/share/demo/jvmti/index.html and add in reference to this demo.

* Create make directory at make/mkdemo/jvmti
  (Use the Demo.gmk file like make/mkdemo/jvmti/mtrace/Makefile)

* Edit make/mkdemo/jvmti/Makefile and add in the new demo

* Create test directory at test/demo/jvmti, create at least one test
  (Use test/demo/jvmti/mtrace as a template)

* Don't forget to SCCS in all the new files

* Build and create images (cd make && gnumake && gnumake images)
  (Do this on Solaris, Linux, and at least one Windows platform)

* Verify that browsing build/*/j2sdk-images/demo/jvmti looks right

* Run the tests: cd test/demo/jvmti && runregress .
  (Do this on Solaris, Linux, and at least one Windows platform)

Contact: jk-svc-group@sun.com for more information or help.

