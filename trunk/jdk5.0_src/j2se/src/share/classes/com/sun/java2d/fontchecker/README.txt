/*
 * @(#)README.txt	1.2 03/11/25
 *
 * Copyright 2002 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
Instructions for running FontChecker 
------------------------------------

FontChecker is a program designed to identify fonts that may cause JRE
crashes. Such fonts may be corrupted files, or badly constructed fonts.
Some crashes may also be due to bugs in the JRE's font code.
This test is designed to run quickly and silently as part of the JRE
installation process. It will only benefit users who install the JRE
via that mechanism. It cannot guarantee to identify all "bad fonts" because
the tests are minimal. Nor can it prevent problems due to fonts installed
subsequently to the JRE's installation. However it does ensure that the
vast majority of problem fonts are identified. This is important
"RAS" functionality. It is targeted at the consumer/plugin market where
there is substantial likelihood of end-users having installed software
packages which may be delivered with fonts that are not up to commercial
standards.

The test is designed to be "fail safe". If the program fails to run
properly it has no impact on the installer or on JRE execution.
Thus there is no need to monitor successful execution of the test.

The test is not a new "tool" in the sense of "javah" etc.
The test is not designed to be user executable or visible, and should
be unpacked by the installer into a temporary location, and executed
once the rest of the JRE is installed (ie as a postinstall step), and
can then be deleted from the temporary location once installation is
complete. Not deleting the jar file before execution is complete is
probably the sole reason that the installer may want to wait for
the program to complete.

The FontChecker application can be run directly from the jar 
file with this command: 
	%java -jar -cp FontChecker.jar FontChecker.jar -o <file>

The output file is a required parameter in this version of the application.
The JRE installer should use the above form, and use it to create an
output file which must be named "badfonts.txt" and be placed into
the JRE's lib\fonts directory eg:-

        java -jar -cp FontChecker.jar FontChecker.jar -o "C:\Program Files\jre\lib\fonts\badfonts.txt"

Note the lower case "badfonts.txt", and the string quotes because of the spaces
in the path name.
The location given here is an example and needs to be calculated at install
time as $JREHOME\lib\fonts\badfonts.txt
The location and name are important, because the JRE at runtime will
look for this exactly located name and file.
This location is private to that JRE instance. It will not affect
any other JRE installed on the system.

If running from a different directory than that containing the jar file,
use the form containing the full path to the jar file, eg :

	java -jar -cp C:\fc\FontChecker.jar C:\fc\FontChecker.jar -o "C:\Program Files\jre\lib\fonts\badfonts.txt"

FontChecker application accepts following command line flags. 
usage: java FontChecker -o outputfile
	                -v 

       -o is the name of the file to contains canonical path names of
          bad fonts that are identified. This file is not created if
          no bad fonts are found.

       -v verbose mode: print progress/warning messages. Not recommended
         for installer use.

       -w if running on Windows, use "javaw" to exec the sub-process.
