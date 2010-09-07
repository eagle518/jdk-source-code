/*
 * @(#)ResultHandler.java	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.io.IOException;

import sun.plugin2.util.SystemUtil;

/** Abstracts the necessarily platform-dependent and somewhat
    browser-dependent code which needs to run while we are waiting
    for LiveConnect and other operations to complete. */

public abstract class ResultHandler {
    protected static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);

    /** Waits for a LiveConnect result to be provided with the
        specified ResultID, which is associated with the applet with
        the given AppletID. On Windows, this enters a while loop which
        waits on a native Windows Event object processing Windows
        messages; this ties in with the notifyMainThread() mechanism
        to wake up the browser once the result has been provided. 
        This detects an applet hopping to another JVM in case of a relaunch,
        where the GetApplet message is resend.
        This also detects if the JVM is no more available.
       */
    public void waitForResult(ResultID resultID, 
                              AppletID appletID) throws IOException {
        int appletJVMInstance = JVMManager.getManager().getJVMIDForApplet(appletID);
        int thisJVMInstance= appletJVMInstance;

        while (!LiveConnectSupport.resultAvailable(resultID) &&
               !JVMManager.getManager().instanceExited(thisJVMInstance) &&
               !JVMManager.getManager().appletExited(appletID) &&
               thisJVMInstance>=0) {

            waitForSignal();
            thisJVMInstance = JVMManager.getManager().getJVMIDForApplet(appletID);
        }
    }

    /** Waits for a LiveConnect result to be provided with the
        specified ResultID, which is associated with the applet with
        the given AppletID and the JVM instance with the given jvmID.
        On Windows, this enters a while loop which waits on a native
        Windows Event object processing Windows messages; this ties in
        with the notifyMainThread() mechanism to wake up the browser
        once the result has been provided. (FIXME: it's unclear
        whether this is really needed -- perhaps the
        JVMManager.appletExited() notification is sufficient.) 
        This detects an applet hopping to another JVM in case of a relaunch,
        where the GetApplet message is resend.
        This detects if the JVM is no more available, 
        or the applet to JVM mapping has changed, then we bail out.
        */
    public void waitForResult(ResultID resultID,
                              int jvmID,
                              AppletID appletID) {
        int thisJVMInstance = JVMManager.getManager().getJVMIDForApplet(appletID);

        while (!LiveConnectSupport.resultAvailable(resultID) &&
               !JVMManager.getManager().instanceExited(jvmID) &&
               !JVMManager.getManager().appletExited(appletID) &&
               thisJVMInstance>=0 && jvmID==thisJVMInstance ) {
            waitForSignal();
            thisJVMInstance = JVMManager.getManager().getJVMIDForApplet(appletID);
        }
    }

    public abstract void waitForSignal();

    public abstract void waitForSignal(long millis);
}
