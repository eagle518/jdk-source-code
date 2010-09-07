/*
 * @(#)XMouseInfoPeer.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Point;
import java.awt.Window;
import java.awt.GraphicsEnvironment;
import java.awt.GraphicsConfiguration;
import java.awt.GraphicsDevice;
import java.awt.peer.MouseInfoPeer;

public class XMouseInfoPeer implements MouseInfoPeer {

    /**
     * Package-private constructor to prevent instantiation.
     */
    XMouseInfoPeer() {
    }

    public int fillPointWithCoords(Point point) {
        long display = XToolkit.getDisplay();
        GraphicsEnvironment ge = GraphicsEnvironment.
                                     getLocalGraphicsEnvironment();
        GraphicsDevice[] gds = ge.getScreenDevices();
        int gdslen = gds.length;

        synchronized (XToolkit.getAWTLock()) {
            for (int i = 0; i < gdslen; i++) {
                long screenRoot = XlibWrapper.RootWindow(display, i); 
                boolean pointerFound = XlibWrapper.XQueryPointer(
                                           display, screenRoot,
                                           XlibWrapper.larg1,  // root_return
                                           XlibWrapper.larg2,  // child_return
                                           XlibWrapper.larg3,  // xr_return 
                                           XlibWrapper.larg4,  // yr_return
                                           XlibWrapper.larg5,  // xw_return 
                                           XlibWrapper.larg6,  // yw_return
                                           XlibWrapper.larg7); // mask_return
                if (pointerFound) {
                    point.x = Native.getInt(XlibWrapper.larg3);
                    point.y = Native.getInt(XlibWrapper.larg4);
                    return i;
                }
            }
        }

        // this should never happen
        assert false : "No pointer found in the system.";
        return 0;
    }

    public boolean isWindowUnderMouse(Window w) {
        long display = XToolkit.getDisplay();

        // java.awt.Component.findUnderMouseInWindow checks that 
        // the peer is non-null by checking that the component
        // is showing.
        long window = ((XWindow)w.getPeer()).getWindow();
        
        synchronized (XToolkit.getAWTLock()) {
            return XlibWrapper.XQueryPointer(display, window, XlibWrapper.larg1,
                                             XlibWrapper.larg2, XlibWrapper.larg3,
                                             XlibWrapper.larg4, XlibWrapper.larg5,
                                             XlibWrapper.larg6, XlibWrapper.larg7);
        }
    }

}
