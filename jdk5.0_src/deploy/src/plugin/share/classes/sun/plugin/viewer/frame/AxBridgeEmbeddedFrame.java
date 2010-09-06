/*
 * @(#)AxBridgeEmbeddedFrame.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.viewer.frame;

import sun.plugin.util.Trace;
import sun.plugin.viewer.IExplorerPluginObject;

public class AxBridgeEmbeddedFrame extends IExplorerEmbeddedFrame {

    public AxBridgeEmbeddedFrame(int hWnd, IExplorerPluginObject obj) {
        super(hWnd, obj);
    }

    //Registers the embedded frame to listen to the focus traversal events
    public void registerFocusListener() {
	registerListeners();
    }

    protected boolean traverseOut(boolean direction) {
	Trace.println("Giving focus to next control");
        transferFocus(handle, direction);
	return true;
    }

    //native code to give focus to the next/previous ActiveX control
    private native void transferFocus(int handle, boolean direction);
}


