/*
 * @(#)WPrintDialogPeer.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.*;
import java.awt.dnd.DropTarget;
import sun.awt.AppContext;

public class WPrintDialogPeer extends WWindowPeer {

    static {
        initIDs();
    }

    private WComponentPeer parent;

    WPrintDialogPeer(WPrintDialog target) {
        super(target);
    }

    void create(WComponentPeer parent) {
        this.parent = parent;
    }

    protected void disposeImpl() {
	WToolkit.targetDisposedPeer(target, this);
    }

    private native boolean _show();

    public void show() {
        new Thread(new Runnable() {
            public void run() {
                ((WPrintDialog)target).setRetVal(_show());
		((WPrintDialog)target).hide();
            }
        }).start();
    }

    // unused methods.
    void initialize() {}
    public void toFront() {}
    public void toBack() {}
    public void setResizable(boolean resizable) {}
    public void hide() {}
    public void enable() {}
    public void disable() {}
    public void reshape(int x, int y, int width, int height) {}
    public boolean handleEvent(Event e) { return false; }
    public void setForeground(Color c) {}
    public void setBackground(Color c) {}
    public void setFont(Font f) {}
    public boolean requestFocus(boolean temporary,
				boolean focusedWindowChangeAllowed) {
	return false;
    }
    void start() {}
    void invalidate(int x, int y, int width, int height) {}
    public void addDropTarget(DropTarget dt) {}
    public void removeDropTarget(DropTarget dt) {}

    /**
     * Initialize JNI field and method ids
     */
    private static native void initIDs();

    /**
     * WPrintDialogPeer doesn't have native pData so we don't do restack on it
     * @see java.awt.peer.ContainerPeer#restack
     */
    public void restack() {
    }    

    /**
     * @see java.awt.peer.ContainerPeer#isRestackSupported
     */
    public boolean isRestackSupported() {
        return false;
    }
}
