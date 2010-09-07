/*
 * @(#)XDecoratedPeer.java	1.66 04/06/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import sun.misc.Unsafe;
import java.util.*;
import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import sun.awt.im.*;
import sun.awt.SunToolkit;
import sun.java2d.SunGraphics2D;
import sun.awt.*;
import java.util.logging.*;

class XDecoratedPeer extends XWindowPeer {
    private static Logger log = Logger.getLogger("sun.awt.X11.XDecoratedPeer");
    private static Logger insLog = Logger.getLogger("sun.awt.X11.insets.XDecoratedPeer");
    private static final Logger focusLog = Logger.getLogger("sun.awt.X11.focus.XDecoratedPeer");

    private static XAtom resize_request = new XAtom("_SUN_AWT_RESIZE_REQUEST", false);

    // Set to true when we get the first ConfigureNotify after being
    // reparented - indicates that WM has adopted the top-level.
    boolean configure_seen;

    WindowDimensions dimensions;
    XContentWindow content;
    Insets currentInsets;

    XDecoratedPeer(Window target) {
        super(target);
    }

    XDecoratedPeer(XCreateWindowParams params) {
        super(params);
    }

    public long getShell() {
        return window;
    }

    public long getContentWindow() {
        return (content == null) ? window : content.getWindow();
    }

    void preInit(XCreateWindowParams params) {
        super.preInit(params);
        if (!resize_request.isInterned()) {
            resize_request.intern(false);
        }
        winAttr.initialFocus = true;

        currentInsets = new Insets(0,0,0,0); // replacemenet for wdata->top, left, bottom, right

        changeInsets();
        Rectangle bounds = (Rectangle)params.get(BOUNDS);
        dimensions = new WindowDimensions(bounds, getRealInsets(), false);
        params.put(BOUNDS, dimensions.getClientRect());
        insLog.log(Level.FINE, "Initial dimensions {0}", new Object[] { dimensions });
    }

    void postInit(XCreateWindowParams params) {
        super.postInit(params);
        // The lines that follow need to be in a postInit, so they
        // happen after the X window is created.
        initResizability();
        updateSizeHints(dimensions);
        content = createContent(dimensions);
        content.initialize();
        if (warningWindow != null) {
            warningWindow.toFront();
        }
    }    

    XContentWindow createContent(WindowDimensions dims) {
        Rectangle rec = dims.getBounds();
        // Fix for  - set the location of the content window to the (-left inset, -top inset)
        Insets ins = dims.getInsets();
        if (ins != null) {
            rec.x = -ins.left;
            rec.y = -ins.top;
        } else {
            rec.x = 0;
            rec.y = 0;
        }
        return new XContentWindow(this, rec);
    }

    protected XAtomList getWMProtocols() {
        XAtomList protocols = super.getWMProtocols();
        protocols.add(wm_delete_window);
        protocols.add(wm_take_focus);
        return protocols;
    }

    public Graphics getGraphics() {
        return getGraphics(content.surfaceData,
                           target.getForeground(),
                           target.getBackground(),
                           target.getFont());
    }

    public void setTitle(String title) {
        if (log.isLoggable(Level.FINE)) log.fine("Title is " + title);
        setWMName(title);
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleIconify() {
        postEvent(new WindowEvent((Window)target, WindowEvent.WINDOW_ICONIFIED));
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleDeiconify() {
        postEvent(new WindowEvent((Window)target, WindowEvent.WINDOW_DEICONIFIED));
    }

    public void handleFocusEvent(long ptr) {
        super.handleFocusEvent(ptr);
        XFocusChangeEvent xfe = new XFocusChangeEvent(ptr);
        if (xfe.get_type() == FocusOut && xfe.get_mode() == NotifyGrab) {
            // User grabbed window by title
            log.finer("Generating MOVED event due to a user grab on toplevel");
            postEvent(new ComponentEvent(target, ComponentEvent.COMPONENT_MOVED));
        }
    }

/***************************************************************************************
 *                             I N S E T S   C O D E
 **************************************************************************************/

    protected boolean isInitialReshape() {
        return false;
    }
    
    Insets difference(Insets i1, Insets i2) {
        return new Insets(i1.top-i2.top, i1.left - i2.left, i1.bottom-i2.bottom, i1.right-i2.right);
    }

    void add(Insets i1, Insets i2) {
        i1.left += i2.left;
        i1.top += i2.top;
        i1.right += i2.right;
        i1.bottom += i2.bottom;
    }
    boolean isNull(Insets i) {
        return (i.left | i.top | i.right | i.bottom) == 0;
    }
    Insets copy(Insets i) {
        return new Insets(i.top, i.left, i.bottom, i.right);
    }
    
    long reparent_serial = 0;

    public void handleReparentNotifyEvent(long ptr) { // awt_TopLevel.c, 1.97
        XReparentEvent  xe = new XReparentEvent(ptr);
        if (insLog.isLoggable(Level.FINE)) insLog.fine(xe.toString());
        reparent_serial = xe.get_serial();
        XToolkit.awtLock();
        try {
            long root = XlibWrapper.RootWindow(XToolkit.getDisplay(), getScreenNumber());
            
            if (isEmbedded()) {
                reparented = true;
                return;
            }
            Component t = (Component)target;
            if (getDecorations() == winAttr.AWT_DECOR_NONE) {
                reparented = true;
                reshape(dimensions, SET_SIZE, false);
            } else if (xe.get_parent() == root) {
                configure_seen = false;

                /*
                 * We can be repareted to root for two reasons:
                 *   . setVisible(false)
                 *   . WM exited
                 */
                if (isVisible()) { /* WM exited */
                    /* Work around 4775545 */
                    XWM.getWM().unshadeKludge(this);
                }
            } else { /* reparented to WM frame, figure out our insets */
                reparented = true;
                Insets correctWM = XWM.getWM().getInsets(this, xe);

                insLog.log(Level.FINER, "correctWM {0}", new Object[] {correctWM});
                /*
                 * Ok, now see if we need adjust window size because
                 * initial insets were wrong (most likely they were).
                 */
                Insets correction = difference(correctWM, currentInsets);                    
                insLog.log(Level.FINEST, "Corrention {0}", new Object[] {correction});
                if (!isNull(correction)) {
                    /*
                     * Actual insets account for menubar/warning label,
                     * so we can't assign directly but must adjust them.
                     */
                    add(currentInsets, correction);
                    changeInsets();

                    /* 
                     * If this window has been sized by a pack() we need
                     * to keep the interior geometry intact.  Since pack()
                     * computed width and height with wrong insets, we
                     * must adjust the target dimensions appropriately.
                     */
                }
                if (insLog.isLoggable(Level.FINER)) insLog.finer("Dimensions before reparent: " + dimensions);

                dimensions.setInsets(getRealInsets());

                if (isMaximized()) {
                    return;
                }

                if ((getHints().get_flags() & (USPosition | PPosition)) != 0) {
                    reshape(dimensions, SET_BOUNDS, false);
                } else {
                    reshape(dimensions, SET_SIZE, false);
                }
            }
        } finally {
            XToolkit.awtUnlock();
        }
    }

    public void handleMoved(WindowDimensions dims) {
        Point loc = dims.getLocation();
        ComponentAccessor.setX((Component)target, loc.x);
        ComponentAccessor.setY((Component)target, loc.y);
        postEvent(new ComponentEvent(target, ComponentEvent.COMPONENT_MOVED));
    }


    protected Insets guessInsets() {
        if (isEmbedded()) {
            return new Insets(0, 0, 0, 0);
        } else {
            if (currentInsets.top > 0) {
                /* insets were set on wdata by System Properties */
                return copy(currentInsets);
            } else {
                return XWM.getWM().guessInsets(this);
            }
        }
    }

    void changeInsets() {
        Insets guessed = guessInsets();
        currentInsets = copy(guessed);
        insets = copy(currentInsets);
    }   

    public void revalidate() {
        XToolkit.executeOnEventHandlerThread(target, new Runnable() {
                public void run() {
                    target.invalidate();
                    target.validate();
                }
            });
    }

    Insets getRealInsets() {
        if (isNull(insets)) {
            changeInsets();
        }
        return insets;
    }
    
    public Insets getInsets() {  
        Insets in = copy(getRealInsets());
        in.top += getMenuBarHeight() + getWarningWindowHeight();
        if (insLog.isLoggable(Level.FINEST)) insLog.log(Level.FINEST, "Get insets returns {0}", new Object[] {in});
        return in;
    }

    boolean gravityBug() { 
        return XWM.configureGravityBuggy();
    }
    
    // The height of area used to display current active input method
    int getInputMethodHeight() {
        return 0;
    }

    void updateSizeHints(WindowDimensions dims) {
        Rectangle rec = dims.getClientRect();
        checkShellRect(rec);
        updateSizeHints(rec.x, rec.y, rec.width, rec.height);
    }

    void updateSizeHints() {
        updateSizeHints(dimensions);
    }

    // Coordinates are that of the target
    // Called only on Toolkit thread
    public void reshape(WindowDimensions newDimensions, int op,
                        boolean userReshape)
    {
        if (insLog.isLoggable(Level.FINE)) insLog.fine("Reshaping " + this + " to " + newDimensions);
        XToolkit.awtLock();
        try {
            if (!reparented || !isVisible()) {
                insLog.log(Level.FINE, "- not reparented({0}) or not visible({1}), default reshape",
                           new Object[] {Boolean.valueOf(reparented), Boolean.valueOf(visible)});
                dimensions = new WindowDimensions(newDimensions);
                updateSizeHints(dimensions);
                Rectangle client = dimensions.getClientRect();
                checkShellRect(client);
                setShellBounds(client);
                if (content != null && 
                    !content.getSize().equals(newDimensions.getSize())) 
                {
                    reconfigureContentWindow(newDimensions);
                }
                return;
            }
            
            int wm = XWM.getWMID();
            updateChildrenSizes();
            changeInsets();
            
            Rectangle shellRect = newDimensions.getClientRect();

            if (gravityBug()) {
                Insets in = newDimensions.getInsets();
                shellRect.translate(in.left, in.top);
            }

            if ((op & NO_EMBEDDED_CHECK) == 0 && isEmbedded()) {
                shellRect.setLocation(0, 0);
            }

            checkShellRect(shellRect);

            if (op == SET_LOCATION) {
                setShellPosition(shellRect);
            } else if (isResizable()) {
                if (op == SET_BOUNDS) {
                    setShellBounds(shellRect);
                } else {
                    setShellSize(shellRect);
                }
            } else {
                XWM.setShellNotResizable(this, newDimensions, shellRect, true);
                if (op == SET_BOUNDS) {
                    setShellPosition(shellRect);
                }
            }

            reconfigureContentWindow(newDimensions);
        } finally {
            XToolkit.awtUnlock();
        }
    }

    /**
     * @param x, y, width, heith - dimensions of the window with insets
     */
    private void reshape(int x, int y, int width, int height, int operation,
                         boolean userReshape)
    {
        Rectangle newRec;
        boolean setClient = false;
        WindowDimensions dims = new WindowDimensions(dimensions);
        switch (operation & (~NO_EMBEDDED_CHECK)) {
          case SET_LOCATION:
              // Set location always sets bounds location. However, until the window is mapped we
              // should use client coordinates
              dims.setLocation(x, y);
              break;
          case SET_SIZE:
              // Set size sets bounds size. However, until the window is mapped we
              // should use client coordinates
              dims.setSize(width, height);
              break;
          case SET_CLIENT_SIZE: {
              // Sets client rect size. Width and height contain insets.
              Insets in = currentInsets;
              width -= in.left+in.right;
              height -= in.top+in.bottom;
              dims.setClientSize(width, height);
              break;
          }
          case SET_BOUNDS:
          default:
              dims.setLocation(x, y);
              dims.setSize(width, height);
              break;
        }
        if (insLog.isLoggable(Level.FINE)) insLog.log(Level.FINE, "For the operation {0} new dimensions are {1}",
                                                      new Object[] {operationToString(operation), dims});

        reshape(dims, operation, userReshape);
    }
    
    /**
     * @see java.awt.peer.ComponentPeer#setBounds
     */
    public void setBounds(int x, int y, int width, int height, int op) {
        // TODO: Rewrite with WindowDimensions
        reshape(x, y, width, height, op, true);
        validateSurface();
    }

    // Coordinates are that of the shell
    void reconfigureContentWindow(WindowDimensions dims) {
        if (content == null) {
            insLog.fine("WARNING: Content window is null");
            return;
        }
        content.setContentBounds(dims);
    }

    public void handleConfigureNotifyEvent(long ptr) {
        XConfigureEvent xe = new XConfigureEvent(ptr);
        insLog.log(Level.FINE, "Configure notify {0}", new Object[] {xe});

        // XXX: should really only consider synthetic events, but 
        if (reparented)
            configure_seen = true;

        if (!isMaximized() && (xe.get_serial() == reparent_serial || xe.get_window() != getShell())) {
            insLog.fine("- reparent artifact, skipping");
            return;
        }
        
        /**
         * When there is a WM we receive some CN before being visible and after.
         * We should skip all CN which are before being visible, because we assume
         * the gravity is in action while it is not yet.
         *
         * When there is no WM we receive CN only _before_ being visible.
         * We should process these CNs.
         */
        if (!isVisible() && XWM.getWMID() != XWM.NO_WM) {
            insLog.fine(" - not visible, skipping");
            return;
        }

        /*
         * Some window managers configure before we are reparented and
         * the send event flag is set! ugh... (Enlighetenment for one,
         * possibly MWM as well).  If we haven't been reparented yet
         * this is just the WM shuffling us into position.  Ignore
         * it!!!! or we wind up in a bogus location.
         */
        XToolkit.awtLock();
        try {
            int runningWM = XWM.getWMID();
            if (!reparented && isVisible() && runningWM != XWM.NO_WM 
                && getDecorations() != winAttr.AWT_DECOR_NONE) 
            {
                insLog.fine("- visible but not reparented, skipping");
                return;
            }
            updateChildrenSizes();

            // Bounds of the window
            Rectangle targetBounds = new Rectangle(ComponentAccessor.getX((Component)target),
                                                   ComponentAccessor.getY((Component)target),
                                                   ComponentAccessor.getWidth((Component)target),
                                                   ComponentAccessor.getHeight((Component)target));
            // FIXME: How about menu? Client code treats them as insets, we don't
            WindowDimensions targetDimensions = new WindowDimensions(targetBounds, copy(currentInsets), false);


            // Location, Client size + insets
            WindowDimensions newDimensions = 
                new WindowDimensions(
                                     new Point((xe.get_send_event()) ? xe.get_x()-currentInsets.left : ComponentAccessor.getX((Component)target),
                                               (xe.get_send_event()) ? xe.get_y()-currentInsets.top : ComponentAccessor.getY((Component)target)),
                                     new Dimension(xe.get_width(), xe.get_height()),                                     
                                     // FIXME: How about menu? Client code treats them as insets, we don't
                                     copy(currentInsets),
                                     true);
            insLog.log(Level.FINER, "Insets are {0}, new dimensions {1}", 
                       new Object[] {currentInsets, newDimensions});

            checkIfOnNewScreen(newDimensions.getBounds());

            dimensions = newDimensions;
            if (!newDimensions.equals(targetDimensions)) {
                if (!newDimensions.getLocation().equals(targetDimensions.getLocation())) {
                    handleMoved(newDimensions);
                }
            }
            reconfigureContentWindow(newDimensions);
            updateChildrenSizes();
        } finally {
            XToolkit.awtUnlock();
        }
    }

    private void checkShellRect(Rectangle shellRect) {
        if (shellRect.width < 0) {
            shellRect.width = 1;
        }
        if (shellRect.height < 0) {
            shellRect.height = 1;
        }
        int wm = XWM.getWMID();
        if (wm == XWM.MOTIF_WM || wm == XWM.CDE_WM) {
            if (shellRect.x == 0 && shellRect.y == 0) {
                shellRect.x = shellRect.y = 1;
            }
        }
    }

    public void setShellBounds(Rectangle rec) {
        if (insLog.isLoggable(Level.FINE)) insLog.fine("Setting shell bounds on " + 
                                                       this + " to " + rec);
        try {
            XToolkit.awtLock();
            updateSizeHints(rec.x, rec.y, rec.width, rec.height);
            XlibWrapper.XResizeWindow(XToolkit.getDisplay(), getShell(), rec.width, rec.height);
            XlibWrapper.XMoveWindow(XToolkit.getDisplay(), getShell(), rec.x, rec.y);
        }
        finally {
            XToolkit.awtUnlock();
        }
    }
    public void setShellSize(Rectangle rec) {
        if (insLog.isLoggable(Level.FINE)) insLog.fine("Setting shell size on " + 
                                                       this + " to " + rec);
        try {
            XToolkit.awtLock();
            updateSizeHints(rec.x, rec.y, rec.width, rec.height);
            XlibWrapper.XResizeWindow(XToolkit.getDisplay(), getShell(), rec.width, rec.height);
        }
        finally {
            XToolkit.awtUnlock();
        }
    }
    public void setShellPosition(Rectangle rec) {
        if (insLog.isLoggable(Level.FINE)) insLog.fine("Setting shell position on " + 
                                                       this + " to " + rec);
        try {
            XToolkit.awtLock();
            updateSizeHints(rec.x, rec.y, rec.width, rec.height);
            XlibWrapper.XMoveWindow(XToolkit.getDisplay(), getShell(), rec.x, rec.y);
        }
        finally {
            XToolkit.awtUnlock();
        }
    }

    void initResizability() {
        setResizable(winAttr.initialResizability);
    }
    public void setResizable(boolean resizable) {
        if (!isResizable() && resizable) {
            insets = currentInsets = new Insets(0, 0, 0, 0);
            target.invalidate();
            if (!isEmbedded()) {
                reparented = false;
            }
            winAttr.isResizable = resizable;
            XWM.setShellResizable(this);
        } else if (isResizable() && !resizable) {
            insets = currentInsets = new Insets(0, 0, 0, 0);
            target.invalidate();
            if (!isEmbedded()) {
                reparented = false;
            }
            winAttr.isResizable = resizable;
            XWM.setShellNotResizable(this, dimensions, dimensions.getBounds(), false);
        }
    }

    Rectangle getShellBounds() {
        return dimensions.getClientRect();
    }

    public Rectangle getBounds() {
        return dimensions.getBounds();
    }

    public Dimension getSize() {
        return dimensions.getSize();
    }

    public WindowDimensions getDimensions() {
        return dimensions;
    }

    public Point getLocationOnScreen() {        
        synchronized (getAWTLock()) {
            if (configure_seen) {
                return toGlobal(0,0);
            } else {
                Point location = target.getLocation();
                if (insLog.isLoggable(Level.FINE))
                    insLog.fine("getLocationOnScreen " + this
                                + " not reparented: " + location);
                return location;
            }
        }
    }

    
