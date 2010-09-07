/*
 * @(#)SecurityBaseline_pre.java	1.4 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

public class SecurityBaseline {

    private static final String BASELINE_VERSION_131 = "_SECURITY_BASELINE_131";
    private static final String BASELINE_VERSION_142 = "_SECURITY_BASELINE_142";
    private static final String BASELINE_VERSION_150 = "_SECURITY_BASELINE_150";
    private static final String BASELINE_VERSION_160 = "_SECURITY_BASELINE_160";
    private static final String CURRENT_VERSION = "_PLUGIN_MAJOR_VER._PLUGIN_MINOR_VER._PLUGIN_MICRO_VER_PLUGIN_UNDERSCORE_UPDAT_VER";
    private static final String CURRENT_NODOT_VERSION = "_PLUGIN_MAJOR_VER_PLUGIN_MINOR_VER_PLUGIN_MICRO_VER_PLUGIN_UNDERSCORE_UPDAT_VER";

    private static String getBaselineVersion131() {
	return BASELINE_VERSION_131;
    }

    private static String getBaselineVersion142() {
	return BASELINE_VERSION_142;
    }

    private static String getBaselineVersion150() {
	return BASELINE_VERSION_150;
    }

    private static String getBaselineVersion160() {
	return BASELINE_VERSION_160 ;
    }

    private static String getBaselineVersion(String requestedVersion) {
	if (requestedVersion.startsWith("1.3.1")) {
	    return getBaselineVersion131();
	} else if (requestedVersion.startsWith("1.4.2")) {
	    return getBaselineVersion142();
	} else if (requestedVersion.startsWith("1.5")) {
	    return getBaselineVersion150();
	} else if (requestedVersion.startsWith("1.6")) {
	    return getBaselineVersion160();
	} else {
	    return CURRENT_VERSION;
	}
    }

    public static boolean satisfiesSecurityBaseline(String version) {
	return (version.compareTo(getBaselineVersion(version)) >= 0);
    }

    /** Helper for determining which version of the Java Plug-In we're
        running on top of */
    public static String getCurrentVersion() {
        return CURRENT_VERSION;
    }

    /** Helper for determining which version of the Java Plug-In we're
        running on top of */
    public static String getCurrentNoDotVersion() {
        return CURRENT_NODOT_VERSION;
    }
}

