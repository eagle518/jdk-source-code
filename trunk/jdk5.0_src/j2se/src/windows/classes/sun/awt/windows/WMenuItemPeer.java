/*
 * @(#)WMenuItemPeer.java	1.37 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.util.ResourceBundle;
import java.util.MissingResourceException;
import java.awt.*;
import java.awt.peer.*;
import java.awt.event.ActionEvent;
import java.security.AccessController;
import java.security.PrivilegedAction;
import sun.awt.AppContext;

class WMenuItemPeer extends WObjectPeer implements MenuItemPeer {

    static {
        initIDs();
    }

    String shortcutLabel;

    // MenuItemPeer implementation

    private synchronized native void _dispose();
    protected void disposeImpl() {
        WToolkit.targetDisposedPeer(target, this);
	_dispose();
    }

    public void setEnabled(boolean b) {
	enable(b);
    }

    /**
     * DEPRECATED:  Replaced by setEnabled(boolean).
     */
    public void enable() {
        enable(true);
    }

    /**
     * DEPRECATED:  Replaced by setEnabled(boolean).
     */
    public void disable() {
        enable(false);
    }

    public void setLabel(String label) {
        MenuShortcut sc = ((MenuItem)target).getShortcut();
        shortcutLabel = (sc != null) ? sc.toString() : null;
        _setLabel(label);
    }
    public native void _setLabel(String label);

    // Toolkit & peer internals

    boolean	isCheckbox = false;

    protected WMenuItemPeer() {
    }
    WMenuItemPeer(MenuItem target) {
	this.target = target;
	WMenuPeer parent = (WMenuPeer) WToolkit.targetToPeer(target.getParent());
	create(parent);
        MenuShortcut sc = ((MenuItem)target).getShortcut();
        if (sc != null) {
            shortcutLabel = sc.toString();
        }
    }

    /*
     * Post an event. Queue it for execution by the callback thread.
     */
    void postEvent(AWTEvent event) {
        WToolkit.postEvent(WToolkit.targetToAppContext(target), event);
    }

    native void create(WMenuPeer parent);

    native void enable(boolean e);

    // native callbacks

    void handleAction(final long when, final int modifiers) {
	WToolkit.executeOnEventHandlerThread(target, new Runnable() {
	    public void run() {
		postEvent(new ActionEvent(target, ActionEvent.ACTION_PERFORMED,
					  ((MenuItem)target).
                                              getActionCommand(), when,
                                          modifiers));
	    }
	});
    }

    private static Font defaultMenuFont;

    static {
        defaultMenuFont = (Font) AccessController.doPrivileged(
            new PrivilegedAction() {
                public Object run() {
                    try {
                        ResourceBundle rb = ResourceBundle.getBundle("sun.awt.windows.awtLocalization");
                        return Font.decode(rb.getString("menuFont"));
                    } catch (MissingResourceException e) {
                        System.out.println(e.getMessage());
                        System.out.println("Using default MenuItem font\n");
                        return new Font("SanSerif", Font.PLAIN, 11);
                    }
                }
            });
    }

    static Font getDefaultFont() {
        return defaultMenuFont;
    }

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();

    // Needed for MenuComponentPeer.
    public void setFont(Font f) {
    }
}
