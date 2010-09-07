/*
 * @(#)WPanelPeer.java	1.23 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.util.Vector;

import sun.awt.SunGraphicsCallback;

class WPanelPeer extends WCanvasPeer implements PanelPeer {

    // ComponentPeer overrides

    public void paint(Graphics g) {
	super.paint(g);
	SunGraphicsCallback.PaintHeavyweightComponentsCallback.getInstance().
	    runComponents(((Container)target).getComponents(), g,
			  SunGraphicsCallback.LIGHTWEIGHTS |
			  SunGraphicsCallback.HEAVYWEIGHTS);
    }
    public void print(Graphics g) {
	super.print(g);
	SunGraphicsCallback.PrintHeavyweightComponentsCallback.getInstance().
	    runComponents(((Container)target).getComponents(), g,
			  SunGraphicsCallback.LIGHTWEIGHTS |
			  SunGraphicsCallback.HEAVYWEIGHTS);
    }

    // ContainerPeer (via PanelPeer) implementation

    public Insets getInsets() {
	return insets_;
    }

    // Toolkit & peer internals

    Insets insets_;

    static {
        initIDs();
    }

    /**
     * Initialize JNI field IDs
     */
    private static native void initIDs();

    WPanelPeer(Component target) {
	super(target);
    }

    void initialize() {
        super.initialize();
    	insets_ = new Insets(0,0,0,0);

	Color c = ((Component)target).getBackground();
	if (c == null) {
            c = WColor.getDefaultColor(WColor.WINDOW_BKGND);
	    ((Component)target).setBackground(c);
	    setBackground(c);
	}
	c = ((Component)target).getForeground();
	if (c == null) {
            c = WColor.getDefaultColor(WColor.WINDOW_TEXT);
	    ((Component)target).setForeground(c);
	    setForeground(c);
	}
    }

    /**
     * DEPRECATED:  Replaced by getInsets().
     */
    public Insets insets() {
	return getInsets();
    }

    /*
     * Called from WCanvasPeer.displayChanged().
     * Since graphicsConfiguration for java.awt.Panels are never set, and there
     * are often many panels in a GUI, we can save some time by overriding this
     * method to do nothing.
     */
    void resetTargetGC() {}

    /**
     * Recursive method that handles the propagation of the displayChanged
     * event into the entire hierarchy of peers.  This ensures that
     * any heavyweights embedded in a lightweight hierarchy (e.g.,
     * a Canvas in the contentPane of a Swing JComponent) will
     * receive the message.
     */
    private void recursiveDisplayChanged(Component c) {
	if (c instanceof Container) {
	    Component children[] = ((Container)c).getComponents();
	    for (int i = 0; i < children.length; ++i) {
		recursiveDisplayChanged(children[i]);
	    }
	}
        ComponentPeer peer = c.getPeer();
	if (peer != null && peer instanceof WComponentPeer) {
            WComponentPeer wPeer = (WComponentPeer)peer;
            wPeer.displayChanged();
	}
    }

    /*
     * From the DisplayChangedListener interface.
     * Often up-called from a WWindowPeer instance.
     * Calls displayChanged() on all heavyweight childrens' peers.
     * Recurses into Container children to ensure all heavyweights
     * get the message.
     */
    public void displayChanged() {
        super.displayChanged();
        Component children[] = ((Container)target).getComponents();
        for (int i = 0; i < children.length; i++) {
	    recursiveDisplayChanged(children[i]);
	}
    }
    private native void pRestack(Object[] peers);
    private void restack(Container cont, Vector peers) {
        for (int i = 0; i < cont.getComponentCount(); i++) {
            Component comp = cont.getComponent(i);
            if (!comp.isLightweight()) {
                if (comp.getPeer() != null) {
                    peers.add(comp.getPeer());
                }
            }
            if (comp.isLightweight() && comp instanceof Container) {
                restack((Container)comp, peers);
            }
        }
    }

    /**
     * @see java.awt.peer.ContainerPeer#restack
     */
    public void restack() {
        Vector peers = new Vector();
        peers.add(this);
        Container cont = (Container)target;
        restack(cont, peers);
        pRestack(peers.toArray());
    }        

    /**
     * @see java.awt.peer.ContainerPeer#isRestackSupported
     */
    public boolean isRestackSupported() {
        return true;
    }
}
