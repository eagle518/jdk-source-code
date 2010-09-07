/*
 * @(#)Applet2Listener.java	1.13 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.applet;

import com.sun.javaws.exceptions.ExitException;

/** An Applet2Listener can be registered with an Plugin2Manager to
    provide a minimal amount of interaction with the applet's
    lifecycle. */

public interface Applet2Listener {
    /** Returns true if it's OK to relaunch the applet in requested version
	of JRE, false to continue in current jvm. 
        No relaunch is triggered from here, use appletJRERelaunch() for relaunch.
     */
    public boolean appletSSVValidation(Plugin2Manager hostingManager) throws ExitException;
    /** The Java version indicates the version of the JRE to use for
        the relaunch. The jvmArgs contain all of the command-line
        arguments (including min/max heap) that need to be specified
        for the relaunched JVM. */
    public void appletJRERelaunch(Plugin2Manager hostingManager, String javaVersion, String jvmArgs);
    public boolean isAppletRelaunchSupported();
    public void appletLoaded(Plugin2Manager hostingManager);
    public void appletReady(Plugin2Manager hostingManager);
    public void appletErrorOccurred(Plugin2Manager hostingManager);

    public String getBestJREVersion(Plugin2Manager hostingManager, String javaVersionStr);
    /**
     * called when this listener gets released from attached listener queue
     */
    public void released(Plugin2Manager hostingManager); 
}
