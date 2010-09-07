/*
 * @(#)PluginEmbeddedFrame.java	1.4 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.io.*;

import sun.plugin2.message.*;
import sun.plugin2.util.*;

public class PluginEmbeddedFrame extends javax.swing.JFrame {
    private Pipe pipe;
    private int appletID;

    public PluginEmbeddedFrame(long handle,
                               long parentConnection,
                               boolean supportsXEmbed,
                               ModalityInterface modality,
                               Pipe pipe,
                               int appletID) {
        super();
        // If the SharedWindow interface didn't initialize, give the user a handle to move the window around
        if (SharedWindowProvider.initializedSuccessfully()) {
            setUndecorated(true);
        }
        // Turn off the window shadow
        getRootPane().putClientProperty("Window.hasShadow", Boolean.FALSE);
        // FIXME: this is the way it's supposed to be according to the docs
        // See http://developer.apple.com/technotes/tn2007/tn2196.html
        getRootPane().putClientProperty("Window.shadow", Boolean.FALSE);
        // getRootPane().putClientProperty("apple.awt.windowShadow.revalidateNow", new Object());
        authorize(parentConnection);
        this.pipe = pipe;
        this.appletID = appletID;
    }

    // We need to defer fetching the window handle until super.setVisible(true) completes
    public void setVisible(boolean b) {
        super.setVisible(b);
        if (b) {
            long windowHandle = SharedWindowAWT.windowHandleFor(this);
            // This isn't working properly
            //            SharedWindowAWT.setHasShadow(this, false);
            SharedWindowProvider.shareWindowHandle(windowHandle);
            try {
                pipe.send(new SetChildWindowHandleMessage(null, appletID, windowHandle));
            } catch (IOException e) {
                throw new RuntimeException(e);
            }
        }
    }

    public void synthesizeWindowActivation(boolean doActivate) {
        if (doActivate) {
            requestFocus();
            toFront();
        }
    }

    private static boolean authorized;
    private static synchronized void authorize(long parentConnection) {
        if (!authorized) {
            authorized = true;
            SharedWindowProvider.authorizeParent(parentConnection);
        }
    }
}
