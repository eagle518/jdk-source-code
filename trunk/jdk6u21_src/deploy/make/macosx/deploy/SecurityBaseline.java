/*
 * @(#)SecurityBaseline_pre.java	1.2 07/12/07
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

public class SecurityBaseline {

    private static final String BASELINE_VERSION_131 = "1.3.1_21";
    private static final String BASELINE_VERSION_142 = "1.4.2_16";
    private static final String BASELINE_VERSION_150 = "1.5.0_14";
    // NOTE: for the Mac OS X port we'll consider their latest, 1.6.0_05, as the current
    private static final String CURRENT_VERSION = "1.6.0_05";
    private static final String CURRENT_NODOT_VERSION = "160_05";

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
	return CURRENT_VERSION;
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