/***************************************************************************************
 *              END            OF             I N S E T S   C O D E
 **************************************************************************************/

    protected boolean isEventDisabled(IXAnyEvent e) {
        switch (e.get_type()) {
            // Do not generate MOVED/RESIZED events since we generate them by ourselves
          case ConfigureNotify:
              return true;
          case EnterNotify:
          case LeaveNotify:
              // Disable crossing event on outer borders of Frame so 
              // we receive only one set of cross notifications(first set is from content window)
              return true;
          default:
              return super.isEventDisabled(e);
        }        
    }

    int getDecorations() {
        return winAttr.decorations;
    }

    public void setVisible(boolean vis) {
        log.log(Level.FINER, "Setting to visible {0}", new Object[] {Boolean.valueOf(vis)});
        if (vis && !isVisible()) {
            XWM.setShellDecor(this);
            super.setVisible(vis);
            if (winAttr.isResizable) {
                XWM.removeSizeHints(this, XlibWrapper.PMinSize | XlibWrapper.PMaxSize);
            }
        } else {
            super.setVisible(vis);
        }
    }

    public void dispose() {
        if (content != null) {
            content.destroy();
        }
        super.dispose();
    }

    public void handleClientMessage(long ptr) {
        super.handleClientMessage(ptr);
        XClientMessageEvent cl = new XClientMessageEvent(ptr);
        if ((wm_protocols != null) && (cl.get_message_type() == wm_protocols.getAtom())) {
            if (cl.get_data(0) == wm_delete_window.getAtom()) {
                handleQuit();
            } else if (cl.get_data(0) == wm_take_focus.getAtom()) {
                focusLog.log(Level.FINE, "WM_TAKE_FOCUS on {0}", new Object[]{this});
                if (focusAllowedFor()) {
                    focusLog.fine("WM_TAKE_FOCUS accepted focus");
                    xRequestFocus(cl.get_data(1));
                }
            }
        } else if (cl.get_message_type() == resize_request.getAtom()) {
            reshape((int)cl.get_data(0), (int)cl.get_data(1),
                    (int)cl.get_data(2), (int)cl.get_data(3),
                    (int)cl.get_data(4), true);
        }
    }

    public void handleQuit() {
        postEvent(new WindowEvent((Window)target, WindowEvent.WINDOW_CLOSING));
    }

    final void dumpMe() {
        System.err.println(">>> Peer: " + x + ", " + y + ", " + width + ", " + height);
    }

    final void dumpTarget() {
        int getWidth = ComponentAccessor.getWidth((Component)target);
        int getHeight = ComponentAccessor.getHeight((Component)target);
        int getTargetX = ComponentAccessor.getX((Component)target); 
        int getTargetY = ComponentAccessor.getY((Component)target); 
        System.err.println(">>> Target: " + getTargetX + ", " + getTargetY + ", " + getWidth + ", " + getHeight);        
    }

    final void dumpShell() {
        dumpWindow("Shell", getShell());
    }
    final void dumpContent() {
        dumpWindow("Content", getContentWindow());
    }
    final void dumpParent() {
        XQueryTree qt = new XQueryTree(getShell());
        try {
            qt.execute();
            if (qt.get_parent() != 0) {
                dumpWindow("Parent", qt.get_parent());
            } else {
                System.err.println(">>> NO PARENT");
            }
        } finally {
            qt.dispose();
        }
    }

    final void dumpWindow(String id, long window) {
        XWindowAttributes pattr = new XWindowAttributes();
        try {
            XToolkit.awtLock();
            int status = XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                          window, pattr.pData);
        }
        finally {
            XToolkit.awtUnlock();
        }
        System.err.println(">>>> " + id + ": " + pattr.get_x() + ", " + pattr.get_y() + ", " + pattr.get_width() + ", " + pattr.get_height());
        pattr.dispose();
    }

    final void dumpAll() {
        dumpTarget();
        dumpMe();
        dumpParent();
        dumpShell();
        dumpContent();
    }

    boolean isMaximized() {
        return false;
    }

    boolean isOverrideRedirect() {
        return false;
    }

    public boolean requestWindowFocus() {
        focusLog.fine("Request for decorated window focus");
        // If this is Frame or Dialog we can't assure focus request success - but we still can try
        // If this is Window and its owner Frame is active we can be sure request succedded.        
        Window win = (Window)target;
        focusLog.log(Level.FINER, "Current window is: focused={0}, active={1}", 
                     new Object[]{Boolean.valueOf(win.isActive()), Boolean.valueOf(win.isFocused())});
        if (focusAllowedFor()){
            if (win.isActive() && !win.isFocused()) {
                // Happens when focus is on window child
                focusLog.fine("Focus is on child window - transfering it back");
                handleWindowFocusInSync(-1);
            } else {
                focusLog.fine("Requesting focus to this top-level");
                ((XWindowPeer)target.getPeer()).xRequestFocus();
            }
        }
        // This might change when WM will have property to determine focus policy.
        // Right now, because policy is unknown we can't be sure we succedded
        return true;
    }

    private XWindowPeer actualFocusedWindow = null;
    boolean requestWindowFocus(XWindowPeer actualFocusedWindow) {
        synchronized(getStateLock()) {
            this.actualFocusedWindow = actualFocusedWindow;
        }
        return requestWindowFocus();
    }
    public void handleWindowFocusIn(long serial) {
        super.handleWindowFocusIn(serial);
        XWindowPeer fw = null;
        synchronized(getStateLock()) {
            fw = actualFocusedWindow;
            actualFocusedWindow = null;
        }
        if (fw != null && fw.isVisible()) {
            fw.handleWindowFocusIn(serial);
        }
    }

    public void handleWindowFocusOut(Window oppositeWindow, long serial) {
        Window actualFocusedWindow = XKeyboardFocusManagerPeer.getCurrentNativeFocusedWindow();

        // If the actual focused window is not this decorated window then retain it.
        if (actualFocusedWindow != null && actualFocusedWindow != target) {
            Window owner = actualFocusedWindow.getOwner();

            // Check that actual focused window is one of owned windows
            while (owner != null && !(owner instanceof Frame || owner instanceof Dialog)) {
                owner = actualFocusedWindow.getOwner();
            }
            if (owner != null && owner == target) {
                this.actualFocusedWindow = (XWindowPeer)actualFocusedWindow.getPeer();
            }
        }
        super.handleWindowFocusOut(oppositeWindow, serial);
    }
}
