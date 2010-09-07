/*
 * @(#)Pings.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.pings;

import com.sun.deploy.Environment;
import com.sun.deploy.config.Config;

public class Pings {

    // JavaFX ping
    public static final String JAVAFX_RT_JNLP_URL =
            "http://dl.javafx.com/javafx-rt.jnlp";
    public static final String JAVAFX_CACHE_JNLP_URL =
            "http://dl.javafx.com/javafx-cache.jnlp";
    public static final String JAVAFX_PRELOAD_INSTALL_METHOD = "jfxp";
    public static final String JAVAFX_AUTOUPDATE_INSTALL_METHOD = "jfxau";
    public static final String JAVAFX_INSTALL_COMPLETED_PING = "jfxic";

    public static final int JAVAFX_RETURNCODE_SUCCESS = 0;
    public static final int JAVAFX_RETURNCODE_UNKNOWN_FAILURE = 2;
    public static final int JAVAFX_RETURNCODE_DOWNLOAD_FAILED_FAILURE = 3;

    public static final String JAVAFX_UNDEFINED_PING_FIELD = "XX";

    // Setup information of the JavaFX ping and call into
    // native method in regutils to send the actual ping
    public static void sendJFXPing(String pingName, String currentJavaFxVersion,
            String requestedJavaFxVersion, int returnCode, String errorFile) {

        if (Environment.allowAltJavaFxRuntimeURL()) {
            // no ping if alternate JavaFX runtime URL is used
            return;
        }

        // <evar7>Method</evar7>
        String installMethod = JAVAFX_UNDEFINED_PING_FIELD;

        if (Environment.getJavaFxInstallMode() ==
                Environment.JAVAFX_INSTALL_AUTOUPDATE) {
            installMethod = JAVAFX_AUTOUPDATE_INSTALL_METHOD;
        } else if (Environment.getJavaFxInstallMode() ==
                Environment.JAVAFX_INSTALL_PRELOAD_INSTALLER){
            installMethod = JAVAFX_PRELOAD_INSTALL_METHOD;
        }

        // <evar18>CurrentJavaVersion</evar18>
        String currentJavaVersion = System.getProperty(
                "java.version");

        // <evar19>ErrorFile</evar19>
        String errFile = JAVAFX_UNDEFINED_PING_FIELD;

        if (errorFile != null) {
            errFile = errorFile;
        }

        // call into native method in regutils
        // TODO: this should be replaced with a Java implementation of SendPing
        Config.getInstance().sendJFXPing(installMethod, pingName,
                currentJavaFxVersion, requestedJavaFxVersion,
                currentJavaVersion, returnCode, errFile);
    }
}

