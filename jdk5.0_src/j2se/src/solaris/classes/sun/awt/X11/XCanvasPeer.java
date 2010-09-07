/*
 * @(#)XCanvasPeer.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import sun.awt.DisplayChangedListener;
import sun.awt.X11GraphicsConfig;
import sun.awt.X11GraphicsDevice;
import sun.awt.X11GraphicsEnvironment;

class XCanvasPeer extends XComponentPeer implements CanvasPeer,
                                                    DisplayChangedListener {

    XCanvasPeer() {}

    XCanvasPeer(XCreateWindowParams params) {
        super(params);
    }

    XCanvasPeer(Component target) {
	super(target);
    }

    /* --- DisplayChangedListener Stuff --- */
    public void displayChanged() {}
    public void paletteChanged() {}
    void resetTargetGC(Component target) {}
    /*
     * Called when the Window this
     * Canvas is on is moved onto another Xinerama screen.
     *
     * Canvases can be created with a non-defulat GraphicsConfiguration.  The
     * GraphicsConfiguration needs to be changed to one on the new screen,
     * preferably with the same visual ID.
     * 
     * Up-called for other windows peer instances (XPanelPeer, XWindowPeer).
     *
     * Should only be called from the event thread.
     */
    public void displayChanged(int screenNum) {
        resetLocalGC(screenNum);
        resetTargetGC(target);  /* call Canvas.setGCFromPeer() via native */
    }

    /* Set graphicsConfig to a GraphicsConfig with the same visual on the new
     * screen, which should be easy in Xinerama mode.
     *
     * Should only be called from displayChanged(), and therefore only from
     * the event thread.
     */
    void resetLocalGC(int screenNum) {
        // Opt: Only need to do if we're not using the default GC
        if (graphicsConfig != null) {
            X11GraphicsConfig parentgc;
            // save vis id of current gc
            int visual = graphicsConfig.getVisual();

            X11GraphicsDevice newDev = (X11GraphicsDevice) GraphicsEnvironment.
                getLocalGraphicsEnvironment().
                getScreenDevices()[screenNum];

            for (int i = 0; i < newDev.getNumConfigs(screenNum); i++) {
                if (visual == newDev.getConfigVisualId(i, screenNum)) {
                    // use that
                    graphicsConfig = (X11GraphicsConfig)newDev.getConfigurations()[i];
                    break;
                }
            }
            // just in case...
            if (graphicsConfig == null) {
                graphicsConfig = (X11GraphicsConfig) GraphicsEnvironment.
                    getLocalGraphicsEnvironment().
                    getScreenDevices()[screenNum].
                    getDefaultConfiguration();
            }
        }
    }
    protected boolean shouldFocusOnClick() {
        // Canvas should always be able to be focused by mouse clicks.
        return true;
    }
    public void setBackground(Color c) {
        boolean doRepaint = false;
        if( getPeerBackground() == null || 
           !getPeerBackground().equals( c ) ) {
            doRepaint = true;
        }    
        super.setBackground(c);
        if( doRepaint ) {
            target.repaint();
        }
    }
}

