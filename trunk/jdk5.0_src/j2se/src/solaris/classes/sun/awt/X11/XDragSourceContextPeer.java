/*
 * @(#)XDragSourceContextPeer.java	1.10 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Component;
import java.awt.Cursor;
import java.awt.Window;

import java.awt.datatransfer.Transferable;

import java.awt.dnd.DnDConstants;
import java.awt.dnd.DragSourceDragEvent;
import java.awt.dnd.DragSourceDropEvent;
import java.awt.dnd.DragSourceEvent;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.InvalidDnDOperationException;

import java.util.Iterator;
import java.util.Map;

import sun.awt.SunToolkit;

import sun.awt.dnd.SunDragSourceContextPeer;
import sun.awt.dnd.SunDropTargetContextPeer;

/**
 * The XDragSourceContextPeer class is the class responsible for handling
 * the interaction between the XDnD/Motif DnD subsystem and Java drag sources.
 *
 * @since 1.5
 */
public final class XDragSourceContextPeer 
    extends SunDragSourceContextPeer implements XDragSourceProtocolListener {

    /* The events selected on the root window when the drag begins. */
    private static final int ROOT_EVENT_MASK = (int)XlibWrapper.ButtonMotionMask |
        (int)XlibWrapper.KeyPressMask | (int)XlibWrapper.KeyReleaseMask; 
    /* The events to be delivered during grab. */
    private static final int GRAB_EVENT_MASK = (int)XlibWrapper.ButtonPressMask |
        (int)XlibWrapper.ButtonMotionMask | (int)XlibWrapper.ButtonReleaseMask;

    /* The event mask of the root window before the drag operation starts. */
    private long rootEventMask = 0;
    private boolean dndInProgress = false;
    private boolean dragInProgress = false;
    private long dragRootWindow = 0;

    /* The protocol chosen for the communication with the current drop target. */
    private XDragSourceProtocol dragProtocol = null;
    /* The drop action chosen by the current drop target. */
    private int targetAction = DnDConstants.ACTION_NONE;
    /* The set of drop actions supported by the drag source. */
    private int sourceActions = DnDConstants.ACTION_NONE;
    /* The drop action selected by the drag source based on the modifiers state
       and the action selected by the current drop target. */
    private int sourceAction = DnDConstants.ACTION_NONE;
    /* The data formats supported by the drag source for the current drag
       operation. */
    private long[] sourceFormats = null;
    /* The XID of the root subwindow that contains the current target. */
    private long targetRootSubwindow = 0;
    /* The pointer location. */
    private int xRoot = 0;
    private int yRoot = 0;
    /* Keyboard modifiers state. */
    private int eventState = 0;

    /* The singleton instance. */
    private static final XDragSourceContextPeer theInstance = 
        new XDragSourceContextPeer(null);

    private XDragSourceContextPeer(DragGestureEvent dge) {
        super(dge);
    }

    static XDragSourceProtocolListener getXDragSourceProtocolListener() {
        return theInstance;
    }

    static XDragSourceContextPeer createDragSourceContextPeer(DragGestureEvent dge) 
      throws InvalidDnDOperationException { 
    theInstance.setTrigger(dge);
        return theInstance;
    }

    protected void startDrag(Transferable transferable, 
                             long[] formats, Map formatMap) {
        Component component = getTrigger().getComponent();
        Component c = null;
        XWindowPeer wpeer = null;

        for (c = component; c != null && !(c instanceof Window);
             c = ComponentAccessor.getParent_NoClientCode(c));

        if (c instanceof Window) {
            wpeer = (XWindowPeer)c.getPeer();
        }

        if (wpeer == null) {
            throw new InvalidDnDOperationException(
                "Cannot find top-level for the drag source component");
        }

        long xcursor = 0;
        long rootWindow = 0;
        long dragWindow = 0;
        long timeStamp = 0;

        /* Retrieve the X cursor for the drag operation. */
        {
            Cursor cursor = getCursor();
            if (cursor != null) {
                xcursor = XGlobalCursorManager.getCursor(cursor);
            }
        }

        synchronized(XToolkit.getAWTLock()) {        
            /* Determine the root window for the drag operation. */
            {
                long screen = XlibWrapper.XScreenNumberOfScreen(wpeer.getScreen());
                rootWindow = XlibWrapper.RootWindow(XToolkit.getDisplay(), screen);
            }

            dragWindow = XWindow.getXAWTRootWindow().getWindow();
        
            timeStamp = XToolkit.getCurrentServerTime();

            int dropActions = getDragSourceContext().getSourceActions();

            Iterator dragProtocols = XDragAndDropProtocols.getDragSourceProtocols();
            while (dragProtocols.hasNext()) {
                XDragSourceProtocol dragProtocol = (XDragSourceProtocol)dragProtocols.next();
                try {
                    dragProtocol.initializeDrag(dropActions, transferable,
                                                formatMap, formats);
                } catch (XException xe) {
                    throw (InvalidDnDOperationException)
                        new InvalidDnDOperationException().initCause(xe);
                }
            }

            /* Install X grabs. */
            {
                int status;
                XWindowAttributes wattr = new XWindowAttributes();
                try {
                    status = XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                              rootWindow, wattr.pData);
                
                    if (status == 0) {
                        throw new InvalidDnDOperationException("XGetWindowAttributes failed");
                    }
                
                    rootEventMask = wattr.get_your_event_mask();
                
                    XlibWrapper.XSelectInput(XToolkit.getDisplay(), rootWindow,
                                             rootEventMask | ROOT_EVENT_MASK);
                } finally {
                    wattr.dispose();
                }
                
                XBaseWindow.ungrabInput();
                /* Confine the pointer to the root window. */
                status = XlibWrapper.XGrabPointer(XToolkit.getDisplay(), rootWindow, 
                                                  0, GRAB_EVENT_MASK, 
                                                  XlibWrapper.GrabModeAsync,
                                                  XlibWrapper.GrabModeAsync,
                                                  rootWindow, xcursor, timeStamp);
                
                if (status != XlibWrapper.GrabSuccess) {
                    cleanup(timeStamp);
                    throwGrabFailureException("Cannot grab pointer", status);
                    return;
                }
                
                status = XlibWrapper.XGrabKeyboard(XToolkit.getDisplay(), rootWindow,
                                                   0, 
                                                   XlibWrapper.GrabModeAsync,
                                                   XlibWrapper.GrabModeAsync,
                                                   timeStamp);

                if (status != XlibWrapper.GrabSuccess) {
                    cleanup(timeStamp);
                    throwGrabFailureException("Cannot grab keyboard", status);
                    return;
                }
            }

            /* Update the global state. */
            dndInProgress = true;
            dragInProgress = true;
            dragRootWindow = rootWindow;
            sourceActions = dropActions;
            sourceFormats = formats;
        }

        /* This implementation doesn't use native context */
        setNativeContext(0);
        
        SunDropTargetContextPeer.setCurrentJVMLocalSourceTransferable(transferable);
    }

    /**
     * set cursor
     */

    public void setCursor(Cursor c) throws InvalidDnDOperationException {
        synchronized(XToolkit.getAWTLock()) {
            super.setCursor(c);
        }
    }

    protected void setNativeCursor(long nativeCtxt, Cursor c, int cType) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        if (c == null) {
            return;
        }
        
        long xcursor = XGlobalCursorManager.getCursor(c);   

        if (xcursor == 0) {
            return;
        }

        XlibWrapper.XChangeActivePointerGrab(XToolkit.getDisplay(),
                                             GRAB_EVENT_MASK,
                                             xcursor,
                                             XlibWrapper.CurrentTime);
    }

    protected boolean needsBogusExitBeforeDrop() {
        return false;
    }

    private void throwGrabFailureException(String msg, int grabStatus) 
      throws InvalidDnDOperationException {
        String msgCause = "";
        switch (grabStatus) {
        case XlibWrapper.GrabNotViewable:  msgCause = "not viewable";    break;
        case XlibWrapper.AlreadyGrabbed:   msgCause = "already grabbed"; break;
        case XlibWrapper.GrabInvalidTime:  msgCause = "invalid time";    break;
        case XlibWrapper.GrabFrozen:       msgCause = "grab frozen";     break;
        default:                           msgCause = "unknown failure"; break;
        }
        throw new InvalidDnDOperationException(msg + ": " + msgCause);
    }

    /**
     * The caller must own awtLock.
     */
    public void cleanup(long time) {
        if (dndInProgress) {
            if (dragProtocol != null) {
                dragProtocol.sendLeaveMessage(time);
            }

            if (targetAction != DnDConstants.ACTION_NONE) {
                dragExit(xRoot, yRoot);
            }

            dragDropFinished(false, DnDConstants.ACTION_NONE, xRoot, yRoot);
        }

        Iterator dragProtocols = XDragAndDropProtocols.getDragSourceProtocols();
        while (dragProtocols.hasNext()) {
            XDragSourceProtocol dragProtocol = (XDragSourceProtocol)dragProtocols.next();
            try {
                dragProtocol.cleanup();
            } catch (XException xe) {
                // Ignore the exception.
            }
        }

        dndInProgress = false;
        dragInProgress = false;
        dragRootWindow = 0;
        sourceFormats = null;
        sourceActions = DnDConstants.ACTION_NONE;
        sourceAction = DnDConstants.ACTION_NONE;
        eventState = 0;
        xRoot = 0;
        yRoot = 0;

        cleanupTargetInfo();

        removeDnDGrab(time);
    }

    /**
     * The caller must own awtLock.
     */
    private void cleanupTargetInfo() {
        targetAction = DnDConstants.ACTION_NONE;
        dragProtocol = null;
        targetRootSubwindow = 0;
    }

    private void removeDnDGrab(long time) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        XlibWrapper.XUngrabPointer(XToolkit.getDisplay(), time);
        XlibWrapper.XUngrabKeyboard(XToolkit.getDisplay(), time);

        /* Restore the root event mask if it was changed. */
        if ((rootEventMask | ROOT_EVENT_MASK) != rootEventMask &&
            dragRootWindow != 0) {

            XlibWrapper.XSelectInput(XToolkit.getDisplay(), 
                                     dragRootWindow, 
                                     rootEventMask);
        }

        rootEventMask = 0;
        dragRootWindow = 0;
    }

    private boolean processClientMessage(XClientMessageEvent xclient) {
        if (dragProtocol != null) {
            return dragProtocol.processClientMessage(xclient);
        }
        return false;
    }

    /**
     * Updates the source action according to the specified state.
     *
     * @returns true if the source
     */
    private boolean updateSourceAction(int state) {
        int action = SunDragSourceContextPeer.convertModifiersToDropAction(XWindow.getModifiers(state, 0, 0),
                                                                           sourceActions);
        if (sourceAction == action) {
            return false;
        } 
        sourceAction = action;
        return true;
    }

    /**
     * Returns the client window under the specified root subwindow.
     */
    private static long findClientWindow(long window) {
        WindowPropertyGetter wpg = 
            new WindowPropertyGetter(window, XWM.XA_WM_STATE, 0, 1, false, 
                                     XWM.XA_WM_STATE);
        
        try {
            wpg.execute(XToolkit.IgnoreBadWindowHandler);
            if (wpg.getActualType() == XWM.XA_WM_STATE.getAtom()) {
                return window;
            }
        } finally {
            wpg.dispose();
        }

        XQueryTree xqt = new XQueryTree(window);

        try {
            int status = xqt.execute(null);
            if (status == 0) {
                return 0;
            }

            long children = xqt.get_children();
            
            if (children == 0) {
                return 0;
            }

            int childrenCount = xqt.get_nchildren();

            for (int i = 0; i < childrenCount; i++) {
                long win = findClientWindow(Native.getLong(children, i));
                if (win != 0) {
                    return win;
                }
            }
            
            return 0;
        } finally {
            xqt.dispose();
        }
    }

    private void doUpdateTargetWindow(long subwindow, long time) {
        long clientWindow = 0;
        long proxyWindow = 0;
        XDragSourceProtocol protocol = null;
        boolean isReceiver = false;
        
        if (subwindow != 0) {
            clientWindow = findClientWindow(subwindow);
        }

        if (clientWindow != 0) {
            Iterator dragProtocols = XDragAndDropProtocols.getDragSourceProtocols();
            while (dragProtocols.hasNext()) {
                XDragSourceProtocol dragProtocol = (XDragSourceProtocol)dragProtocols.next();
                if (dragProtocol.attachTargetWindow(clientWindow, time)) {
                    protocol = dragProtocol;
                    break;
                }
            }
        }

        /* Update the global state. */
        dragProtocol = protocol;
        targetAction = DnDConstants.ACTION_NONE;    
        targetRootSubwindow = subwindow;
    }

    private void updateTargetWindow(XMotionEvent xmotion) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        int x = xmotion.get_x_root();
        int y = xmotion.get_y_root();
        long time = xmotion.get_time();
        long subwindow = xmotion.get_subwindow();

        /* 
         * If this event had occurred before the pointer was grabbed, 
         * query the server for the current root subwindow.
         */
        if (xmotion.get_window() != xmotion.get_root()) {
            XlibWrapper.XQueryPointer(XToolkit.getDisplay(), 
                                      xmotion.get_root(), 
                                      XlibWrapper.larg1,  // root
                                      XlibWrapper.larg2,  // subwindow
                                      XlibWrapper.larg3,  // x_root
                                      XlibWrapper.larg4,  // y_root
                                      XlibWrapper.larg5,  // x
                                      XlibWrapper.larg6,  // y
                                      XlibWrapper.larg7); // modifiers  
            subwindow = Native.getLong(XlibWrapper.larg2);
        }

        if (targetRootSubwindow != subwindow) {
            if (dragProtocol != null) {
                dragProtocol.sendLeaveMessage(time);
        
                /*
                 * Neither Motif DnD nor XDnD provide a mean for the target
                 * to notify the source that the pointer exits the drop site
                 * that occupies the whole top level.
                 * We detect this situation and post dragExit.
                 */
                if (targetAction != DnDConstants.ACTION_NONE) {
                    dragExit(x, y);
                }
            }

            /* Update the global state. */
            doUpdateTargetWindow(subwindow, time);

            if (dragProtocol != null) {
                dragProtocol.sendEnterMessage(sourceFormats, 
                                              sourceAction,
                                              sourceActions, 
                                              time);
            }
        }
    }

    private void processMouseMove(XMotionEvent xmotion) {
        if (!dragInProgress) {
            return;
        }
        if (xRoot != xmotion.get_x_root() || yRoot != xmotion.get_y_root()) {
            xRoot = xmotion.get_x_root();
            yRoot = xmotion.get_y_root();

            postDragSourceDragEvent(targetAction, 
                                    XWindow.getModifiers(xmotion.get_state(),0,0),
                                    xRoot, yRoot, DISPATCH_MOUSE_MOVED);
        }

        if (eventState != xmotion.get_state()) {
            if (updateSourceAction(xmotion.get_state()) && dragProtocol != null) {
                postDragSourceDragEvent(targetAction, 
                                        XWindow.getModifiers(xmotion.get_state(),0,0),
                                        xRoot, yRoot, DISPATCH_CHANGED);
            }
            eventState = xmotion.get_state();
        }

        updateTargetWindow(xmotion);

        if (dragProtocol != null) {
            dragProtocol.sendMoveMessage(xmotion.get_x_root(),
                                         xmotion.get_y_root(),
                                         sourceAction, sourceActions,
                                         xmotion.get_time());
        }
    }

    private void processDrop(XButtonEvent xbutton) {
        try {
            dragProtocol.initiateDrop(xbutton.get_x_root(), 
                                      xbutton.get_y_root(), 
                                      sourceAction, sourceActions,
                                      xbutton.get_time());
        } catch (XException e) {
            cleanup(xbutton.get_time());
        }
    }

    /**
     * The caller must own awtLock.
     *
     * @returns true if the even was processed and shouldn't be passed along.
     */
    private boolean doProcessEvent(XAnyEvent ev) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        if (!dndInProgress) {
            return false;
        }

        switch (ev.get_type()) {
        case (int)XlibWrapper.ClientMessage: {
            XClientMessageEvent xclient = new XClientMessageEvent(ev.pData);
            return processClientMessage(xclient);
        }
        case (int)XlibWrapper.DestroyNotify: {
            XDestroyWindowEvent xde = new XDestroyWindowEvent(ev.pData);

            /* Target crashed during drop processing - cleanup. */
            if (!dragInProgress && 
                dragProtocol != null && 
                xde.get_window() == dragProtocol.getTargetWindow()) { 
                cleanup(XlibWrapper.CurrentTime);
                return true;
            }
            /* Pass along */
            return false;
        }
        }

        if (!dragInProgress) {
            return false;
        }

        /* Process drag-only messages. */
        switch (ev.get_type()) {
        case (int)XlibWrapper.KeyRelease:
        case (int)XlibWrapper.KeyPress: {
            XKeyEvent xkey = new XKeyEvent(ev.pData);
            long keysym = XlibWrapper.XKeycodeToKeysym(XToolkit.getDisplay(),
                                                       xkey.get_keycode(), 0);
            switch ((int)keysym) {
            case (int)XKeySymConstants.XK_Escape: {
                if (ev.get_type() == (int)XlibWrapper.KeyRelease) {
                    cleanup(xkey.get_time());
                }
                break;
            }
            case (int)XKeySymConstants.XK_Control_R:
            case (int)XKeySymConstants.XK_Control_L:
            case (int)XKeySymConstants.XK_Shift_R:
            case (int)XKeySymConstants.XK_Shift_L: {
                XlibWrapper.XQueryPointer(XToolkit.getDisplay(), 
                                          xkey.get_root(),
                                          XlibWrapper.larg1,  // root
                                          XlibWrapper.larg2,  // subwindow
                                          XlibWrapper.larg3,  // x_root
                                          XlibWrapper.larg4,  // y_root
                                          XlibWrapper.larg5,  // x
                                          XlibWrapper.larg6,  // y
                                          XlibWrapper.larg7); // modifiers  
                int state = (int)Native.getLong(XlibWrapper.larg7);
                xkey.set_state(state);
                //It's safe to use key event as motion event since we use only their common fields.
                processMouseMove(new XMotionEvent(xkey.pData));
                break;
            }
            }
            return true;
        }
        case (int)XlibWrapper.ButtonPress:
            return true;
        case (int)XlibWrapper.MotionNotify: 
            processMouseMove(new XMotionEvent(ev.pData));
            return true;
        case (int)XlibWrapper.ButtonRelease: {
            XButtonEvent xbutton = new XButtonEvent(ev.pData);
            /*
             * On some X servers it could happen that ButtonRelease coordinates
             * differ from the latest MotionNotify coordinates, so we need to
             * process it as a mouse motion.
             * MotionNotify differs from ButtonRelease only in is_hint member, but
             * we never use it, so it is safe to cast to MotionNotify. 
             */
            processMouseMove(new XMotionEvent(ev.pData));
            if (xbutton.get_button() == XlibWrapper.Button1) {
                removeDnDGrab(xbutton.get_time());
                dragInProgress = false;
                if (dragProtocol != null && targetAction != DnDConstants.ACTION_NONE) {
                    /*
                     * ACTION_NONE indicates that either the drop target rejects the
                     * drop or it haven't responded yet. The latter could happen in
                     * case of fast drag, slow target-server connection or slow
                     * drag notifications processing on the target side.
                     */
                    processDrop(xbutton);
                } else {
                    cleanup(xbutton.get_time());
                }
            }
            return true;
        }
        }

        return false;
    }

    static boolean processEvent(XAnyEvent ev) {
        synchronized(XToolkit.getAWTLock()) {
            try {
                return theInstance.doProcessEvent(ev);
            } catch (XException e) {
                e.printStackTrace();
                return false;
            }
        }
    }

    /* XDragSourceProtocolListener implementation */

    public void handleDragReply(int action) {
        // NOTE: we have to use the current pointer location, since
        // the target didn't specify the coordinates for the reply.
        handleDragReply(action, xRoot, yRoot);
    }

    public void handleDragReply(int action, int x, int y) {
        // NOTE: we have to use the current modifiers state, since
        // the target didn't specify the modifiers state for the reply. 
        handleDragReply(action, xRoot, yRoot, XWindow.getModifiers(eventState,0,0));
    }

    public void handleDragReply(int action, int x, int y, int modifiers) {
        if (action == DnDConstants.ACTION_NONE &&
            targetAction != DnDConstants.ACTION_NONE) {
            dragExit(x, y);
        } else if (action != DnDConstants.ACTION_NONE) {
            int type = 0;
            
            if (targetAction == DnDConstants.ACTION_NONE) {
                type = SunDragSourceContextPeer.DISPATCH_ENTER;
            } else {
                type = SunDragSourceContextPeer.DISPATCH_MOTION;
            }

            // Note that we use the modifiers state a
            postDragSourceDragEvent(action, modifiers, x, y, type);
        }

        targetAction = action;
    }

    public void handleDragFinished() {
        /* Assume that the drop was successful. */
        handleDragFinished(true);
    }

    public void handleDragFinished(boolean success) {
        /* Assume that the performed drop action is the latest drop action
           accepted by the drop target. */
        handleDragFinished(true, targetAction);
    }

    public void handleDragFinished(boolean success, int action) {
        // NOTE: we have to use the current pointer location, since
        // the target didn't specify the coordinates for the reply.
        handleDragFinished(success, action, xRoot, yRoot);
    }

    public void handleDragFinished(boolean success, int action, int x, int y) {
        dragDropFinished(success, action, x, y);

        dndInProgress = false;
        cleanup(XlibWrapper.CurrentTime);
    }
}
