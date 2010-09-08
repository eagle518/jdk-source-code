/*
 * @(#)XRootWindow.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

/**
 * This class represents AWT application root window functionality.
 * Object of this class is singleton, all window reference it to have
 * common logical ancestor
 */
class XRootWindow extends XBaseWindow {
    private static XRootWindow xawtRootWindow = null;
    static XRootWindow getInstance() {
        XToolkit.awtLock();
        try {
            if (xawtRootWindow == null) {
                xawtRootWindow = new XRootWindow();
                xawtRootWindow.init(xawtRootWindow.getDelayedParams().delete(DELAYED));
            }
            return xawtRootWindow;
        } finally {
            XToolkit.awtUnlock();
        }
    }

    private XRootWindow() {
        super(new XCreateWindowParams(new Object[] {DELAYED, Boolean.TRUE}));
    }

    public void postInit(XCreateWindowParams params){
        super.postInit(params);
        setWMClass(getWMClass());
    }

    protected String getWMName() {
        return XToolkit.getAWTAppClassName();
    }
    protected String[] getWMClass() {
        return new String[] {XToolkit.getAWTAppClassName(), XToolkit.getAWTAppClassName()};
    }

  /* Fix 4976517.  Return awt_root_shell to XToolkit.c */
    private static long getXRootWindow() {
        return getXAWTRootWindow().getWindow();
    }
}
