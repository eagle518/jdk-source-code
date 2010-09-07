/*
 * @(#)XPanelPeer.java	1.16 04/02/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import sun.awt.SunGraphicsCallback;

public class XPanelPeer extends XCanvasPeer implements PanelPeer {

    XEmbeddingContainer embedder = null; //new XEmbeddingContainer();
    /**
     * Embeds the given window into container using XEmbed protocol
     */
    public void xembed(long window) {
        if (embedder != null) {
            embedder.add(window);
        }
    }
    XPanelPeer() {}

    XPanelPeer(XCreateWindowParams params) {
        super(params);
    }

    XPanelPeer(Component target) {
	super(target);
    }

    void postInit(XCreateWindowParams params) {
        super.postInit(params);
        if (embedder != null) {
            embedder.install(this);
        }        
    }

    public Insets getInsets() {
	return new Insets(0, 0, 0, 0);
    }

    public void paint(Graphics g) {
        super.paint(g);
        /*      SunGraphicsCallback.PaintHeavyweightComponentsCallback.getInstance().
                runComponents(((Container)target).getComponents(), g,
                SunGraphicsCallback.LIGHTWEIGHTS |
                SunGraphicsCallback.HEAVYWEIGHTS);
        */ }
    public void print(Graphics g) {
        super.print(g);
        SunGraphicsCallback.PrintHeavyweightComponentsCallback.getInstance().
            runComponents(((Container)target).getComponents(), g,
                          SunGraphicsCallback.LIGHTWEIGHTS |
                          SunGraphicsCallback.HEAVYWEIGHTS);
            
    }

    public void setBackground(Color c) {
	Component comp;
	int i;

	Container cont = (Container) target;
	synchronized(target.getTreeLock()) {
	    int n = cont.getComponentCount();
	    for(i=0; i < n; i++) {
	        comp = cont.getComponent(i);
	        ComponentPeer peer = comp.getPeer();
	        if (peer != null) {
		    Color color = comp.getBackground();
                    if (color == null || color.equals(c)) {
			peer.setBackground(c);
		    }
 		}
	    }
	}
  	super.setBackground(c);
    }

    public void setForeground(Color c) {
        setForegroundForHierarchy((Container) target, c);
    }

    private void setForegroundForHierarchy(Container cont, Color c) {
        synchronized(target.getTreeLock()) {
            int n = cont.getComponentCount();
            for(int i=0; i < n; i++) {
                Component comp = cont.getComponent(i);
                Color color = comp.getForeground();
                if (color == null || color.equals(c)) {
                    ComponentPeer cpeer = comp.getPeer();
                    if (cpeer != null) {
                        cpeer.setForeground(c);
                    }
                    if (cpeer instanceof LightweightPeer
                        && comp instanceof Container)
                    {
                        setForegroundForHierarchy((Container) comp, c);
                    }
                }
            }
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
     * event into the entire hierarchy of peers.
     * Unlike on win32, on X we don't worry about handling on-the-fly
     * display settings changes, only windows being dragged across Xinerama
     * screens.  Thus, we only need to tell XCanvasPeers, not all
     * XComponentPeers.
     */
    private void recursiveDisplayChanged(Component c, int screenNum) {
        if (c instanceof Container) {
            Component children[] = ((Container)c).getComponents();
            for (int i = 0; i < children.length; ++i) {
                recursiveDisplayChanged(children[i], screenNum);
            }
        }
        ComponentPeer peer = c.getPeer();
        if (peer != null && peer instanceof XCanvasPeer) {
            XCanvasPeer mPeer = (XCanvasPeer)peer;
            mPeer.displayChanged(screenNum);
        }
    }

    /*
     * Often up-called from a XWindowPeer instance.
     * Calls displayChanged() on all child canvas' peers.
     * Recurses into Container children to ensure all canvases
     * get the message.
     */
    public void displayChanged(int screenNum) {
        // Don't do super call because XWindowPeer has already updated its GC

        Component children[] = ((Container)target).getComponents();
        
        for (int i = 0; i < children.length; i++) {
            recursiveDisplayChanged(children[i], screenNum);
        }
    }

    public void dispose() {
        if (embedder != null) {
            embedder.deinstall();
        }
        super.dispose();
    }

    protected boolean shouldFocusOnClick() {
        // Return false if this container has children so in that case it won't
        // be focused. Return true otherwise.
        return ((Container)target).getComponentCount() == 0;
    }
}

