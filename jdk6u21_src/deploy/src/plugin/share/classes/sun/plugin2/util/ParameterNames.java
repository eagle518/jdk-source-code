/*
 * @(#)ParameterNames.java	1.10 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.util;

/** Some common parameter names. */

public interface ParameterNames {
    // Parameter name for specifying the JRE version
    public static final String JAVA_VERSION = "java_version";

    // Parameter name for specifying the command-line arguments
    public static final String JAVA_ARGUMENTS = "java_arguments";

    // Parameter name for indicating that this applet should run in a separate JVM
    public static final String SEPARATE_JVM = "separate_jvm";

    public static final String EAGER_INSTALL = "eager_install";

    public static final String APPLET_STOP_TIMEOUT = "applet_stop_timeout";

    // Parameter name for indicating that a JRE has been installed and the config shall be read again
    public static final String JRE_INSTALLED = "__jre_installed";

    // applet parameter, available to the client applet as well
    public static final String APPLET_RELAUNCHED  = "__applet_relaunched";

    // parameter holding the time is us (long), when the JVM has been launched
    public static final String JVM_LAUNCH_TIME = "__jvm_launched";

    // parameter indicates that this is a SSV relaunch
    public static final String SSV_VALIDATED = "__applet_ssv_validated";

    // parameter indicates the SSVersion requested by applet
    public static final String SSV_VERSION = "__applet_ssv_version";

    public static final String BOX_BG_COLOR = "boxbgcolor";

    public static final String BOX_FG_COLOR = "boxfgcolor";
}
