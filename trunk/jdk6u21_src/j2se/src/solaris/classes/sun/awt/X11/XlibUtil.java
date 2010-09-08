/*
 * @(#)XlibUtil.java	1.4 10/04/07
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.GraphicsEnvironment;

import sun.awt.X11GraphicsEnvironment;

/*
 * This class is a collection of utility methods that operate
 * with native windows.
 */
public class XlibUtil
{
    /**
     * The constructor is made private to eliminate any
     * instances of this class
    */
    private XlibUtil()
    {
    }

    /**
     * Xinerama-aware version of XlibWrapper.RootWindow method.
     */
    public static long getRootWindow(int screenNumber)
    {
        XToolkit.awtLock();
        try {
            X11GraphicsEnvironment x11ge = (X11GraphicsEnvironment)
                GraphicsEnvironment.getLocalGraphicsEnvironment();
            if (x11ge.runningXinerama()) {
                // all the Xinerama windows share the same root window
                return XlibWrapper.RootWindow(XToolkit.getDisplay(), 0);
            } else {
                return XlibWrapper.RootWindow(XToolkit.getDisplay(), screenNumber);
            }
        } finally {
            XToolkit.awtUnlock();
        }
    }

    /**
     * XSHAPE extension support.
     */
    
    // The variable is declared static as the XSHAPE extension cannot
    // be disabled at run-time, and thus is available all the time
    // once the check is passed.
    private static Boolean isShapingSupported = null;
    
    /**
     *  Returns whether the XSHAPE extension available
     */
    public static synchronized boolean isShapingSupported() {
        if (isShapingSupported == null) {
            XToolkit.awtLock();
            try {
                isShapingSupported = 
                    XlibWrapper.XShapeQueryExtension(
                            XToolkit.getDisplay(), 
                            XlibWrapper.larg1, 
                            XlibWrapper.larg2);
            } finally {
                XToolkit.awtUnlock();
            }
        }
        
        return isShapingSupported.booleanValue();
    }

}
