/*
 * @(#)WComponentPeer.java	1.153 04/06/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.windows;

import java.awt.*;
import java.awt.peer.*;
import java.awt.image.WritableRaster;
import java.awt.image.VolatileImage;
import sun.awt.RepaintArea;
import sun.awt.AppContext;
import sun.awt.image.OffScreenImage;
import sun.awt.image.SunVolatileImage;
import sun.awt.image.ToolkitImage;
import java.awt.image.BufferedImage;
import java.awt.image.ImageProducer;
import java.awt.image.ImageObserver;
import java.awt.image.ColorModel;
import java.awt.image.DirectColorModel;
import java.awt.event.PaintEvent;
import sun.awt.Win32GraphicsConfig;
import sun.awt.Win32GraphicsDevice;
import sun.java2d.InvalidPipeException;
import sun.java2d.SunGraphics2D;
import sun.java2d.SurfaceData;
import sun.awt.DisplayChangedListener;

import java.awt.dnd.DropTarget;
import java.awt.dnd.peer.DropTargetPeer;

import sun.awt.DebugHelper;

public abstract class WComponentPeer extends WObjectPeer 
    implements ComponentPeer, DropTargetPeer, DisplayChangedListener
{

    private static final DebugHelper dbg = DebugHelper.create(WComponentPeer.class);

    static {
        wheelInit();
    }

    // Only actually does stuff if running on 95
    native static void wheelInit();

    // ComponentPeer implementation
    SurfaceData surfaceData;

    private RepaintArea paintArea;

    protected Win32GraphicsConfig winGraphicsConfig;

    boolean isLayouting = false;
    boolean paintPending = false;
    int	    oldWidth = -1;
    int	    oldHeight = -1;
    private int numBackBuffers = 0;
    private VolatileImage backBuffer = null;

    public native boolean isObscured();
    public boolean canDetermineObscurity() { return true; }

    // DropTarget support

    int nDropTargets;
    long nativeDropTargetContext; // native pointer

    public synchronized native void pShow();
    public synchronized native void hide();
    public synchronized native void enable();
    public synchronized native void disable();

    /* New 1.1 API */
    public native Point getLocationOnScreen();

    /* New 1.1 API */
    public void setVisible(boolean b) {
	if (b) {
    	    show();
    	} else {
    	    hide();
    	}
    }

    public void show() {
	Dimension s = ((Component)target).getSize();
	oldHeight = s.height;
	oldWidth = s.width;
	pShow();
    }    

    /* New 1.1 API */
    public void setEnabled(boolean b) {
    	if (b) {
	    enable();
	} else {
	    disable();
	}
    }

    public int serialNum = 0;

    private native void reshapeNoCheck(int x, int y, int width, int height);

    /* New 1.1 API */
    public void setBounds(int x, int y, int width, int height, int op) {
        // Should set paintPending before reahape to prevent
        // thread race between paint events
        // Native components do redraw after resize
        paintPending = (width != oldWidth) || (height != oldHeight);

        if ( (op & NO_EMBEDDED_CHECK) != 0 ) {
            reshapeNoCheck(x, y, width, height);
        } else {
            reshape(x, y, width, height);
        }
        if ((width != oldWidth) || (height != oldHeight)) {
            // Only recreate surfaceData if this setBounds is called
            // for a resize; a simple move should not trigger a recreation
            try {
                replaceSurfaceData();
            } catch (InvalidPipeException e) {
                // REMIND : what do we do if our surface creation failed?
            }
            oldWidth = width;
            oldHeight = height;
        }

        serialNum++;
    }

    /*
     * Called from native code (on Toolkit thread) in order to
     * dynamically layout the Container during resizing
     */
    void dynamicallyLayoutContainer() {
        // If we got the WM_SIZING, this must be a Container, right?
        // In fact, it must be the top-level Container.
        if (dbg.on) {
            Container parent = WToolkit.getNativeContainer((Component)target);
            dbg.assertion(parent == null);
        }
        final Container cont = (Container)target;

        WToolkit.executeOnEventHandlerThread(cont, new Runnable() {
            public void run() {
                // Discarding old paint events doesn't seem to be necessary.
                cont.invalidate();
                cont.validate();
                // Forcing a paint here doesn't seem to be necessary.
                // paintDamagedAreaImmediately();
            }
        });
    }

    /*
     * Paints any portion of the component that needs updating
     * before the call returns (similar to the Win32 API UpdateWindow)
     */
    void paintDamagedAreaImmediately() {
	// force Windows to send any pending WM_PAINT events so
	// the damage area is updated on the Java side
	updateWindow();
	// make sure paint events are transferred to main event queue
	// for coalescing
	WToolkit.getWToolkit().flushPendingEvents();
	// paint the damaged area
	paintArea.paint(target, shouldClearRectBeforePaint());
    }

    native synchronized void updateWindow();

    public void paint(Graphics g) {
        ((Component)target).paint(g);
    }

    public void repaint(long tm, int x, int y, int width, int height) {
    }

    private static final double BANDING_DIVISOR = 4.0;
    private native int[] createPrintedPixels(int srcX, int srcY,
					     int srcW, int srcH);
    public void print(Graphics g) {

        Component comp = (Component)target;

        // To conserve memory usage, we will band the image.

        int totalW = comp.getWidth();
        int totalH = comp.getHeight();

        int hInc = (int)(totalH / BANDING_DIVISOR);
        if (hInc == 0) {
            hInc = totalH;
        }

        for (int startY = 0; startY < totalH; startY += hInc) {
            int endY = startY + hInc - 1;
            if (endY >= totalH) {
                endY = totalH - 1;
            }
            int h = endY - startY + 1;

            int[] pix = createPrintedPixels(0, startY, totalW, h);
            if (pix != null) {
                BufferedImage bim = new BufferedImage(totalW, h,
                                              BufferedImage.TYPE_INT_RGB);
                bim.setRGB(0, 0, totalW, h, pix, 0, totalW);
                g.drawImage(bim, 0, startY, null);
                bim.flush();
            }
        }

        comp.print(g);
    }

    public void coalescePaintEvent(PaintEvent e) {
        Rectangle r = e.getUpdateRect();
	paintArea.add(r, e.getID());

        if (dbg.on) {
	    switch(e.getID()) {
	    case PaintEvent.UPDATE:       
                dbg.println("WCP coalescePaintEvent : UPDATE : add : x = " +
		    r.x + ", y = " + r.y + ", width = " + r.width + ",height = " + r.height);
		return;
	    case PaintEvent.PAINT:
                dbg.println("WCP coalescePaintEvent : PAINT : add : x = " +
                    r.x + ", y = " + r.y + ", width = " + r.width + ",height = " + r.height);
		return;
	    }
	}
    }

    public synchronized native void reshape(int x, int y, int width, int height);

    native void nativeHandleEvent(AWTEvent e);

    public void handleEvent(AWTEvent e) {
        int id = e.getID();

        switch(id) {
	case PaintEvent.PAINT:
	    // Got native painting
	    paintPending = false;
	    // Fallthrough to next statement
	case PaintEvent.UPDATE:	  
	    // Skip all painting while layouting and all UPDATEs
	    // while waiting for native paint
	    if (!isLayouting && ! paintPending) {
		paintArea.paint(target,shouldClearRectBeforePaint());
	    }
	    return;
	default:
            break;
        }

        // Call the native code
        nativeHandleEvent(e);
    }

    public Dimension getMinimumSize() {
	return ((Component)target).getSize();
    }

    public Dimension getPreferredSize() {
	return getMinimumSize();
    }

    // Do nothing for heavyweight implementation
    public void layout() {}

    public Rectangle getBounds() {
    	return ((Component)target).getBounds();
    }

    public boolean isFocusable() {
	return false;
    }

    /*
     * Return the GraphicsConfiguration associated with this peer, either
     * the locally stored winGraphicsConfig, or that of the target Component.
     */
    public GraphicsConfiguration getGraphicsConfiguration() {
        if (winGraphicsConfig != null) {
            return winGraphicsConfig;
        }
        else {
            // we don't need a treelock here, since
            // Component.getGraphicsConfiguration() gets it itself.
            return ((Component)target).getGraphicsConfiguration();
        }
    }

    public SurfaceData getSurfaceData() {
	return surfaceData;
    }

    /**
     * Creates new surfaceData object and invalidates the previous
     * surfaceData object.
     * Replacing the surface data should never lock on any resources which are
     * required by other threads which may have them and may require
     * the tree-lock.
     * This is a degenerate version of replaceSurfaceData(numBackBuffers), so
     * just call that version with our current numBackBuffers.
     */
    public void replaceSurfaceData() {
	replaceSurfaceData(this.numBackBuffers);
    }

    /**
     * Multi-buffer version of replaceSurfaceData.  This version is called
     * by createBuffers(), which needs to acquire the same locks in the same
     * order, but also needs to perform additional functions inside the
     * locks.
     */
    public void replaceSurfaceData(int newNumBackBuffers) {
        synchronized(((Component)target).getTreeLock()) {
            synchronized(this) {
                if (pData == 0) {
                    return;
                }
		numBackBuffers = newNumBackBuffers;
                SurfaceData oldData = surfaceData;
                Win32GraphicsConfig gc =
                    (Win32GraphicsConfig)getGraphicsConfiguration();
                surfaceData = gc.createSurfaceData(this, numBackBuffers);
                if (oldData != null) {
                    oldData.invalidate();
                }
                if (numBackBuffers > 0) {
                    backBuffer = gc.createBackBuffer(this);
                } else {
                    backBuffer = null;
                }
            }
        }
    }

    public void replaceSurfaceDataLater() {
        EventQueue.invokeLater(new Runnable() {
            public void run() {
                // Shouldn't do anything if object is disposed in meanwhile
                // No need for sync as disposeAction in Window is performed 
                // on EDT
                if (!isDisposed()) { 
                    try {
                        replaceSurfaceData();
                    } catch (InvalidPipeException e) {
                    // REMIND : what do we do if our surface creation failed?
                    }
                }
            }
        });
    }

    /**
     * From the DisplayChangedListener interface.
     *
     * Called after a change in the display mode.  This event
     * triggers replacing the surfaceData object (since that object
     * reflects the current display depth information, which has
     * just changed).
     */
    public void displayChanged() {
        try {
            replaceSurfaceData();
        } catch (InvalidPipeException e) {
            // REMIND : what do we do if our surface creation failed?
        }
    }
    
    /**
     * Part of the DisplayChangedListener interface: components
     * do not need to react to this event
     */
    public void paletteChanged() {
    }

    //This will return null for Components not yet added to a Container
    public ColorModel getColorModel() {
        GraphicsConfiguration gc = getGraphicsConfiguration();
        if (gc != null) {
            return gc.getColorModel();
        }
        else {
            return null;
        }
    }

    //This will return null for Components not yet added to a Container
    public ColorModel getDeviceColorModel() {
        Win32GraphicsConfig gc = 
	    (Win32GraphicsConfig)getGraphicsConfiguration();
        if (gc != null) {
            return gc.getDeviceColorModel();
        }
        else {
            return null;
        }
    }

    //Returns null for Components not yet added to a Container
    public ColorModel getColorModel(int transparency) {
//	return WToolkit.config.getColorModel(transparency);
        GraphicsConfiguration gc = getGraphicsConfiguration();
        if (gc != null) {
            return gc.getColorModel(transparency);
        }
        else {
            return null;
        }
    }
    public java.awt.Toolkit getToolkit() {
	return Toolkit.getDefaultToolkit();
    }

    // fallback default font object
    final static Font defaultFont = new Font("Dialog", Font.PLAIN, 12);

    public synchronized Graphics getGraphics() {
	if (!isDisposed()) {
	    Component target = (Component) this.target;

            /* Fix for bug 4746122. Color and Font shouldn't be null */
            Color bgColor = target.getBackground();
            if (bgColor == null) {
                bgColor = SystemColor.window;
            }
            Color fgColor = target.getForeground();
            if (fgColor == null) {
                fgColor = SystemColor.windowText;
            }
            Font font = target.getFont(); 
            if (font == null) {
                font = defaultFont;
            }
            return new SunGraphics2D(surfaceData, fgColor, bgColor, font);
	}

	return null;
    }
    public FontMetrics getFontMetrics(Font font) {
	return WFontMetrics.getFontMetrics(font);
    }

    private synchronized native void _dispose();
    protected void disposeImpl() {
        SurfaceData oldData = surfaceData;
        surfaceData = null;
        oldData.invalidate();
        // remove from updater before calling targetDisposedPeer
	WToolkit.targetDisposedPeer(target, this);
	_dispose();
    }

    public synchronized void setForeground(Color c) {_setForeground(c.getRGB());}
    public synchronized void setBackground(Color c) {_setBackground(c.getRGB());}

    public native void _setForeground(int rgb);
    public native void _setBackground(int rgb);

    public synchronized native void setFont(Font f);
    public final void updateCursorImmediately() {
	WGlobalCursorManager.getCursorManager().updateCursorImmediately();
    }
    
    native static boolean processSynchronousLightweightTransfer(Component heavyweight, Component descendant,
                                                                boolean temporary, boolean focusedWindowChangeAllowed,
                                                                long time);
    public boolean requestFocus
        (Component lightweightChild, boolean temporary,
         boolean focusedWindowChangeAllowed, long time) {
        if (processSynchronousLightweightTransfer((Component)target, lightweightChild, temporary, 
                                                                      focusedWindowChangeAllowed, time)) {
            return true;
        } else {
            return _requestFocus(lightweightChild, temporary, focusedWindowChangeAllowed, time);
        }
    }
    public native boolean _requestFocus
        (Component lightweightChild, boolean temporary,
         boolean focusedWindowChangeAllowed, long time);

    public Image createImage(ImageProducer producer) {
	return new ToolkitImage(producer);
    }

    public Image createImage(int width, int height) {
        Win32GraphicsConfig gc =
            (Win32GraphicsConfig)getGraphicsConfiguration();
        return gc.createAcceleratedImage((Component)target, width, height);
    }

    public VolatileImage createVolatileImage(int width, int height) {
	return new SunVolatileImage((Component)target, width, height);
    }

    public boolean prepareImage(Image img, int w, int h, ImageObserver o) {
	return getToolkit().prepareImage(img, w, h, o);
    }

    public int checkImage(Image img, int w, int h, ImageObserver o) {
	return getToolkit().checkImage(img, w, h, o);
    }

    // Object overrides

    public String toString() {
	return getClass().getName() + "[" + target + "]";
    }

    // Toolkit & peer internals

    private int updateX1, updateY1, updateX2, updateY2;

    WComponentPeer(Component target) {
	this.target = target;
        this.paintArea = new RepaintArea();
	Container parent = WToolkit.getNativeContainer(target);
	WComponentPeer parentPeer = (WComponentPeer) WToolkit.targetToPeer(parent);
	create(parentPeer);
        this.winGraphicsConfig =
            (Win32GraphicsConfig)getGraphicsConfiguration();
	this.surfaceData =
            winGraphicsConfig.createSurfaceData(this, numBackBuffers);
	initialize();
	start();  // Initialize enable/disable state, turn on callbacks
    }
    abstract void create(WComponentPeer parent);

    synchronized native void start();

    void initialize() {
	if (((Component)target).isVisible()) {
	    show();  // the wnd starts hidden
	}
	Color fg = ((Component)target).getForeground();
	if (fg != null) {
	    setForeground(fg);
	}
        // Set background color in C++, to avoid inheriting a parent's color.
	Font  f = ((Component)target).getFont();
	if (f != null) {
	    setFont(f);
	}
	if (! ((Component)target).isEnabled()) {
	    disable();
	}
	Rectangle r = ((Component)target).getBounds();
	setBounds(r.x, r.y, r.width, r.height, SET_BOUNDS);
    }

    // Callbacks for window-system events to the frame

    // Invoke a update() method call on the target
    void handleRepaint(int x, int y, int w, int h) {
        // Repaints are posted from updateClient now...
    }

    // Invoke a paint() method call on the target, after clearing the
    // damaged area.
    void handleExpose(int x, int y, int w, int h) {
        // Bug ID 4081126 & 4129709 - can't do the clearRect() here,
        // since it interferes with the java thread working in the
        // same window on multi-processor NT machines.

        if (!((Component)target).getIgnoreRepaint()) {
            postEvent(new PaintEvent((Component)target, PaintEvent.PAINT,
                                 new Rectangle(x, y, w, h)));
	}
    }

    /* Invoke a paint() method call on the target, without clearing the
     * damaged area.  This is normally called by a native control after
     * it has painted itself. 
     *
     * NOTE: This is called on the privileged toolkit thread. Do not
     *       call directly into user code using this thread!
     */
    void handlePaint(int x, int y, int w, int h) {
        if (!((Component)target).getIgnoreRepaint()) {
            postEvent(new PaintEvent((Component)target, PaintEvent.PAINT,
                                  new Rectangle(x, y, w, h)));
	}
    }

    /*
     * Post an event. Queue it for execution by the callback thread.
     */
    void postEvent(AWTEvent event) {
        WToolkit.postEvent(WToolkit.targetToAppContext(target), event);
    }

    // Routines to support deferred window positioning.
    public void beginLayout() {
	// Skip all painting till endLayout
	isLayouting = true;
    }

    public void endLayout() {
        if(!paintArea.isEmpty() && !paintPending &&
            !((Component)target).getIgnoreRepaint()) {
	    // if not waiting for native painting repaint damaged area
	    postEvent(new PaintEvent((Component)target, PaintEvent.PAINT, 
			  new Rectangle()));
	}
	isLayouting = false;
    }		     
		     	
    public native void beginValidate();
    public native void endValidate();

    /**
     * DEPRECATED
     */
    public Dimension minimumSize() {
	return getMinimumSize();
    }

    /**
     * DEPRECATED
     */
    public Dimension preferredSize() {
	return getPreferredSize();
    }

    /**
     * register a DropTarget with this native peer
     */

    public synchronized void addDropTarget(DropTarget dt) {
	if (nDropTargets == 0) {
	    nativeDropTargetContext = addNativeDropTarget();
	}
	nDropTargets++;
    }

    /**
     * unregister a DropTarget with this native peer
     */

    public synchronized void removeDropTarget(DropTarget dt) {
        nDropTargets--;
	if (nDropTargets == 0) {
	    removeNativeDropTarget();
	    nativeDropTargetContext = 0;
	}
    }

    /**
     * add the native peer's AwtDropTarget COM object
     * @return reference to AwtDropTarget object
     */

    native long addNativeDropTarget();

    /**
     * remove the native peer's AwtDropTarget COM object
     */

    native void removeNativeDropTarget();
    native boolean nativeHandlesWheelScrolling();

    public boolean handlesWheelScrolling() {
        // should this be cached?
        return nativeHandlesWheelScrolling();
    }  

    // Returns true if we are inside begin/endLayout and
    // are waiting for native painting    
    public boolean isPaintPending() {
	return paintPending && isLayouting;
    }

    public void cancelPendingPaint(int x, int y, int w, int h) {
        paintArea.subtract(x, y, w, h);
    }

    /**
     * The following multibuffering-related methods delegate to our
     * associated GraphicsConfig (Win or WGL) to handle the appropriate
     * native windowing system specific actions.
     */
    
    public void createBuffers(int numBuffers, BufferCapabilities caps)
        throws AWTException
    {
        Win32GraphicsConfig gc =
            (Win32GraphicsConfig)getGraphicsConfiguration();
        gc.assertOperationSupported((Component)target, numBuffers, caps);

        // Re-create the primary surface with the new number of back buffers
        try {
            replaceSurfaceData(numBuffers - 1);
        } catch (InvalidPipeException e) {
            throw new AWTException(e.getMessage());
        }
    }

    public synchronized void destroyBuffers() {
        disposeBackBuffer();
        numBackBuffers = 0;
    }

    private synchronized void disposeBackBuffer() {
        if (backBuffer == null) {
            return;
        }
        backBuffer = null;
    }

    public synchronized void flip(BufferCapabilities.FlipContents flipAction) {
        if (backBuffer == null) {
            throw new IllegalStateException("Buffers have not been created");
        }
        Win32GraphicsConfig gc =
            (Win32GraphicsConfig)getGraphicsConfiguration();
        gc.flip(this, (Component)target, backBuffer, flipAction);
    }

    public Image getBackBuffer() {
        if (backBuffer == null) {
            throw new IllegalStateException("Buffers have not been created");
        }
        return backBuffer;
    }

    /* override and return false on components that DO NOT require
       a clearRect() before painting (i.e. native components) */
    public boolean shouldClearRectBeforePaint() {
        return true;
    }

    native void pSetParent(ComponentPeer newNativeParent);

    /**
     * @see java.awt.peer.ComponentPeer#reparent
     */
    public void reparent(ContainerPeer newNativeParent) {        
        pSetParent(newNativeParent);
    }

    /**
     * @see java.awt.peer.ComponentPeer#isReparentSupported
     */
    public boolean isReparentSupported() {
        return true;
    }

    public void setBoundsOperation(int operation) {
    }
    
    /**
     * Handle to native window
     */
    private long hwnd;
}
