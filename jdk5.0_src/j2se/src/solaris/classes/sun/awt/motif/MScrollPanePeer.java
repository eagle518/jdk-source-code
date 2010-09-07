/*
 * @(#)MScrollPanePeer.java	1.39 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.motif;

import java.awt.*;
import java.awt.event.AdjustmentEvent;
import java.awt.peer.ScrollPanePeer;
import sun.awt.DebugHelper;
import sun.awt.PeerEvent;

class MScrollPanePeer extends MPanelPeer implements ScrollPanePeer {
    private static final DebugHelper dbg
	= DebugHelper.create(MScrollPanePeer.class);

    final static int UNIT_INCREMENT = 0;
    final static int BLOCK_INCREMENT = 1;

    boolean ignore;

    native void create(MComponentPeer parent);

    static {
        initIDs();
    }

    /**
     * Initialize JNI field and method IDs
     */
    private static native void initIDs();

    MScrollPanePeer(Component target) {
	init(target);
        scrollPaneInit();
    }

    MScrollPanePeer(Component target, Object arg) {
	init(target, arg);
        scrollPaneInit();
    }

    void scrollPaneInit() {
	ignore = false;
        ScrollPane sp = (ScrollPane)target;
	Adjustable vadj, hadj;
	if ((vadj = sp.getVAdjustable()) != null) {
            pSetIncrement(Adjustable.VERTICAL, UNIT_INCREMENT, vadj.getUnitIncrement());
	}
	if ((hadj = sp.getHAdjustable()) != null) {
	    pSetIncrement(Adjustable.HORIZONTAL, UNIT_INCREMENT, hadj.getUnitIncrement());
	}
	super.pSetScrollbarBackground(sp.getBackground());
    }

    public void setScrollChild(MComponentPeer child) {
        pSetScrollChild(child);
    }

    public void setBackground(Color c) {
        super.setBackground(c);
	pSetScrollbarBackground(c);
    }

    public void setForeground(Color c) {
        super.setForeground(c);
	pSetInnerForeground(c);
    }

    native void pSetScrollChild(MComponentPeer child);
    native void pSetIncrement(int orient, int type, int incr);
    native int pGetScrollbarSpace(int orient);
    native int pGetBlockIncrement(int orient);
    native Insets pInsets(int w, int h, int childw, int childh);
    native int pGetShadow();

    public int getHScrollbarHeight() {
        ScrollPane sp = (ScrollPane)target;
        if (sp.getScrollbarDisplayPolicy() == ScrollPane.SCROLLBARS_NEVER) {
	    return 0;
	} else {
	    return pGetScrollbarSpace(Adjustable.HORIZONTAL);
	}
    }

    public int getVScrollbarWidth() {
        ScrollPane sp = (ScrollPane)target;
        if (sp.getScrollbarDisplayPolicy() == ScrollPane.SCROLLBARS_NEVER) {
	    return 0;
	} else {
	    return pGetScrollbarSpace(Adjustable.VERTICAL);
	}
    }

    public Insets insets() {
	ScrollPane sp = (ScrollPane)target;
	Dimension d = sp.size();
	Dimension cd;
        Component c = getScrollChild();
        if (c != null) {
	    cd = c.size();
	} else {
	    cd = new Dimension(0, 0);
	}
	return pInsets(d.width, d.height, cd.width, cd.height);
    }

    public void setUnitIncrement(Adjustable adj, int u) {
        ScrollPane sp = (ScrollPane)target;
        if (sp.getScrollbarDisplayPolicy() != ScrollPane.SCROLLBARS_NEVER) {
	    pSetIncrement(adj.getOrientation(), UNIT_INCREMENT, u);
	}
    }

    public void setValue(Adjustable adj, int v) {
	if (! ignore) {
	    Point p;
	    Component c = getScrollChild();
            if (c == null) {
                return;
            }
	    p = c.getLocation();
	    switch(adj.getOrientation()) {
	    case Adjustable.VERTICAL:
		setScrollPosition(-(p.x), v);
		break;
	    case Adjustable.HORIZONTAL:
		setScrollPosition(v, -(p.y));
		break;
	    }
	}
    }

    public native void setScrollPosition(int x, int y);

    public void childResized(int w, int h) {
	// REMIND AIM:  May need to revisit this...
        if (((ScrollPane)target).getScrollbarDisplayPolicy() != ScrollPane.SCROLLBARS_NEVER) {
            ScrollPane sp = (ScrollPane)target;
	    Adjustable vAdj = sp.getVAdjustable();
	    Adjustable hAdj = sp.getHAdjustable();
            pSetIncrement(Scrollbar.VERTICAL, UNIT_INCREMENT, vAdj.getUnitIncrement());
            pSetIncrement(Scrollbar.HORIZONTAL, UNIT_INCREMENT, hAdj.getUnitIncrement());
            pSetIncrement(Scrollbar.VERTICAL, BLOCK_INCREMENT, vAdj.getBlockIncrement());
            pSetIncrement(Scrollbar.HORIZONTAL, BLOCK_INCREMENT, hAdj.getBlockIncrement());
        }

    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    private void postScrollEvent(int orient, int type,
				 int pos, boolean isAdjusting)
    {
	Runnable adjustor = new Adjustor(orient, type, pos, isAdjusting);
	MToolkit.executeOnEventHandlerThread(new ScrollEvent(target, adjustor));
    }

    /**
     * This is used to change the adjustable on dispatch thread to
     * represent a change made in the native scrollbar.  Since the
     * change was reflected immediately at the native level,
     * notification from the adjustable is temporarily ignored.
     */
    class ScrollEvent extends PeerEvent {
	ScrollEvent(Object source, Runnable runnable) {
	    super(source, runnable, 0L);
	}

	public PeerEvent coalesceEvents(PeerEvent newEvent) {
	    if (dbg.on) dbg.println("ScrollEvent coalesced "+newEvent);
	    if (newEvent instanceof ScrollEvent) {
		return newEvent;
	    }
	    return null;
	}
    }

    native void setTypedValue(ScrollPaneAdjustable adjustable, int value, int type);

    /**
     * Runnable for the ScrollEvent that performs the adjustment.
     */
    class Adjustor implements Runnable {
	int orient;		// selects scrollbar
	int type;		// adjustment type
	int pos;		// new position (only used for absolute)
	boolean isAdjusting;	// isAdjusting status

	Adjustor(int orient, int type, int pos, boolean isAdjusting) {
	    this.orient = orient;
	    this.type = type;
	    this.pos = pos;
	    this.isAdjusting = isAdjusting;
	}

	public void run() {
	    ScrollPane sp = (ScrollPane)MScrollPanePeer.this.target;
	    ScrollPaneAdjustable adj = null;

	    // ScrollPaneAdjustable made public in 1.4, but
	    // get[HV]Adjustable can't be declared to return
	    // ScrollPaneAdjustable because it would break backward
	    // compatibility -- hence the cast

	    if (orient == Adjustable.VERTICAL) {
		adj = (ScrollPaneAdjustable)sp.getVAdjustable();
	    } else if (orient == Adjustable.HORIZONTAL) {
		adj = (ScrollPaneAdjustable)sp.getHAdjustable();
	    } else {
		if (dbg.on) dbg.assertion(false);
	    }

	    if (adj == null) {
		return;
	    }

	    int newpos = adj.getValue();
	    switch (type) {
	      case AdjustmentEvent.UNIT_DECREMENT:
		  newpos -= adj.getUnitIncrement();
		  break;
	      case AdjustmentEvent.UNIT_INCREMENT:
		  newpos += adj.getUnitIncrement();
		  break;
	      case AdjustmentEvent.BLOCK_DECREMENT:
		  newpos -= adj.getBlockIncrement();
		  break;
	      case AdjustmentEvent.BLOCK_INCREMENT:
		  newpos += adj.getBlockIncrement();
		  break;
	      case AdjustmentEvent.TRACK:
		  newpos = this.pos;
		  break;
	      default:
		  if (dbg.on) dbg.assertion(false);
		  return;
	    }

	    // keep scroll position in acceptable range
	    newpos = Math.max(adj.getMinimum(), newpos);
	    newpos = Math.min(adj.getMaximum(), newpos);

	    // set value; this will synchronously fire an AdjustmentEvent
	    try {
		MScrollPanePeer.this.ignore = true;
		adj.setValueIsAdjusting(isAdjusting);

                // Fix for 4075484 - consider type information when creating AdjustmentEvent
                // We can't just call adj.setValue() because it creates AdjustmentEvent with type=TRACK
                // Instead, we call private method setTypedValue of ScrollPaneAdjustable. 
                // Because ScrollPaneAdjustable is in another package we should call it through native code.                
                setTypedValue(adj, newpos, type);
	    } finally {
		MScrollPanePeer.this.ignore = false;
	    }
	}
    } // class Adjustor


    private Component getScrollChild() {
        ScrollPane sp = (ScrollPane)target;
        Component child = null;
        try {
            child = sp.getComponent(0);
        } catch (ArrayIndexOutOfBoundsException e) {
            // do nothing.  in this case we return null
        }
        return child;
    }

    final static int 	MARGIN = 1;
    final static int 	SCROLLBAR = 16;
    int hsbSpace;
    int vsbSpace;
    int vval;
    int hval;
    int vmax;
    int hmax;
    /*
     * Print the native component by rendering the Motif look ourselves.
     * ToDo(aim): needs to query native motif for more accurate size and
     * color information.
     */
    public void print(Graphics g) {
        ScrollPane sp = (ScrollPane)target;
	Dimension d = sp.size();
	Color bg = sp.getBackground();
	Color fg = sp.getForeground();
	Point p = sp.getScrollPosition();
	Component c = getScrollChild();
	Dimension cd;
        if (c != null) {
            cd = c.size();
        } else {
            cd = new Dimension(0, 0);
        }
	int sbDisplay = sp.getScrollbarDisplayPolicy();
	int vvis, hvis, vmin, hmin, vmax, hmax, vval, hval;

	switch (sbDisplay) {
	  case ScrollPane.SCROLLBARS_NEVER:
	    hsbSpace = vsbSpace = 0;
	    break;
	  case ScrollPane.SCROLLBARS_ALWAYS:
	    hsbSpace = vsbSpace = SCROLLBAR;
	    break;
	  case ScrollPane.SCROLLBARS_AS_NEEDED:
	    hsbSpace = (cd.width <= (d.width - 2*MARGIN)? 0 : SCROLLBAR);
	    vsbSpace = (cd.height <= (d.height - 2*MARGIN)? 0 : SCROLLBAR);

	    if (hsbSpace == 0 && vsbSpace != 0) {
		hsbSpace = (cd.width <= (d.width - SCROLLBAR - 2*MARGIN)? 0 : SCROLLBAR);
	    }
	    if (vsbSpace == 0 && hsbSpace != 0) {
		vsbSpace = (cd.height <= (d.height - SCROLLBAR - 2*MARGIN)? 0 : SCROLLBAR);
	    }
	}

        vvis = hvis = vmin = hmin = vmax = hmax = vval = hval = 0;

	if (vsbSpace > 0) {
	    vmin = 0;
	    vvis = d.height - (2*MARGIN) - hsbSpace;
	    vmax = Math.max(cd.height - vvis, 0);
	    vval = p.y;
	}
	if (hsbSpace > 0) {
	    hmin = 0;
	    hvis = d.width - (2*MARGIN) - vsbSpace;
	    hmax = Math.max(cd.width - hvis, 0);
	    hval = p.x;
	}

	// need to be careful to add the margins back in here because
	// we're drawing the margin border, after all!
	int w = d.width - vsbSpace;
	int h = d.height - hsbSpace;

	g.setColor(bg);
	g.fillRect(0, 0, d.width, d.height);

	if (hsbSpace > 0) {
	    int sbw = d.width - vsbSpace;
	    g.fillRect(1, d.height - SCROLLBAR - 3, sbw - 1, SCROLLBAR - 3);
	    Graphics ng = g.create();
	    try {
	        ng.translate(0, d.height - (SCROLLBAR - 2));
	        drawScrollbar(ng, bg, SCROLLBAR - 2, sbw,
			       hmin, hmax, hval, hvis, true);
	    } finally {
	        ng.dispose();
	    }
	}
	if (vsbSpace > 0) {
	    int sbh = d.height - hsbSpace;
	    g.fillRect(d.width - SCROLLBAR - 3, 1, SCROLLBAR - 3, sbh - 1);
	    Graphics ng = g.create();
	    try {
	        ng.translate(d.width - (SCROLLBAR - 2), 0);
	        drawScrollbar(ng, bg, SCROLLBAR - 2, sbh,
			       vmin, vmax, vval, vvis, false);
	    } finally {
	        ng.dispose();
	    }
	}

	draw3DRect(g, bg, 0, 0, w - 1, h - 1, false);

	target.print(g);
	sp.printComponents(g);
    }

    /**
     * @see ContainerPeer#restack
     */
    public void restack() {
        // Since ScrollPane can only have one child its restacking does nothing.
        // Also, it is dangerous, since SP child is actually not a child of SP widget
        // but the child of SP content widget.
    }
}

