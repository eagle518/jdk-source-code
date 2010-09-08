/*
 * @(#)WCanvasPeer.java	1.37 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.lang.ref.WeakReference;
import java.lang.reflect.Method;
import sun.awt.AWTAccessor;
import sun.awt.SunToolkit;
import sun.awt.Win32GraphicsDevice;
import sun.awt.PaintEventDispatcher;

class WCanvasPeer extends WComponentPeer implements CanvasPeer {

    private boolean eraseBackground;

    Method resetGCMethod;

    // Toolkit & peer internals

    WCanvasPeer(Component target) {
        super(target);
        if (AWTAccessor.getComponentAccessor().getBackgroundEraseDisabled(target)) {
            disableBackgroundErase();
        }
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
     * through reflection.
     */
    public void resetTargetGC() {
        AWTAccessor.getComponentAccessor().resetGC((Component)target);
    }

    /*
     * Clears the peer's winGraphicsConfig member.
     * Overridden by WWindowPeer, which shouldn't have a null winGraphicsConfig.
     */
    void clearLocalGC() {
        winGraphicsConfig = null;
    }

    native void create(WComponentPeer parent);

    void initialize() {
        eraseBackground = !SunToolkit.getSunAwtNoerasebackground();
        boolean eraseBackgroundOnResize = SunToolkit.getSunAwtErasebackgroundonresize();
        // Optimization: the default value in the native code is true, so we 
        // call setNativeBackgroundErase only when the value changes to false
        if (!PaintEventDispatcher.getPaintEventDispatcher().
                shouldDoNativeBackgroundErase((Component)target)) {
            eraseBackground = false;
        }
        setNativeBackgroundErase(eraseBackground, eraseBackgroundOnResize);
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

    public boolean shouldClearRectBeforePaint() {
        return eraseBackground;
    }

    /*
     * Disables background erasing for this canvas, both for resizing
     * and not-resizing repaints.
     */
    void disableBackgroundErase() {
        eraseBackground = false;
        setNativeBackgroundErase(false, false);
    }

    /*
     * Sets background erasing flags at the native level. If {@code
     * doErase} is set to {@code true}, canvas background is erased on
     * every repaint. If {@code doErase} is {@code false} and {@code
     * doEraseOnResize} is {@code true}, then background is only erased
     * on resizing repaints. If both {@code doErase} and {@code
     * doEraseOnResize} are false, then background is never erased.
     */
    private native void setNativeBackgroundErase(boolean doErase,
                                                 boolean doEraseOnResize);
}
