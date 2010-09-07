/*
 * @(#)XDnDDropTargetProtocol.java	1.9 04/03/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Point;

import java.awt.dnd.DnDConstants;

import java.awt.event.MouseEvent;

import java.io.IOException;

import sun.misc.Unsafe;

/**
 * XDropTargetProtocol implementation for XDnD protocol.
 *
 * @since 1.5
 */
class XDnDDropTargetProtocol extends XDropTargetProtocol {
    private static final Unsafe unsafe = XlibWrapper.unsafe;

    private long sourceWindow = 0;
    private long sourceWindowMask = 0;
    private int sourceProtocolVersion = 0;
    private int sourceActions = DnDConstants.ACTION_NONE;
    private long[] sourceFormats = null;
    private boolean trackSourceActions = false;
    private int userAction = DnDConstants.ACTION_NONE;
    private int sourceX = 0;
    private int sourceY = 0;
    private XWindow targetXWindow = null;

    protected XDnDDropTargetProtocol(XDropTargetProtocolListener listener) {
        super(listener);
    }

    /**
     * Creates an instance associated with the specified listener.
     *
     * @throws NullPointerException if listener is <code>null</code>.
     */
    static XDropTargetProtocol createInstance(XDropTargetProtocolListener listener) {
        return new XDnDDropTargetProtocol(listener);
    }

    public void registerDropTarget(long window) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        long data = Native.allocateLongArray(1);

