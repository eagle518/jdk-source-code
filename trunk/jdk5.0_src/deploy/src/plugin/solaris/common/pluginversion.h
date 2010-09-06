/*
 * @(#)pluginversion.h	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#if !defined(_PLUGINVERSION_H_)
#define _PLUGINVERSION_H_ 1

/* When ever the version number of the plugin/jre changes you must CHANGE the
 * jpi-version accordingly, as well as the NODOTVERSION.  Also ADD new
 * java-applet and java-bean strings for new version.
 *
 * These values are also used by ControlPanel and ControlPanel.html which
 * are found in the build directory.  There are awk/sed commands in the GNUmakefile
 * file that do the extraction.
 *
 * The mime type to identify a plugin as an OJI plugin is added in the code
 * in the navig5 directory.
 *
*/

#define PLUGIN_MIMETABLE \
\
"application/x-java-applet::Java(tm) Plug-in"\
";application/x-java-applet;version=1.1::Java(tm) Plug-in"\
";application/x-java-applet;version=1.1.1::Java(tm) Plug-in"\
";application/x-java-applet;version=1.1.2::Java(tm) Plug-in"\
";application/x-java-applet;version=1.1.3::Java(tm) Plug-in"\
";application/x-java-applet;version=1.2::Java(tm) Plug-in"\
";application/x-java-applet;version=1.2.1::Java(tm) Plug-in"\
";application/x-java-applet;version=1.2.2::Java(tm) Plug-in"\
";application/x-java-applet;version=1.3::Java(tm) Plug-in"\
";application/x-java-applet;version=1.3.1::Java(tm) Plug-in"\
";application/x-java-applet;version=1.4::Java(tm) Plug-in"\
";application/x-java-applet;version=1.4.1::Java(tm) Plug-in"\
";application/x-java-applet;version=1.4.2::Java(tm) Plug-in"\
";application/x-java-applet;version=1.5::Java(tm) Plug-in"\
";application/x-java-applet;jpi-version=1.5::Java(tm) Plug-in"\
";application/x-java-bean::Java(tm) Plug-in"\
";application/x-java-bean;version=1.1::Java(tm) Plug-in"\
";application/x-java-bean;version=1.1.1::Java(tm) Plug-in"\
";application/x-java-bean;version=1.1.2::Java(tm) Plug-in"\
";application/x-java-bean;version=1.1.3::Java(tm) Plug-in"\
";application/x-java-bean;version=1.2::Java(tm) Plug-in"\
";application/x-java-bean;version=1.2.1::Java(tm) Plug-in"\
";application/x-java-bean;version=1.2.2::Java(tm) Plug-in"\
";application/x-java-bean;version=1.3::Java(tm) Plug-in"\
";application/x-java-bean;version=1.3.1::Java(tm) Plug-in"\
";application/x-java-bean;version=1.4::Java(tm) Plug-in"\
";application/x-java-bean;version=1.4.1::Java(tm) Plug-in"\
";application/x-java-bean;version=1.4.2::Java(tm) Plug-in"\
";application/x-java-bean;version=1.5::Java(tm) Plug-in"\
";application/x-java-bean;jpi-version=1.5::Java(tm) Plug-in"\
""
#endif
