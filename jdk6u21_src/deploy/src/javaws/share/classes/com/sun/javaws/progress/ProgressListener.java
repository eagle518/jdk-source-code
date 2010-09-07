/*
 * @(#)ProgressListener.java	1.2 10/03/24
 *
 * Copyright (c) 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws.progress;

import java.awt.Component;
import java.net.URL;

public interface ProgressListener {


    /*
     * from:  LaunchDownload.downloadProgress
     */

        // Downloading a JRE
        public void jreDownload(String version, URL location);

        // Downloading a JNLP file for an extension
        public void extensionDownload(String name, int remaining);

        // The percentage might not be readSoFar/total due to the validation
        // pass
        // The total and percentage will be -1 for unknown
        public void progress  (URL url, String version, long readSoFar,
                long total, int overallPercent);
        public void validating(URL url, String version, long entry,
                long total, int overallPercent);
        public void upgradingArchive(URL url, String version, int patchPercent,
                             int overallPercent);
        public void downloadFailed(URL url, String version);


    /*
     * from:  LaunchDownload.downloadProgressWindow
     */

        public void setHeading(final String text, final boolean singleLine);
        public void setStatus(String text);
        public Component getOwner();
        public void setVisible(final boolean show);
        public void setProgressBarVisible(final boolean isVisible);
        public void setProgressBarValue(final int value);
        public void showLaunchingApplication(final String title);

}