        try {
            Native.putLong(data, 0, XDnDConstants.XDND_PROTOCOL_VERSION);

            XToolkit.WITH_XERROR_HANDLER(XWM.VerifyChangePropertyHandler);
            XDnDConstants.XA_XdndAware.setAtomData(window, XAtom.XA_ATOM, data, 1);
            XToolkit.RESTORE_XERROR_HANDLER();
            
            if (XToolkit.saved_error != null && 
                XToolkit.saved_error.get_error_code() != XlibWrapper.Success) {
                throw new XException("Cannot write XdndAware property");
            }
        } finally {
            unsafe.freeMemory(data);
            data = 0;
        }
    }

    public void unregisterDropTarget(long window) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        XDnDConstants.XA_XdndAware.DeleteProperty(window);
    }

    public void registerEmbedderDropSite(long embedder) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        boolean overriden = false;
        int version = 0;
        long proxy = 0;
        long newProxy = XDropTargetRegistry.getDnDProxyWindow();
        int status = 0;

        WindowPropertyGetter wpg1 = 
            new WindowPropertyGetter(embedder, XDnDConstants.XA_XdndAware, 0, 1,
                                     false, XlibWrapper.AnyPropertyType);

        try {
            status = wpg1.execute(XToolkit.IgnoreBadWindowHandler);

            if (status == XlibWrapper.Success && 
                wpg1.getData() != 0 && wpg1.getActualType() == XAtom.XA_ATOM) {
                
                overriden = true;
                version = (int)Native.getLong(wpg1.getData());
            }
        } finally {
            wpg1.dispose();
        }

        /* XdndProxy is not supported for prior to XDnD version 4 */
        if (overriden && version >= 4) {
            WindowPropertyGetter wpg2 = 
                new WindowPropertyGetter(embedder, XDnDConstants.XA_XdndProxy,
                                         0, 1, false, XAtom.XA_WINDOW);

            try {
                status = wpg2.execute(XToolkit.IgnoreBadWindowHandler);
                
                if (status == XlibWrapper.Success && 
                    wpg2.getData() != 0 && 
                    wpg2.getActualType() == XAtom.XA_WINDOW) {

                    proxy = Native.getLong(wpg2.getData());
                }
            } finally {
                wpg2.dispose();
            }

            if (proxy != 0) {
                WindowPropertyGetter wpg3 = 
                    new WindowPropertyGetter(proxy, XDnDConstants.XA_XdndProxy,
                                             0, 1, false, XAtom.XA_WINDOW);

                try {
                    status = wpg3.execute(XToolkit.IgnoreBadWindowHandler);

                    if (status != XlibWrapper.Success ||
                        wpg3.getData() == 0 ||
                        wpg3.getActualType() != XAtom.XA_WINDOW ||
                        Native.getLong(wpg3.getData()) != proxy) {
                        
                        proxy = 0;
                    } else {
                        WindowPropertyGetter wpg4 = 
                            new WindowPropertyGetter(proxy,
                                                     XDnDConstants.XA_XdndAware, 
                                                     0, 1, false,
                                                     XlibWrapper.AnyPropertyType);
                        
                        try {
                            status = wpg4.execute(XToolkit.IgnoreBadWindowHandler);
                            
                            if (status != XlibWrapper.Success || 
                                wpg4.getData() == 0 || 
                                wpg4.getActualType() != XAtom.XA_ATOM) {
                                
                                proxy = 0;
                            }
                        } finally {
                            wpg4.dispose();
                        } 
                    }
                } finally {
                    wpg3.dispose();
                }
            }
        }

        if (proxy == newProxy) {
            // Embedder already registered.
            return;
        }

        long data = Native.allocateLongArray(1);
        
        try {
            Native.putLong(data, 0, XDnDConstants.XDND_PROTOCOL_VERSION);

            /* The proxy window must have the XdndAware set, as XDnD protocol
               prescribes to check the proxy window for XdndAware. */ 
            XToolkit.WITH_XERROR_HANDLER(XWM.VerifyChangePropertyHandler);
            XDnDConstants.XA_XdndAware.setAtomData(newProxy, XAtom.XA_ATOM,
                                                   data, 1);
            XToolkit.RESTORE_XERROR_HANDLER();
            
            if (XToolkit.saved_error != null && 
                XToolkit.saved_error.get_error_code() !=
                XlibWrapper.Success) { 
                throw new XException("Cannot write XdndAware property");
            }

            Native.putLong(data, 0, newProxy);

            /* The proxy window must have the XdndProxy set to point to itself.*/ 
            XToolkit.WITH_XERROR_HANDLER(XWM.VerifyChangePropertyHandler);
            XDnDConstants.XA_XdndProxy.setAtomData(newProxy, XAtom.XA_WINDOW,
                                                   data, 1); 
            XToolkit.RESTORE_XERROR_HANDLER();
            
            if (XToolkit.saved_error != null && 
                XToolkit.saved_error.get_error_code() !=
                XlibWrapper.Success) { 
                throw new XException("Cannot write XdndProxy property");
            }

            Native.putLong(data, 0, XDnDConstants.XDND_PROTOCOL_VERSION);

            XToolkit.WITH_XERROR_HANDLER(XWM.VerifyChangePropertyHandler);
            XDnDConstants.XA_XdndAware.setAtomData(embedder, XAtom.XA_ATOM,
                                                   data, 1);
            XToolkit.RESTORE_XERROR_HANDLER();
            
            if (XToolkit.saved_error != null && 
                XToolkit.saved_error.get_error_code() !=
                XlibWrapper.Success) { 
                throw new XException("Cannot write XdndAware property");
            }

            Native.putLong(data, 0, newProxy);

            XToolkit.WITH_XERROR_HANDLER(XWM.VerifyChangePropertyHandler);
            XDnDConstants.XA_XdndProxy.setAtomData(embedder, XAtom.XA_WINDOW,
                                                   data, 1);
            XToolkit.RESTORE_XERROR_HANDLER();
            
            if (XToolkit.saved_error != null && 
                XToolkit.saved_error.get_error_code() !=
                XlibWrapper.Success) { 
                throw new XException("Cannot write XdndProxy property");
            }
        } finally {
            unsafe.freeMemory(data);
            data = 0;
        }

        putEmbedderRegistryEntry(embedder, overriden, version, proxy);
    }

    public void unregisterEmbedderDropSite(long embedder) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        EmbedderRegistryEntry entry = getEmbedderRegistryEntry(embedder);
        
        // Shouldn't happen.
        assert entry != null;
        if (entry == null) {
            return;
        }

        if (entry.isOverriden()) {
            long data = Native.allocateLongArray(1);
        
            try {
                Native.putLong(data, 0, entry.getVersion());

                XToolkit.WITH_XERROR_HANDLER(XWM.VerifyChangePropertyHandler);
                XDnDConstants.XA_XdndAware.setAtomData(embedder, XAtom.XA_ATOM,
                                                       data, 1);
                XToolkit.RESTORE_XERROR_HANDLER();
            
                if (XToolkit.saved_error != null && 
                    XToolkit.saved_error.get_error_code() !=
                    XlibWrapper.Success) { 
                    throw new XException("Cannot write XdndAware property");
                }

                Native.putLong(data, 0, (int)entry.getProxy());

                XToolkit.WITH_XERROR_HANDLER(XWM.VerifyChangePropertyHandler);
                XDnDConstants.XA_XdndProxy.setAtomData(embedder, XAtom.XA_WINDOW,
                                                       data, 1);
                XToolkit.RESTORE_XERROR_HANDLER();
            
                if (XToolkit.saved_error != null && 
                    XToolkit.saved_error.get_error_code() !=
                    XlibWrapper.Success) { 
                    throw new XException("Cannot write XdndProxy property");
                }
            } finally {
                unsafe.freeMemory(data);
                data = 0;
            }
        } else {
            XDnDConstants.XA_XdndAware.DeleteProperty(embedder);
            XDnDConstants.XA_XdndProxy.DeleteProperty(embedder);
        }
    }

    public boolean isProtocolSupported(long window) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        WindowPropertyGetter wpg1 = 
            new WindowPropertyGetter(window, XDnDConstants.XA_XdndAware, 0, 1,
                                     false, XlibWrapper.AnyPropertyType);

        try {
            int status = wpg1.execute(XToolkit.IgnoreBadWindowHandler);

            if (status == XlibWrapper.Success && 
                wpg1.getData() != 0 && wpg1.getActualType() == XAtom.XA_ATOM) {
                
                return true;
            } else {
                return false;
            }
        } finally {
            wpg1.dispose();
        }
    }

    private boolean processXdndEnter(XClientMessageEvent xclient) {
        long source_win = 0;
        long source_win_mask = 0;
        int protocol_version = 0;
        int actions = DnDConstants.ACTION_NONE;
        boolean track = true;
        long[] formats = null;

        if (getSourceWindow() != 0) {
            return false;
        }

        if (!(XToolkit.windowToXWindow(xclient.get_window()) instanceof XWindow)
            && getEmbedderRegistryEntry(xclient.get_window()) == null) {
            return false;
        }

        if (xclient.get_message_type() != XDnDConstants.XA_XdndEnter.getAtom()){ 
            return false;
        }
         
        protocol_version = 
            (int)((xclient.get_data(1) & XDnDConstants.XDND_PROTOCOL_MASK) >> 
                  XDnDConstants.XDND_PROTOCOL_SHIFT);

        /* XDnD compliance only requires supporting version 3 and up. */
        if (protocol_version < XDnDConstants.XDND_MIN_PROTOCOL_VERSION) {
            return false;
        }

        /* Ignore the source if the protocol version is higher than we support. */
        if (protocol_version > XDnDConstants.XDND_PROTOCOL_VERSION) {
            return false;
        }

        source_win = xclient.get_data(0);

        /* Extract the list of supported actions. */
        if (protocol_version < 2) {
            /* Prior to XDnD version 2 only COPY action was supported. */
            actions = DnDConstants.ACTION_COPY;
        } else {
            WindowPropertyGetter wpg =
                new WindowPropertyGetter(source_win,
                                         XDnDConstants.XA_XdndActionList, 
                                         0, 0xFFFF, false,
                                         XAtom.XA_ATOM);
            try {
                wpg.execute(XToolkit.IgnoreBadWindowHandler);

                if (wpg.getActualType() == XAtom.XA_ATOM && 
                    wpg.getActualFormat() == 32) {
                    long data = wpg.getData();

                    for (int i = 0; i < wpg.getNumberOfItems(); i++) {
                        actions |= 
                            XDnDConstants.getJavaActionForXDnDAction(Native.getLong(data, i));
                    }
                } else {
                    /* 
                     * According to XDnD protocol, XdndActionList is optional.
                     * If XdndActionList is not set we try to guess which actions are
                     * supported.
                     */
                    actions = DnDConstants.ACTION_COPY;
                    track = true;
                }
            } finally {
                wpg.dispose();
            }
        }

        /* Extract the available data types. */
        if ((xclient.get_data(1) & XDnDConstants.XDND_DATA_TYPES_BIT) != 0) {
            WindowPropertyGetter wpg =
                new WindowPropertyGetter(source_win,
                                         XDnDConstants.XA_XdndTypeList, 
                                         0, 0xFFFF, false,
                                         XAtom.XA_ATOM);
            try {
                wpg.execute(XToolkit.IgnoreBadWindowHandler);

                if (wpg.getActualType() == XAtom.XA_ATOM && 
                    wpg.getActualFormat() == 32) {
                    formats = Native.toLongs(wpg.getData(), 
                                             wpg.getNumberOfItems());
                } else {
                    formats = new long[0];
                }                    
            } finally {
                wpg.dispose();
            }
        } else {
            int countFormats = 0;
            long[] formats3 = new long[3];

            for (int i = 0; i < 3; i++) {
                long j;
                if ((j = xclient.get_data(2 + i)) != XlibWrapper.None) {
                    formats3[countFormats++] = j;
                }
            }

            formats = new long[countFormats];

            System.arraycopy(formats3, 0, formats, 0, countFormats);
        }

        assert Thread.holdsLock(XToolkit.getAWTLock());

        /* 
         * Select for StructureNotifyMask to receive DestroyNotify in case of source
         * crash.
         */
        XWindowAttributes wattr = new XWindowAttributes();
        try {
            XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
            int status = XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                          source_win, wattr.pData);
                
            XToolkit.RESTORE_XERROR_HANDLER();

            if (status == 0 || 
                (XToolkit.saved_error != null && 
                 XToolkit.saved_error.get_error_code() != XlibWrapper.Success)) {
                throw new XException("XGetWindowAttributes failed");
            }
             
            source_win_mask = wattr.get_your_event_mask();
        } finally {
            wattr.dispose();
        }
                
        XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
        XlibWrapper.XSelectInput(XToolkit.getDisplay(), source_win,
                                 source_win_mask | 
                                 XlibWrapper.StructureNotifyMask);
            
        XToolkit.RESTORE_XERROR_HANDLER();

        if (XToolkit.saved_error != null && 
            XToolkit.saved_error.get_error_code() != XlibWrapper.Success) {
            throw new XException("XSelectInput failed");
        }

        sourceWindow = source_win;
        sourceWindowMask = source_win_mask;
        sourceProtocolVersion = protocol_version;
        sourceActions = actions;
        sourceFormats = formats;
        trackSourceActions = track;

        return true;
    }

    private boolean processXdndPosition(XClientMessageEvent xclient) {
        long time_stamp = (int)XlibWrapper.CurrentTime;
        long xdnd_action = 0;
        int java_action = DnDConstants.ACTION_NONE;
        int x = 0;
        int y = 0;

        /* Ignore XDnD messages from all other windows. */
        if (sourceWindow != xclient.get_data(0)) {
            return false;
        }

        XWindow xwindow = null;
        {
            XBaseWindow xbasewindow = XToolkit.windowToXWindow(xclient.get_window());
            if (xbasewindow instanceof XWindow) {
                xwindow = (XWindow)xbasewindow;
            }
        }

        x = (int)(xclient.get_data(2) >> 16);
        y = (int)(xclient.get_data(2) & 0xFFFF);

        if (xwindow == null) {
            long receiver =
                XDropTargetRegistry.getRegistry().getEmbeddedDropSite( 
                    xclient.get_window(), x, y);

            if (receiver != 0) {
                XBaseWindow xbasewindow = XToolkit.windowToXWindow(receiver);
                if (xbasewindow instanceof XWindow) {
                    xwindow = (XWindow)xbasewindow;
                }
            }
        }

        if (xwindow != null) {
            /* Translate mouse position from root coordinates 
               to the target window coordinates. */
            Point p = xwindow.toLocal(x, y);
            x = p.x;
            y = p.y;
        }

        /* Time stamp - new in XDnD version 1. */
        if (sourceProtocolVersion > 0) {
            time_stamp = xclient.get_data(3);
        }

        /* User action - new in XDnD version 2. */
        if (sourceProtocolVersion > 1) {
            xdnd_action = xclient.get_data(4);
        } else {
            /* The default action is XdndActionCopy */
            xdnd_action = XDnDConstants.XA_XdndActionCopy.getAtom();
        }

        java_action = XDnDConstants.getJavaActionForXDnDAction(xdnd_action);

        if (trackSourceActions) {
            sourceActions |= java_action;
        }

        if (xwindow == null) {
            if (targetXWindow != null) {
                getProtocolListener().handleDropTargetNotification(targetXWindow, x, y,
                                                                   DnDConstants.ACTION_NONE, 
                                                                   sourceActions, 
                                                                   sourceFormats, 
                                                                   xclient.pData, 
                                                                   MouseEvent.MOUSE_EXITED);
            }
        } else {
            int java_event_id = 0;
            
            if (targetXWindow == null) {
                java_event_id = MouseEvent.MOUSE_ENTERED;
            } else {
                java_event_id = MouseEvent.MOUSE_DRAGGED;
            }

            getProtocolListener().handleDropTargetNotification(xwindow, x, y, 
                                                               java_action, 
                                                               sourceActions, 
                                                               sourceFormats, 
                                                               xclient.pData, 
                                                               java_event_id);
        }

        userAction = java_action;
        sourceX = x;
        sourceY = y;
        targetXWindow = xwindow;
                
        return true;
    }

    private boolean processXdndLeave(XClientMessageEvent xclient) {
        /* Ignore XDnD messages from all other windows. */
        if (sourceWindow != xclient.get_data(0)) {
            return false;
        }

        cleanup();

        return true;
    }

    private boolean processXdndDrop(XClientMessageEvent xclient) {
        /* Ignore XDnD messages from all other windows. */
        if (sourceWindow != xclient.get_data(0)) {
            return false;
        }

        if (targetXWindow != null) {
            getProtocolListener().handleDropTargetNotification(targetXWindow, 
                                                               sourceX, 
                                                               sourceY, 
                                                               userAction, 
                                                               sourceActions, 
                                                               sourceFormats, 
                                                               xclient.pData, 
                                                               MouseEvent.MOUSE_RELEASED);
        }

        return true;
    }

    public int getMessageType(XClientMessageEvent xclient) {
        long messageType = xclient.get_message_type();

        if (messageType == XDnDConstants.XA_XdndEnter.getAtom()) {
            return ENTER_MESSAGE;
        } else if (messageType == XDnDConstants.XA_XdndPosition.getAtom()) {
            return MOTION_MESSAGE;
        } else if (messageType == XDnDConstants.XA_XdndLeave.getAtom()) {
            return LEAVE_MESSAGE;
        } else if (messageType == XDnDConstants.XA_XdndDrop.getAtom()) {
            return DROP_MESSAGE;
        } else {
            return UNKNOWN_MESSAGE;
        }
    }

    protected boolean processClientMessageImpl(XClientMessageEvent xclient) {
        long messageType = xclient.get_message_type();
        
        if (messageType == XDnDConstants.XA_XdndEnter.getAtom()) {
            return processXdndEnter(xclient);
        } else if (messageType == XDnDConstants.XA_XdndPosition.getAtom()) {
            return processXdndPosition(xclient);
        } else if (messageType == XDnDConstants.XA_XdndLeave.getAtom()) {
            return processXdndLeave(xclient);
        } else if (messageType == XDnDConstants.XA_XdndDrop.getAtom()) {
            return processXdndDrop(xclient);
        } else {
            return false;
        }
    }

    protected void sendEnterMessageToToplevel(long toplevel, 
                                              XClientMessageEvent xclient) {
        XClientMessageEvent enter = new XClientMessageEvent();
        try {
            enter.set_type((int)XlibWrapper.ClientMessage);
            enter.set_window(toplevel);
            enter.set_format(32);
            enter.set_message_type(XDnDConstants.XA_XdndEnter.getAtom());
            /* XID of the source window */
            enter.set_data(0, xclient.get_data(0));
            /* flags */
            long data1 = sourceProtocolVersion << XDnDConstants.XDND_PROTOCOL_SHIFT;
            if (sourceFormats != null && sourceFormats.length > 3) {
                data1 |= XDnDConstants.XDND_DATA_TYPES_BIT;
            }
            enter.set_data(1, data1);
            enter.set_data(2, sourceFormats.length > 0 ? sourceFormats[0] : 0);
            enter.set_data(3, sourceFormats.length > 1 ? sourceFormats[1] : 0);
            enter.set_data(4, sourceFormats.length > 2 ? sourceFormats[2] : 0);

            forwardClientMessageToToplevel(toplevel, enter);
        } finally {
            enter.dispose();
        }
    }
    
    protected void sendLeaveMessageToToplevel(long toplevel, 
                                              XClientMessageEvent xclient) {
        XClientMessageEvent leave = new XClientMessageEvent();
        try {
            leave.set_type((int)XlibWrapper.ClientMessage);
            leave.set_window(toplevel);
            leave.set_format(32);
            leave.set_message_type(XDnDConstants.XA_XdndLeave.getAtom());
            /* XID of the source window */
            leave.set_data(0, xclient.get_data(0));
            /* flags */
            leave.set_data(1, 0);

            forwardClientMessageToToplevel(toplevel, leave);
        } finally {
            leave.dispose();
        }
    }

    public boolean sendResponse(long ctxt, int eventID, int action) {
        XClientMessageEvent xclient = new XClientMessageEvent(ctxt);

        if (xclient.get_message_type() !=
            XDnDConstants.XA_XdndPosition.getAtom()) {
            
            return false;
        }

	if (eventID == MouseEvent.MOUSE_EXITED) {
	    action = DnDConstants.ACTION_NONE;
	}	

        XClientMessageEvent msg = new XClientMessageEvent();
        try {
            msg.set_type((int)XlibWrapper.ClientMessage);
            msg.set_window(xclient.get_data(0));
            msg.set_format(32);
            msg.set_message_type(XDnDConstants.XA_XdndStatus.getAtom());
            /* target window */
            msg.set_data(0, xclient.get_window());
            /* flags */
            long flags = 0;
            if (action != DnDConstants.ACTION_NONE) {
                flags |= XDnDConstants.XDND_ACCEPT_DROP_FLAG;
            }
            msg.set_data(1, flags); 
            /* specify an empty rectangle */
            msg.set_data(2, 0); /* x, y */
            msg.set_data(3, 0); /* w, h */
            /* action accepted by the target */
            msg.set_data(4, XDnDConstants.getXDnDActionForJavaAction(action));

            synchronized (XToolkit.getAWTLock()) {
                XlibWrapper.XSendEvent(XToolkit.getDisplay(), 
                                       xclient.get_data(0), 
                                       false, XlibWrapper.NoEventMask, 
                                       msg.pData);
            }
        } finally {
            msg.dispose();
        }

        return true;
    }

    public Object getData(long ctxt, long format) 
      throws IllegalArgumentException, IOException {
        XClientMessageEvent xclient = new XClientMessageEvent(ctxt);
        long message_type = xclient.get_message_type();
        long time_stamp = XlibWrapper.CurrentTime;

        // NOTE: we assume that the source supports at least version 1, so we
        // can use the time stamp
        if (message_type == XDnDConstants.XA_XdndPosition.getAtom()) {
            time_stamp = xclient.get_data(3);
        } else if (message_type == XDnDConstants.XA_XdndDrop.getAtom()) {
            time_stamp = xclient.get_data(2);
        } else {
            throw new IllegalArgumentException();
        }

        return XDnDConstants.XDnDSelection.getData(format, time_stamp);
    }

    public boolean sendDropDone(long ctxt, boolean success, int dropAction) {
        XClientMessageEvent xclient = new XClientMessageEvent(ctxt);

        if (xclient.get_message_type() !=
            XDnDConstants.XA_XdndDrop.getAtom()) {
            return false;
        }
            
        /*
         * The XDnD protocol recommends that the target requests the special
         * target DELETE in case if the drop action is XdndActionMove. 
         */
        if (dropAction == DnDConstants.ACTION_MOVE && success) {

            long time_stamp = xclient.get_data(2);
            long xdndSelectionAtom = 
                XDnDConstants.XDnDSelection.getSelectionAtom().getAtom();

            synchronized (XToolkit.getAWTLock()) {
                XlibWrapper.XConvertSelection(XToolkit.getDisplay(), 
                                              xdndSelectionAtom, 
                                              XAtom.get("DELETE").getAtom(),
                                              XAtom.get("XAWT_SELECTION").getAtom(),
                                              XWindow.getXAWTRootWindow().getWindow(),
                                              time_stamp);
            }
        }

        XClientMessageEvent msg = new XClientMessageEvent();
        try {
            msg.set_type((int)XlibWrapper.ClientMessage);
            msg.set_window(xclient.get_data(0));
            msg.set_format(32);
            msg.set_message_type(XDnDConstants.XA_XdndFinished.getAtom());
            msg.set_data(0, xclient.get_window()); /* target window */
            msg.set_data(1, 0); /* flags */
            /* specify an empty rectangle */
            msg.set_data(2, 0);
            if (sourceProtocolVersion >= 5) {
                if (success) {
                    msg.set_data(1, XDnDConstants.XDND_ACCEPT_DROP_FLAG);
                }
                /* action performed by the target */
                msg.set_data(2, XDnDConstants.getXDnDActionForJavaAction(dropAction));
            }
            msg.set_data(3, 0); 
            msg.set_data(4, 0);

            synchronized (XToolkit.getAWTLock()) {
                XlibWrapper.XSendEvent(XToolkit.getDisplay(), 
                                       xclient.get_data(0), 
                                       false, XlibWrapper.NoEventMask, 
                                       msg.pData);
            }
        } finally {
            msg.dispose();
        }

        /*
         * Flush the buffer to guarantee that the drop completion event is sent 
         * to the source before the method returns.
         */
        synchronized (XToolkit.getAWTLock()) {
            XlibWrapper.XFlush(XToolkit.getDisplay());
        }

        /* Trick to prevent cleanup() from posting dragExit */
        targetXWindow = null;

        /* Cannot do cleanup before the drop finishes as we may need
           source protocol version to send drop finished message. */
        cleanup();
        return true;
    }

    public final long getSourceWindow() {
        return sourceWindow;
    }

    /**
     * Reset the state of the object.
     */
    public void cleanup() {
        // Clear the reference to this protocol.
        XDropTargetEventProcessor.reset();

        if (targetXWindow != null) {
            getProtocolListener().handleDropTargetNotification(targetXWindow, 0, 0,
                                                               DnDConstants.ACTION_NONE, 
                                                               sourceActions, 
                                                               sourceFormats, 0, 
                                                               MouseEvent.MOUSE_EXITED);
        }

        if (sourceWindow != 0) {
            synchronized (XToolkit.getAWTLock()) {
                XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
                XlibWrapper.XSelectInput(XToolkit.getDisplay(), sourceWindow,
                                         sourceWindowMask);
                XToolkit.RESTORE_XERROR_HANDLER();
            }
        }

        sourceWindow = 0;
        sourceWindowMask = 0;
        sourceProtocolVersion = 0;
        sourceActions = DnDConstants.ACTION_NONE;
        sourceFormats = null;
        trackSourceActions = false;
        userAction = DnDConstants.ACTION_NONE;
        sourceX = 0;
        sourceY = 0;
        targetXWindow = null;
    }

    public boolean isDragOverComponent() {
        return targetXWindow != null;
    }

    public void adjustEventForForwarding(XClientMessageEvent xclient, 
                                         EmbedderRegistryEntry entry) {
        /* Adjust the event to match the XDnD protocol version. */
        int version = entry.getVersion();
        if (xclient.get_message_type() == XDnDConstants.XA_XdndEnter.getAtom()) {
            int min_version = sourceProtocolVersion < version ?
                sourceProtocolVersion : version;
            long data1 = min_version << XDnDConstants.XDND_PROTOCOL_SHIFT;
            if (sourceFormats != null && sourceFormats.length > 3) {
                data1 |= XDnDConstants.XDND_DATA_TYPES_BIT;
            }
            xclient.set_data(1, data1);             
        }
    }
}
