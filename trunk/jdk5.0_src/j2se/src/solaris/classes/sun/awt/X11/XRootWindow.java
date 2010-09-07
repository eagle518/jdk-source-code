/*
 * @(#)XRootWindow.java	1.6 04/02/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
        synchronized(XToolkit.getAWTLock()) {
            if (xawtRootWindow == null) {
                xawtRootWindow = new XRootWindow();
                xawtRootWindow.init(xawtRootWindow.getDelayedParams().delete(DELAYED));
            }
            return xawtRootWindow;
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
