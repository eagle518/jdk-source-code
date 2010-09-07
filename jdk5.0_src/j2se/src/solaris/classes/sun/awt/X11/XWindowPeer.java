/*
 * @(#)XWindowPeer.java	1.71 04/07/12
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
import sun.awt.DisplayChangedListener;
import sun.awt.SunToolkit;
import sun.awt.motif.*;
import sun.awt.*;

import java.util.logging.*;

class XWindowPeer extends XPanelPeer implements WindowPeer,
                                                DisplayChangedListener {

    private static Logger log = Logger.getLogger("sun.awt.X11.XWindowPeer");
    private static final Logger focusLog = Logger.getLogger("sun.awt.X11.focus");
    private static Set windows = new HashSet();
    static XAtom wm_protocols;
    static XAtom wm_delete_window; 
    static XAtom wm_take_focus;

    Insets insets = new Insets( 0, 0, 0, 0 );
    XWindowAttributesData winAttr;
    boolean cachedFocusableWindow;
    XWarningWindow warningWindow;
    XDialogPeer modalBlocker;
    boolean modalExclude;
    
    private boolean stateChanged; // Indicates whether the value on savedState is valid
    private int savedState; // Holds last known state of the top-level window
    private boolean mustControlStackPosition = false; // Am override-redirect not on top
    private XEventDispatcher rootPropertyEventDispatcher = null;

    XWindowPeer(XCreateWindowParams params) {
        super(params.putIfNull(PARENT_WINDOW, new Long(0)));
    }

    XWindowPeer(Window target) {
        super(new XCreateWindowParams(new Object[] {
            TARGET, target,
            PARENT_WINDOW, new Long(0)}));
    }

    // fallback default font object
    static Font defaultFont;

    void preInit(XCreateWindowParams params) {
        super.preInit(params);        
        params.putIfNull(BIT_GRAVITY, new Integer(NorthWestGravity));

        savedState = WithdrawnState;

        windows.add(this);
        winAttr = new XWindowAttributesData();
        insets = new Insets(0,0,0,0);

        params.put(OVERRIDE_REDIRECT, Boolean.valueOf(isOverrideRedirect()));
        try {
            XToolkit.awtLock();
            if (wm_protocols == null) {
                wm_protocols = XAtom.get("WM_PROTOCOLS");
                wm_delete_window = XAtom.get("WM_DELETE_WINDOW");
                wm_take_focus = XAtom.get("WM_TAKE_FOCUS");
            }
        }
        finally {
            XToolkit.awtUnlock();
        }
        cachedFocusableWindow = isFocusableWindow();

        Font f = target.getFont();
        if (f == null) {
            if (defaultFont == null) {
                defaultFont = new Font("Dialog", Font.PLAIN, 12);
            }
            f = defaultFont;
            target.setFont(f);
            // we should not call setFont because it will call a repaint
            // which the peer may not be ready to do yet.
        }
        Color c = target.getBackground();
        if (c == null) {
            Color background = SystemColor.window;
            target.setBackground(background);
            // we should not call setBackGround because it will call a repaint
            // which the peer may not be ready to do yet.
        }
        c = target.getForeground();
        if (c == null) {
            target.setForeground(SystemColor.windowText);
            // we should not call setForeGround because it will call a repaint
            // which the peer may not be ready to do yet.
        }    
    }

    private void initWMProtocols() {
        wm_protocols.setAtomListProperty(this, getWMProtocols());
    }

    /**
     * Returns list of protocols which should be installed on this window.
     * Descendants can override this method to add class-specific protocols
     */
    protected XAtomList getWMProtocols() {
        // No protocols on simple window
        return new XAtomList();
    }


    protected String getWMName() {
        String name = target.getName();
        if (name == null || name.trim().equals("")) {
            name = " ";
        }
        return name;
    }
    
    void postInit(XCreateWindowParams params) {
        super.postInit(params);

        // Init WM_PROTOCOLS atom
        initWMProtocols();

        // Set WM_TRANSIENT_FOR and group_leader
        Window t_window = (Window)target;
        Window owner = t_window.getOwner();
        if (owner != null) {
            XWindow ownerPeer = (XWindow)owner.getPeer();
            if (focusLog.isLoggable(Level.FINER)) {
                focusLog.fine("Owner is " + owner);
                focusLog.fine("Owner peer is " + ownerPeer);
                focusLog.fine("Owner X window " + ownerPeer.getWindow());
                focusLog.fine("Owner content X window " + ownerPeer.getContentWindow());
            }
            long ownerWindow = ownerPeer.getWindow();
            if (ownerWindow != 0) {
                try {
                    XToolkit.awtLock();
                    // Set WM_TRANSIENT_FOR
                    if (focusLog.isLoggable(Level.FINE)) focusLog.fine("Setting transient on " + getWindow() + " for " + ownerWindow);                
                    XlibWrapper.XSetTransientFor(XToolkit.getDisplay(), getWindow(), ownerWindow);

                    // Set group leader
                    XWMHints hints = getWMHints();
                    hints.set_flags(hints.get_flags() | (int)XlibWrapper.WindowGroupHint);
                    hints.set_window_group(ownerWindow);
                    XlibWrapper.XSetWMHints(XToolkit.getDisplay(), getWindow(), hints.pData);
                }
                finally {
                    XToolkit.awtUnlock();
                }
            }
        }

        // Init warning window(for applets)
        if (((Window)target).getWarningString() != null) {
            warningWindow = new XWarningWindow((Window)target, getWindow());
        }

        setSaveUnder(true);
    }
    void updateFocusability() {
        try {
            XToolkit.awtLock();
            XWMHints hints = getWMHints();
            hints.set_flags(hints.get_flags() | (int)XlibWrapper.InputHint);
            hints.set_input(false/*isNativelyNonFocusableWindow() ? (0):(1)*/);
            XlibWrapper.XSetWMHints(XToolkit.getDisplay(), getWindow(), hints.pData);
        }
        finally {
            XToolkit.awtUnlock();
        }
    }

    public Insets getInsets() {
        return insets;
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

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleStateChange(int oldState, int newState) {
        postEvent(new WindowEvent((Window)target,
                                  WindowEvent.WINDOW_STATE_CHANGED,
                                  oldState, newState));
    }

    /**
     * DEPRECATED:  Replaced by getInsets().
     */
    public Insets insets() {
        return getInsets();
    }

    // DO NOT CALL ON TOOLKIT THREAD!!!
    boolean isFocusableWindow() {
        return ((Window)target).isFocusableWindow();
    }

    /**
     * Returns whether or not this window peer has native X window
     * configured as non-focusable window. It might happen if:
     * - Java window is non-focusable
     * - Java window is simple Window(not Frame or Dialog)    
     */
    boolean isNativelyNonFocusableWindow() {
        if (XToolkit.isToolkitThread()) {
            return isSimpleWindow() || !isFocusableWindow();
        } else {
            return isSimpleWindow() || !cachedFocusableWindow;
        }
    }


    public void handleWindowFocusInSync(long serial) {
        WindowEvent we = new WindowEvent((Window)target, WindowEvent.WINDOW_GAINED_FOCUS);
//          we.setSerial(serial);
        sendEvent(we);
    }
    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleWindowFocusIn(long serial) {
        WindowEvent we = new WindowEvent((Window)target, WindowEvent.WINDOW_GAINED_FOCUS);
//          we.setSerial(serial);
        /* wrap in Sequenced, then post*/
        postEvent(wrapInSequenced((AWTEvent) we));
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleWindowFocusOut(Window oppositeWindow, long serial) {
        WindowEvent we = new WindowEvent((Window)target, WindowEvent.WINDOW_LOST_FOCUS, oppositeWindow);
//          we.setSerial(serial);
        /* wrap in Sequenced, then post*/        
        postEvent(wrapInSequenced((AWTEvent) we));
    }
    public void handleWindowFocusOutSync(Window oppositeWindow, long serial) {
        WindowEvent we = new WindowEvent((Window)target, WindowEvent.WINDOW_LOST_FOCUS, oppositeWindow);
//          we.setSerial(serial);
        sendEvent(we);
    }

/* --- DisplayChangedListener Stuff --- */

    void resetTargetGC(Component target) {
        ComponentAccessor.resetGC(target);
    }

    /* Xinerama
     * called to check if we've been moved onto a different screen
     * Based on checkNewXineramaScreen() in awt_GraphicsEnv.c
     */
    public void checkIfOnNewScreen(Rectangle newBounds) {
        if (!XToolkit.localEnv.runningXinerama()) {
            return;
        }

        if (log.isLoggable(Level.FINEST)) {
            log.finest("XWindowPeer: Check if we've been moved to a new screen since we're running in Xinerama mode");
        }
        
        int area = newBounds.width * newBounds.height;
        int intAmt, vertAmt, horizAmt;
        int largestAmt = 0;
        int curScreenNum = ((X11GraphicsDevice)getGraphicsConfiguration().getDevice()).getScreen();
        int newScreenNum = 0;
        GraphicsDevice gds[] = XToolkit.localEnv.getScreenDevices();
        Rectangle screenBounds;

        for (int i = 0; i < gds.length; i++) {
            screenBounds = gds[i].getDefaultConfiguration().getBounds();
            if (newBounds.intersects(screenBounds)) {
                horizAmt = Math.min(newBounds.x + newBounds.width,
                                    screenBounds.x + screenBounds.width) -
                           Math.max(newBounds.x, screenBounds.x);
                vertAmt = Math.min(newBounds.y + newBounds.height,
                                   screenBounds.y + screenBounds.height)-
                          Math.max(newBounds.y, screenBounds.y);
                intAmt = horizAmt * vertAmt;
                if (intAmt == area) {
                    // Completely on this screen - done!
                    newScreenNum = i;
                    break;
                }
                if (intAmt > largestAmt) {
                    largestAmt = intAmt;
                    newScreenNum = i;
                }
            }
        }
        if (newScreenNum != curScreenNum) {
            if (log.isLoggable(Level.FINEST)) {
                log.finest("XWindowPeer: Moved to a new screen");
            }
            draggedToNewScreen(newScreenNum);
        }
    }

    /* Xinerama
     * called to update our GC when dragged onto another screen
     */
    public void draggedToNewScreen(int screenNum) {
        final int finalScreenNum = screenNum;

        SunToolkit.executeOnEventHandlerThread((Component)target, new Runnable()
            {
                public void run() {
                    displayChanged(finalScreenNum);
                }
            });
    }

    /* Xinerama
     * called to update our GC when dragged onto another screen
     */
    public void displayChanged(int screenNum) {
        // update our GC
        resetLocalGC(screenNum);         /* upcall to MCanvasPeer */
        resetTargetGC(target);           /* call Window.resetGC() via native */
        //propagate to children
        super.displayChanged(screenNum); /* upcall to XPanelPeer */
    }

    /*
     * Overridden to check if we need to update our GraphicsDevice/Config
     * Added for 4934052.
     */
    public void handleConfigureNotifyEvent(long ptr) {
        // TODO: We create an XConfigureEvent every time we override
        // handleConfigureNotify() - too many!
        XConfigureEvent xe = new XConfigureEvent(ptr);
        checkIfOnNewScreen(new Rectangle(xe.get_x(),
                                         xe.get_y(),  
                                         xe.get_width(),  
                                         xe.get_height()));

        // Don't call super until we've handled a screen change.  Otherwise
        // there could be a race condition in which a ComponentListener could
        // see the old screen.
        super.handleConfigureNotifyEvent(ptr);
    }

    final boolean focusAllowedFor() {
        if (isNativelyNonFocusableWindow()) {
            return false;
        }

        Iterator iter = XWM.getWM().getProtocols(XModalityProtocol.class).iterator();
        boolean res = true;
        while (iter.hasNext() && res) {
            XModalityProtocol proto = (XModalityProtocol)iter.next();
            res = res & !proto.isBlocked(null, this);
        }
        return res;
    }

    public void handleFocusEvent(long ptr) {
        XFocusChangeEvent xfe = new XFocusChangeEvent(ptr);
        FocusEvent fe;   
        focusLog.log(Level.FINE, "{0}", new Object[] {xfe});
        if (isEventDisabled(xfe)) {
            return;
        }
        if (xfe.get_type() == XlibWrapper.FocusIn)
        {
            // If this window is non-focusable don't post any java focus event
            if (focusAllowedFor()) {
                if (xfe.get_mode() == XlibWrapper.NotifyNormal // Normal notify
                    || xfe.get_mode() == XlibWrapper.NotifyWhileGrabbed) // Alt-Tab notify
                {
                    handleWindowFocusIn(xfe.get_serial());
                }
            }
        }
        else
        {
            if (xfe.get_mode() == XlibWrapper.NotifyNormal // Normal notify
                || xfe.get_mode() == XlibWrapper.NotifyWhileGrabbed) // Alt-Tab notify
            {
                // If this window is non-focusable don't post any java focus event
                if (!isNativelyNonFocusableWindow()) {
                    XWindowPeer oppositeXWindow = (XWindowPeer)XToolkit.windowToXWindow(xGetInputFocus());
                    Object oppositeTarget = (oppositeXWindow!=null)? oppositeXWindow.getTarget() : null;
                    Window oppositeWindow = null;
                    if (oppositeTarget instanceof Window) {
                        oppositeWindow = (Window)oppositeTarget;
                    }
                    // Check if opposite window is non-focusable. In that case we don't want to
                    // post any event.
                    if (oppositeXWindow != null && oppositeXWindow.isNativelyNonFocusableWindow()) {
                        return;
                    }
                    // If opposite window is disabled because of modality don't mark as
                    // opposite in a focus event
                    if (oppositeXWindow != null && !oppositeXWindow.focusAllowedFor()) {
                        oppositeXWindow = null;
                    }
                    if (this == oppositeXWindow) {
                        oppositeWindow = null;
                    }
                    handleWindowFocusOut(oppositeWindow, xfe.get_serial());
                }
            }
        }
    }
  
    public void dispatchEvent(XAnyEvent ev) {
        int type = ev.get_type();  
        switch (type)
        {
          case (int) XlibWrapper.FocusIn:
          case (int) XlibWrapper.FocusOut:
              handleFocusEvent(ev.pData);
              break;
        }
        super.dispatchEvent(ev);
    }

    void setSaveUnder(boolean state) {}

    public void toFront() {
        if (isOverrideRedirect() && mustControlStackPosition) {
            mustControlStackPosition = false;
            removeRootPropertyEventDispatcher();
        }
        if (isVisible()) {
            super.toFront();
            if (isFocusableWindow() && !isWithdrawn()) {
                requestInitialFocus();
            }
        } else {
            setVisible(true);
        }
    } 

    public void toBack() {
        try {
            XToolkit.awtLock();
            if(!isOverrideRedirect()) {
                XlibWrapper.XLowerWindow(XToolkit.getDisplay(), getWindow());
            }else{
                lowerOverrideRedirect();
            }
        }
        finally {
            XToolkit.awtUnlock();
        }
    }
    private void lowerOverrideRedirect() {
        //
        // make new hash of toplevels of all windows from 'windows' hash.
        // FIXME: do not call them "toplevel" as it is misleading.
        //
        HashSet toplevels = new HashSet();
        Iterator it = windows.iterator();
        XWindowPeer xp = null;
        long topl = 0, mytopl = 0;

        while( it.hasNext() ) {
            xp = (XWindowPeer)(it.next());
            topl = getToplevelWindow( xp.getWindow() );
            if( xp.equals( this ) ) {
                mytopl = topl;
            }
            if( topl > 0 )
                toplevels.add( new Long( topl ) );
        }
        
        //
        // find in the root's tree:
        // (1) my toplevel, (2) lowest java toplevel, (3) desktop
        // We must enforce (3), (1), (2) order, upward; 
        // note that nautilus on the next restacking will do (1),(3),(2).
        //
        long laux,     wDesktop = -1, wBottom = -1;
        int  iMy = -1, iDesktop = -1, iBottom = -1;
        int i = 0;
        XQueryTree xqt = new XQueryTree(XToolkit.getDefaultRootWindow());
        try {
            if( xqt.execute() > 0 ) {
                int nchildren = xqt.get_nchildren();
                long children = xqt.get_children();
                for(i = 0; i < nchildren; i++) {
                    laux = Native.getWindow(children, i);
                    if( laux == mytopl ) {
                        iMy = i;
                    }else if( isDesktopWindow( laux ) ) {
                        // we need topmost desktop of them all.
                        iDesktop = i;
                        wDesktop = laux;
                    }else if(iBottom < 0 && 
                             toplevels.contains( new Long(laux) ) && 
                             laux != mytopl) {
                        iBottom = i;
                        wBottom = laux;
                    }
                }
            }
        
            if( (iMy < iBottom || iBottom < 0 )&& iDesktop < iMy)
                return; // no action necessary
            
            long to_restack = Native.allocateLongArray(2);    
            Native.putLong(to_restack, 0, wBottom);
            Native.putLong(to_restack, 1,  mytopl);
            XlibWrapper.XRestackWindows(XToolkit.getDisplay(), to_restack, 2);
            XlibWrapper.unsafe.freeMemory(to_restack);
        
        
            if( !mustControlStackPosition ) {
                mustControlStackPosition = true;
                // add root window property listener:
                // somebody (eg nautilus desktop) may obscure us
                addRootPropertyEventDispatcher();
            }
        } finally {
            xqt.dispose();
        }
    }
    /**
        Get XID of closest to root window in a given window hierarchy.
        FIXME: do not call it "toplevel" as it is misleading.
        On error return 0.
    */
    private long getToplevelWindow( long w ) {
        long wi = w, ret, root;
        do {
            ret = wi;
            XQueryTree qt = new XQueryTree(wi);
            try {
                if (qt.execute() == 0) {
                    return 0;
                }
                root = qt.get_root();
                wi = qt.get_parent();
            } finally {
                qt.dispose();
            }

        } while (wi != root);
        
        return ret;
    }
    private boolean isDesktopWindow( long wi ) {
        return XWM.getWM().isDesktopWindow( wi );
    }

    public void updateAlwaysOnTop() {
          XWM.getWM().setLayer(this, 
                               ((Window)target).isAlwaysOnTop() ? 
                               XLayerProtocol.LAYER_ALWAYS_ON_TOP :
                               XLayerProtocol.LAYER_NORMAL);
    }
    
    private void promoteDefaultPosition() {
        if (((Window)target).isLocationByPlatform()) {
            synchronized (getAWTLock()) {
                Rectangle bounds = getBounds();
                XSizeHints hints = getHints();
                setSizeHints(hints.get_flags() & ~(USPosition | PPosition), 
                             bounds.x, bounds.y, bounds.width, bounds.height);
            }
         }
    }
    
    public void setVisible(boolean vis) {
        updateFocusability();
        promoteDefaultPosition();
        super.setVisible(vis);
    }

    boolean isSimpleWindow() {
        return !(target instanceof Frame || target instanceof Dialog);
    }
    boolean hasWarningWindow() {
        return warningWindow != null;
    }

    // The height of menu bar window
    int getMenuBarHeight() {
        return 0;
    }

    // The height of area used to display Applet's warning about securit
    int getWarningWindowHeight() {
        if (warningWindow != null) {
            return warningWindow.getHeight();
        } else {
            return 0;
        }
    }

    // Called when shell changes its size and requires children windows
    // to update their sizes appropriately
    void updateChildrenSizes() {
        if (warningWindow != null) {
            warningWindow.reshape(0, getMenuBarHeight(), getSize().width, warningWindow.getHeight());
        }
    }

    boolean isOverrideRedirect() {
        return true;
    }

    final boolean isOLWMDecorBug() {
        return XWM.getWMID() == XWM.OPENLOOK_WM &&
            winAttr.nativeDecor == false;
    }

    public void dispose() {
        windows.remove(this);
        if (warningWindow != null) {
            warningWindow.destroy();
        }
        removeRootPropertyEventDispatcher();
        mustControlStackPosition = false;
        super.dispose();        
    }
    boolean isResizable() {
        return winAttr.isResizable;
    }

    public void handleVisibilityEvent(long ptr) {
        super.handleVisibilityEvent(ptr);
        XVisibilityEvent ve = new XVisibilityEvent(ptr);
        winAttr.visibilityState = ve.get_state();
        if (ve.get_state() == XlibWrapper.VisibilityUnobscured) {
            // raiseInputMethodWindow
        }
    }

    public void handlePropertyNotify(long event_ptr) {
        super.handlePropertyNotify(event_ptr);
        XPropertyEvent ev = new XPropertyEvent(event_ptr);
        if (ev.get_atom() == XWM.XA_WM_STATE.getAtom()) {
            // State has changed, invalidate saved value
            stateChanged = true;
            stateChanged(ev.get_time(), savedState, getWMState());
        }
    }    
    void handleRootPropertyNotify(IXAnyEvent aev) {
        XPropertyEvent ev = new XPropertyEvent( aev.getPData() );
        if( mustControlStackPosition &&
            ev.get_atom() == XAtom.get("_NET_CLIENT_LIST_STACKING").getAtom()){
            // Restore stack order unhadled/spoiled by WM or some app (nautilus).
            // As of now, don't use any generic machinery: just 
            // do toBack() again.
            if(isOverrideRedirect()) {
                toBack();
            }    
        } 
    }

    public void handleMapNotifyEvent(long ptr) {
        super.handleMapNotifyEvent(ptr);
        if (!isNativelyNonFocusableWindow()) {            
            requestInitialFocus();
        }
        updateAlwaysOnTop();
    }

    protected void requestInitialFocus() {
        xRequestFocus();
    }

    /**
     * Override this methods to get notifications when top-level window state changes. The state is
     * meant in terms of ICCCM: WithdrawnState, IconicState, NormalState
     */
    protected void stateChanged(long time, int oldState, int newState) {
    }


    /*
     * XmNiconic and Map/UnmapNotify (that XmNiconic relies on) are
     * unreliable, since mapping changes can happen for a virtual desktop
     * switch or MacOS style shading that became quite popular under X as
     * well.  Yes, it probably should not be this way, as it violates
     * ICCCM, but reality is that quite a lot of window managers abuse
     * mapping state.
     */
    int getWMState() {
        if (stateChanged) {
            WindowPropertyGetter getter = 
                new WindowPropertyGetter(window, XWM.XA_WM_STATE, 0, 1, false, 
                                         XWM.XA_WM_STATE);
            try {
                int status = getter.execute();
                if (status != XlibWrapper.Success || getter.getData() == 0) {
                    return XlibWrapper.WithdrawnState;
                }
            
                if (getter.getActualType() != XWM.XA_WM_STATE.getAtom() && getter.getActualFormat() != 32) {
                    return XlibWrapper.WithdrawnState;
                }
                savedState = (int)Native.getCard32(getter.getData());
                stateChanged = false;
            } finally {
                getter.dispose();
            }
        }
        return savedState;
    }

    boolean isWithdrawn() {
        return getWMState() == XlibWrapper.WithdrawnState;
    }

    boolean hasDecorations(int decor) {
        if (!winAttr.nativeDecor) {
            return false;
        }
        else {
            int myDecor = winAttr.decorations;
            boolean hasBits = ((myDecor & decor) == decor);
            if ((myDecor & XWindowAttributesData.AWT_DECOR_ALL) != 0)
                return !hasBits;
            else
                return hasBits;
        }
    }

    protected void setModalBlocked(XDialogPeer dialog, boolean blocked) {
        synchronized(getAWTLock()) { // State lock should always be after awtLock
            synchronized(getStateLock()) {
                if (blocked) {
                    log.log(Level.FINE, "{0} is blocked by {1}", new Object[] { this, dialog});
                    modalBlocker = dialog;
                    XAtomList protocols = wm_protocols.getAtomListPropertyList(this);
                    log.log(Level.FINER, "Setting protocols {0}", new Object[] { protocols });
                    wm_protocols.setAtomListProperty(this, protocols); 
                    XToolkit.XSync();
                } else {
                    if (dialog != modalBlocker) {
                        throw new IllegalStateException("Trying to unblock window blocked by another dialog");
                    }
                    modalBlocker = null;            
                    wm_protocols.setAtomListProperty(this, getWMProtocols());
                }
            }
        }
    }

    /**
     * Returns a collection of peers of java Window objects
     */
    static Collection getAllWindows() {
        return Collections.unmodifiableSet(windows);
    }

    /**
     * Returns a collection of window peers which are not blocked by any modal dialog
     */
    static Collection getAllUnblockedWindows() {
        LinkedList wins = new LinkedList();
        Iterator iter = windows.iterator();
        while (iter.hasNext()) {
            XWindowPeer win = (XWindowPeer)iter.next();
            if (!win.isModalBlocked()) {
                wins.add(win);
            }
        }
        return wins;
    }

    public boolean isModalBlocked() {
        return modalBlocker != null;
    }

    public final boolean isModalExclude() {
        return modalExclude;
    }

    public boolean requestWindowFocus() {
        focusLog.fine("Request for window focus");
        // If this is Frame or Dialog we can't assure focus request success - but we still can try
        // If this is Window and its owner Frame is active we can be sure request succedded.        
        Window win = (Window)target;
        Window owner = win.getOwner();
        if (owner.isActive()) {
            focusLog.fine("Parent window is active - generating focus for this window");
            handleWindowFocusInSync(-1);
            return true;
        } else {
            focusLog.fine("Parent window is not active");
        }
        if (owner.getPeer() instanceof XDecoratedPeer) {
            XDecoratedPeer wpeer = (XDecoratedPeer)owner.getPeer();
            if (wpeer.requestWindowFocus(this)) {
                focusLog.fine("Parent window accepted focus request - generating focus for this window");
                return true;
            }
        }
        focusLog.fine("Denied - parent window is not active and didn't accept focus request");
        return false;
    }

    public void xSetVisible(boolean visible) {
        try {
            XToolkit.awtLock();
            this.visible = visible;
            if (visible) {
                XlibWrapper.XMapRaised(XToolkit.getDisplay(), getWindow());
            } else {
                XlibWrapper.XUnmapWindow(XToolkit.getDisplay(), getWindow());
            }
            XlibWrapper.XFlush(XToolkit.getDisplay());
        }
        finally {
            XToolkit.awtUnlock();
        }
    }

    private int dropTargetCount = 0;

    public synchronized void addDropTarget() {
        if (dropTargetCount == 0) {
            long window = getWindow();
            if (window != 0) {
                XDropTargetRegistry.getRegistry().registerDropSite(window);
            }
        }
        dropTargetCount++;
    }

    public synchronized void removeDropTarget() {
        dropTargetCount--;
        if (dropTargetCount == 0) {
            long window = getWindow();
            if (window != 0) {
                XDropTargetRegistry.getRegistry().unregisterDropSite(window);
            }
        }
    }
    void addRootPropertyEventDispatcher() {
        if( rootPropertyEventDispatcher == null ) {
            rootPropertyEventDispatcher = new XEventDispatcher() {
                public void dispatchEvent(IXAnyEvent ev) {
                    if( ev.get_type() == PropertyNotify ) {
                        handleRootPropertyNotify( ev );
                    }
                }
            };
            XlibWrapper.XSelectInput( XToolkit.getDisplay(),
                                      XToolkit.getDefaultRootWindow(),
                                      XlibWrapper.PropertyChangeMask);
            XToolkit.addEventDispatcher(XToolkit.getDefaultRootWindow(), 
                                                rootPropertyEventDispatcher);
        }
    }
    void removeRootPropertyEventDispatcher() {
        if( rootPropertyEventDispatcher != null ) {
            XToolkit.removeEventDispatcher(XToolkit.getDefaultRootWindow(), 
                                                rootPropertyEventDispatcher);
            rootPropertyEventDispatcher = null;
        }
    }
}
