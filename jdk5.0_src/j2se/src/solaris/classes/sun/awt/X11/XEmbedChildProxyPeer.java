/*
 * @(#)XEmbedChildProxyPeer.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.event.*;
import java.awt.image.ColorModel;
import java.awt.image.ImageObserver;
import java.awt.image.ImageProducer;
import java.awt.image.VolatileImage;
import java.awt.peer.*;
import sun.awt.*;
import sun.awt.motif.MToolkit;
import sun.awt.motif.X11FontMetrics;

public class XEmbedChildProxyPeer implements ComponentPeer, XEventDispatcher{
    XEmbeddingContainer container;
    XEmbedChildProxy proxy;
    long handle;
    XEmbedChildProxyPeer(XEmbedChildProxy proxy) {
        this.container = proxy.getEmbeddingContainer();
        this.handle = proxy.getHandle();
        this.proxy = proxy;
        initDispatching();
    }

    void initDispatching() {
        try {
            XToolkit.awtLock();
            XToolkit.addEventDispatcher(handle, this);
            XlibWrapper.XSelectInput(XToolkit.getDisplay(), handle, 
                    XlibWrapper.StructureNotifyMask | XlibWrapper.PropertyChangeMask);
        }
        finally {
            XToolkit.awtUnlock();
        }
        container.notifyChildEmbedded(handle);
    }
    public boolean isObscured() { return false; } 
    public boolean canDetermineObscurity() { return false; }
    public void    	    	setVisible(boolean b) {
        if (!b) {
            try {
                XToolkit.awtLock();
                XlibWrapper.XUnmapWindow(XToolkit.getDisplay(), handle);
            }
            finally {
                XToolkit.awtUnlock();
            }
        } else {
            try {
                XToolkit.awtLock();
                XlibWrapper.XMapWindow(XToolkit.getDisplay(), handle);
            }
            finally {
                XToolkit.awtUnlock();
            }
        }
    }
    public void	setEnabled(boolean b) {}
    public void paint(Graphics g) {}
    public void	repaint(long tm, int x, int y, int width, int height) {}
    public void	print(Graphics g) {}
    public void	setBounds(int x, int y, int width, int height, int op) {
        // Unimplemeneted: Check for min/max hints for non-resizable
        try {
            XToolkit.awtLock();
            XlibWrapper.XMoveResizeWindow(XToolkit.getDisplay(), handle, x, y, width, height);
        }
        finally {
            XToolkit.awtUnlock();
        }
    }
    public void handleEvent(AWTEvent e) {
        switch (e.getID()) {
          case FocusEvent.FOCUS_GAINED:
              XKeyboardFocusManagerPeer.setCurrentNativeFocusOwner(proxy);
              container.focusGained(handle);
              break;
          case FocusEvent.FOCUS_LOST:
              XKeyboardFocusManagerPeer.setCurrentNativeFocusOwner(null);
              container.focusLost(handle);
              break;
          case KeyEvent.KEY_PRESSED:
          case KeyEvent.KEY_RELEASED:
              if (!((InputEvent)e).isConsumed()) {
                  container.forwardKeyEvent(handle, (KeyEvent)e);
              }
              break;
        }
    }
    public void                coalescePaintEvent(PaintEvent e) {}
    public Point		getLocationOnScreen() {
        XWindowAttributes attr = new XWindowAttributes();
        try{ 
            XToolkit.awtLock();
            XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(), handle, attr.pData);
            return new Point(attr.get_x(), attr.get_y());
        } finally {
            XToolkit.awtUnlock();
            attr.dispose();
        }
    }
    public Dimension		getPreferredSize() {
        long p_hints = XlibWrapper.XAllocSizeHints();
        try {
            XToolkit.awtLock();
            XSizeHints hints = new XSizeHints(p_hints);
            XlibWrapper.XGetWMNormalHints(XToolkit.getDisplay(), handle, p_hints, XlibWrapper.larg1);
            Dimension res = new Dimension(hints.get_width(), hints.get_height());
            return res;
        } finally {
            XlibWrapper.XFree(p_hints);
            XToolkit.awtUnlock();            
        }            
    }
    public Dimension		getMinimumSize() {
        XToolkit.awtLock();
        long p_hints = XlibWrapper.XAllocSizeHints();
        try {
            XSizeHints hints = new XSizeHints(p_hints);
            XlibWrapper.XGetWMNormalHints(XToolkit.getDisplay(), handle, p_hints, XlibWrapper.larg1);        
            Dimension res = new Dimension(hints.get_min_width(), hints.get_min_height());
            return res;
        } finally {
            XlibWrapper.XFree(p_hints);
            XToolkit.awtUnlock();            
        }
    }
    public ColorModel		getColorModel() { return null; }
    public Toolkit		getToolkit() { return Toolkit.getDefaultToolkit(); }

    public Graphics		getGraphics() { return null; }
    public FontMetrics		getFontMetrics(Font font) { return null; }
    public void		dispose() { 
        container.detachChild(handle);        
    }
    public void		setForeground(Color c) {} 
    public void		setBackground(Color c) {}
    public void		setFont(Font f) {}
    public void 		updateCursorImmediately() {}

    void postEvent(AWTEvent event) {
        XToolkit.postEvent(XToolkit.targetToAppContext(proxy), event);
    }

    boolean simulateMotifRequestFocus(Component lightweightChild, boolean temporary,
                                      boolean focusedWindowChangeAllowed, long time) 
    {
        if (lightweightChild == null) {
            lightweightChild = (Component)proxy;
        }
        Component currentOwner = XKeyboardFocusManagerPeer.getCurrentNativeFocusOwner();
        if (currentOwner != null && currentOwner.getPeer() == null) {
            currentOwner = null;
        }
        FocusEvent  fg = new FocusEvent(lightweightChild, FocusEvent.FOCUS_GAINED, false, currentOwner );
        FocusEvent fl = null;
        if (currentOwner != null) {
            fl = new FocusEvent(currentOwner, FocusEvent.FOCUS_LOST, false, lightweightChild);
        }
        
        if (fl != null) {
            postEvent(XComponentPeer.wrapInSequenced(fl));
        }
        postEvent(XComponentPeer.wrapInSequenced(fg));
        // End of Motif compatibility code
        return true;
    }

    public boolean		requestFocus(Component lightweightChild,
                                     boolean temporary,
				     boolean focusedWindowChangeAllowed,
                                             long time) 
    {
        int result = XKeyboardFocusManagerPeer
            .shouldNativelyFocusHeavyweight(proxy, lightweightChild, 
                                            temporary, false, time);

        switch (result) {
          case XComponentPeer.SNFH_FAILURE:
              return false;
          case XComponentPeer.SNFH_SUCCESS_PROCEED:
              // Currently we just generate focus events like we deal with lightweight instead of calling
              // XSetInputFocus on native window

              /**
               * The problems with requests in non-focused window arise because shouldNativelyFocusHeavyweight
               * checks that native window is focused while appropriate WINDOW_GAINED_FOCUS has not yet 
               * been processed - it is in EventQueue. Thus, SNFH allows native request and stores request record 
               * in requests list - and it breaks our requests sequence as first record on WGF should be the last focus
               * owner which had focus before WLF. So, we should not add request record for such requests
               * but store this component in mostRecent - and return true as before for compatibility.
               */
              Container parent = proxy.getParent();
              // Search for parent window
              while (parent != null && !(parent instanceof Window)) {
                  parent = parent.getParent();
              }
              if (parent != null) {                
                  Window parentWindow = (Window)parent;
                  // and check that it is focused
                  if (!parentWindow.isFocused() && XKeyboardFocusManagerPeer.getCurrentNativeFocusedWindow() == parentWindow) {
                      // if it is not - skip requesting focus on Solaris
                      // but return true for compatibility.
                      return true;
                  }
              }

              // NOTE: We simulate heavyweight behavior of Motif - component receives focus right
              // after request, not after event. Normally, we should better listen for event
              // by listeners.
              return simulateMotifRequestFocus(lightweightChild, temporary, focusedWindowChangeAllowed, time);
              // Motif compatibility code
          case XComponentPeer.SNFH_SUCCESS_HANDLED:
              // Either lightweight or excessive requiest - all events are generated.
              return true;
        }
        return false;
    }
    public boolean		isFocusable() {
        return true;
    }

    public Image 		createImage(ImageProducer producer) { return null; }
    public Image 		createImage(int width, int height) { return null; }
    public VolatileImage 	createVolatileImage(int width, int height) { return null; }
    public boolean		prepareImage(Image img, int w, int h, ImageObserver o) { return false; }
    public int			checkImage(Image img, int w, int h, ImageObserver o) { return 0; }
    public GraphicsConfiguration getGraphicsConfiguration() { return null; }
    public boolean     handlesWheelScrolling() { return true; }
    public void createBuffers(int numBuffers, BufferCapabilities caps)
      throws AWTException { }
    public Image getBackBuffer() { return null; }
    public void flip(BufferCapabilities.FlipContents flipAction) {  }
    public void destroyBuffers() { }

    /**
     * Used by lightweight implementations to tell a ComponentPeer to layout
     * its sub-elements.  For instance, a lightweight Checkbox needs to layout
     * the box, as well as the text label.
     */
    public void        layout() {}

    /**
     * DEPRECATED:  Replaced by getPreferredSize().
     */
    public Dimension		preferredSize() { 
        return getPreferredSize();
    }

    /**
     * DEPRECATED:  Replaced by getMinimumSize().
     */
    public Dimension		minimumSize() {
        return minimumSize();
    }

    /**
     * DEPRECATED:  Replaced by setVisible(boolean).
     */
    public void		show() {
        setVisible(true);
    }

    /**
     * DEPRECATED:  Replaced by setVisible(boolean).
     */
    public void		hide() {
        setVisible(false);
    }

    /**
     * DEPRECATED:  Replaced by setEnabled(boolean).
     */
    public void		enable() {}

    /**
     * DEPRECATED:  Replaced by setEnabled(boolean).
     */
    public void		disable() {}

    /**
     * DEPRECATED:  Replaced by setBounds(int, int, int, int).
     */
    public void	reshape(int x, int y, int width, int height) {
        setBounds(x, y, width, height, SET_BOUNDS);
    }

    Window getTopLevel(Component comp) {
        while (comp != null && !(comp instanceof Window)) {
            comp = comp.getParent();
        }
        return (Window)comp;
    }

    void childResized() {
        XToolkit.postEvent(XToolkit.targetToAppContext(proxy), new ComponentEvent(proxy, ComponentEvent.COMPONENT_RESIZED));
        container.childResized(proxy);
//         XToolkit.postEvent(XToolkit.targetToAppContext(proxy), new InvocationEvent(proxy, new Runnable() {
//                 public void run() {
//                     getTopLevel(proxy).invalidate();
//                     getTopLevel(proxy).pack();
//                 }
//             }));
    }
    void handlePropertyNotify(long ptr) {
        XPropertyEvent ev = new XPropertyEvent(ptr);
        if (ev.get_atom() == XAtom.XA_WM_NORMAL_HINTS) {
            childResized();
        }
    }
    void handleConfigureNotify(long ptr) {
        childResized();
    }
    public void dispatchEvent(IXAnyEvent ev) {
        int type = ev.get_type();
        switch (type) {
          case (int)XlibWrapper.PropertyNotify:
              handlePropertyNotify(ev.getPData());
              break;
          case (int)XlibWrapper.ConfigureNotify:
              handleConfigureNotify(ev.getPData());
              break;
        }
    }

    void requestXEmbedFocus() {
        postEvent(new InvocationEvent(proxy, new Runnable() {
                public void run() {
                    proxy.requestFocusInWindow();
                }
            }));
    }

    public void reparent(ContainerPeer newNativeParent) {        
    }
    public boolean isReparentSupported() {
        return false;
    }
    public Rectangle getBounds() {
        XWindowAttributes attrs = new XWindowAttributes();
        try {
            XToolkit.awtLock();
            XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(), handle, attrs.pData);
            return new Rectangle(attrs.get_x(), attrs.get_y(), attrs.get_width(), attrs.get_height());
        } finally {
            XToolkit.awtUnlock();
            attrs.dispose();
        }
    }
    public void setBoundsOperation(int operation) {        
    }
}
