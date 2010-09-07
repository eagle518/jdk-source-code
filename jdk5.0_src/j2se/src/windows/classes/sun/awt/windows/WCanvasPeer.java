/*
 * @(#)WCanvasPeer.java	1.25 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.lang.ref.WeakReference;
import sun.awt.Win32GraphicsDevice;

class WCanvasPeer extends WComponentPeer implements CanvasPeer {

    private static boolean eraseBackgroundDisabled;
    private static boolean eraseBackgroundOnResize;
    static {
        String prop;
        prop = (String)java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.awt.noerasebackground"));
        eraseBackgroundDisabled = (prop != null
                              && prop.length() > 0
                              && prop.charAt(0) == 't');
        prop = (String)java.security.AccessController.doPrivileged(
            new sun.security.action.GetPropertyAction("sun.awt.erasebackgroundonresize"));
        eraseBackgroundOnResize = (prop != null
                              && prop.length() > 0
                              && prop.charAt(0) == 't');
    }

    // Toolkit & peer internals

    WCanvasPeer(Component target) {
        super(target);
    }

    /*
     * From the DisplayChangedListener interface.
     *
     * Overrides WComponentPeer version because Canvases can be created with
     * a non-defulat GraphicsConfiguration, which is no longer valid.
     * Up-called for other windows peer instances (WPanelPeer, WWindowPeer).
     */
    public void displayChanged() {
        clearLocalGC();
        resetTargetGC();
        super.displayChanged();
    }

    /*
     * Reset the graphicsConfiguration member of our target Component.
     * Component.resetGC() is a package-private method, so we have to call it
     * through JNI.
     */
    native void resetTargetGC();

    /*
     * Clears the peer's winGraphicsConfig member.
     * Overridden by WWindowPeer, which shouldn't have a null winGraphicsConfig.
     */
    void clearLocalGC() {
        winGraphicsConfig = null;
    }

    native void create(WComponentPeer parent);

    void initialize() {
        super.initialize();
	Color bg = ((Component)target).getBackground();
	if (bg != null) {
	    setBackground(bg);
	}
    }

    public void paint(Graphics g) {
	Dimension d = ((Component)target).getSize();
        if (g instanceof Graphics2D ||
	    g instanceof sun.awt.Graphics2Delegate) {
	    // background color is setup correctly, so just use clearRect
	    g.clearRect(0, 0, d.width, d.height);
	} else {
	    // emulate clearRect
	    g.setColor(((Component)target).getBackground());
	    g.fillRect(0, 0, d.width, d.height);
	    g.setColor(((Component)target).getForeground());
	}
	super.paint(g);
    }

    public void print(Graphics g) {
	Dimension d = ((Component)target).getSize();
        if (g instanceof Graphics2D ||
	    g instanceof sun.awt.Graphics2Delegate) {
	    // background color is setup correctly, so just use clearRect
	    g.clearRect(0, 0, d.width, d.height);
	} else {
	    // emulate clearRect
	    g.setColor(((Component)target).getBackground());
	    g.fillRect(0, 0, d.width, d.height);
	    g.setColor(((Component)target).getForeground());
	}
	super.print(g);
    }
    
    public boolean shouldClearRectBeforePaint() {
        return (eraseBackgroundDisabled == false);
    }

}
