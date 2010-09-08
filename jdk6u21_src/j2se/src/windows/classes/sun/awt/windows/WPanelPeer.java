/*
 * @(#)WPanelPeer.java	1.28 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

    /**
     * Recursive method that handles the propagation of the displayChanged
     * event into the entire hierarchy of peers.  This ensures that
     * any heavyweights embedded in a lightweight hierarchy (e.g.,
     * a Canvas in the contentPane of a Swing JComponent) will
     * receive the message.
     */
    private void recursiveDisplayChanged(Component c) {
        // 5085626:
        // recursiveDisplayChanged() was being called multiple times for
        // WPanelPeers.  It would be called by the panelPeer's parent from
        // WPanelPeer.displayChanged() (which would recurse the panelPeer's
        // children), and then again during the panelPeer's call to
        // displayChanged() (recursing the panelPeer's children again):
        //
        // parentPanelPeer.displayChanged()
        // +-parentPanelPeer.recursiveDisplayChanged(panelPeer's target)
        //   +-recursiveDisplayChanged(panelPeer's target's children)
        //   +-panelPeer.displayChanged()
        //     +-recursiveDisplayChanged(panelPeer's target children)  *again!*
        //
        // The fix is to only allow WPanelPeers to recurse their children from
        // displayChanged().  Though this means that recursiveDisplayChanged()
        // is not truly "recursive", it does ensure that displayChanged() is
        // called for all WPanelPeers.
        //
        // Note that it is important to recurse through the Component hierarchy
        // and not just through the peer hierarchy.  This is to ensure that
        // heavyweights buried inside lightweight containers (which don't have
        // WPanelPeers) are updated after a display change.  See also 4452373.

        ComponentPeer peer = c.getPeer();
        if (c instanceof Container && !(peer instanceof WPanelPeer)) {
            Component children[] = ((Container)c).getComponents();
            for (int i = 0; i < children.length; ++i) {
                recursiveDisplayChanged(children[i]);
            }
        }
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
