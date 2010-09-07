/*
 * @(#)AxBridgeEmbeddedFrame.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.viewer.frame;

import sun.plugin.viewer.IExplorerPluginObject;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;

public class AxBridgeEmbeddedFrame extends IExplorerEmbeddedFrame {

    public AxBridgeEmbeddedFrame(int hWnd, IExplorerPluginObject obj) {
        super(hWnd, obj);
    }

    protected boolean traverseOut(boolean direction) {
	Trace.println("Giving focus to next control", TraceLevel.BASIC);
        transferFocus((int)getEmbedderHandle(), direction);
	return true;
    }

    //native code to give focus to the next/previous ActiveX control
    private native void transferFocus(int handle, boolean direction);
}


