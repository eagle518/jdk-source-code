/*
 * @(#)pluginversion.hpp	1.23 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

/* For some reason, FireFox 3 does not display the part of description strings 
 * after '(' (include '('). So we can not have (tm) there. It looks FF3 support display
 * the trade mark symbol in either ascii &#153 and html &trade.
 */

#define PLUGIN_MIMETABLE \
\
"application/x-java-applet::Java&#153 Plug-in Applet"\
";application/x-java-applet;version=1.1::Java&#153 Plug-in"\
";application/x-java-applet;version=1.1.1::Java&#153 Plug-in"\
";application/x-java-applet;version=1.1.2::Java&#153 Plug-in"\
";application/x-java-applet;version=1.1.3::Java&#153 Plug-in"\
";application/x-java-applet;version=1.2::Java&#153 Plug-in"\
";application/x-java-applet;version=1.2.1::Java&#153 Plug-in"\
";application/x-java-applet;version=1.2.2::Java&#153 Plug-in"\
";application/x-java-applet;version=1.3::Java&#153 Plug-in"\
";application/x-java-applet;version=1.3.1::Java&#153 Plug-in"\
";application/x-java-applet;version=1.4::Java&#153 Plug-in"\
";application/x-java-applet;version=1.4.1::Java&#153 Plug-in"\
";application/x-java-applet;version=1.4.2::Java&#153 Plug-in"\
";application/x-java-applet;version=1.5::Java&#153 Plug-in"\
";application/x-java-applet;version=1.6::Java&#153 Plug-in"\
";application/x-java-applet;jpi-version=_PLUGIN_MAJOR_MIMETYPE_PLUGIN_UNDERSCORE_UPDAT_VER::Java&#153 Plug-in"\
";application/x-java-bean::Java&#153 Plug-in JavaBeans"\
";application/x-java-bean;version=1.1::Java&#153 Plug-in"\
";application/x-java-bean;version=1.1.1::Java&#153 Plug-in"\
";application/x-java-bean;version=1.1.2::Java&#153 Plug-in"\
";application/x-java-bean;version=1.1.3::Java&#153 Plug-in"\
";application/x-java-bean;version=1.2::Java&#153 Plug-in"\
";application/x-java-bean;version=1.2.1::Java&#153 Plug-in"\
";application/x-java-bean;version=1.2.2::Java&#153 Plug-in"\
";application/x-java-bean;version=1.3::Java&#153 Plug-in"\
";application/x-java-bean;version=1.3.1::Java&#153 Plug-in"\
";application/x-java-bean;version=1.4::Java&#153 Plug-in"\
";application/x-java-bean;version=1.4.1::Java&#153 Plug-in"\
";application/x-java-bean;version=1.4.2::Java&#153 Plug-in"\
";application/x-java-bean;version=1.5::Java&#153 Plug-in"\
";application/x-java-bean;version=1.6::Java&#153 Plug-in"\
";application/x-java-bean;jpi-version=_PLUGIN_MAJOR_MIMETYPE_PLUGIN_UNDERSCORE_UPDAT_VER::Java&#153 Plug-in"\
""
#endif
