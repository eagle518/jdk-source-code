/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved. Use is
 * subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.peer.SystemTrayPeer;
import java.util.logging.Logger;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import sun.awt.SunToolkit;
import sun.awt.AppContext;

public class XSystemTrayPeer implements SystemTrayPeer, XMSelectionListener {
    private static final Logger log = Logger.getLogger("sun.awt.X11.XSystemTrayPeer");
    
    SystemTray target;
    static XSystemTrayPeer peerInstance; // there is only one SystemTray peer per application

    private volatile boolean available;  
    private final XMSelection selection = new XMSelection("_NET_SYSTEM_TRAY"); 

    private static final Method addNotifyMethod = XToolkit.getMethod(TrayIcon.class, "addNotify", null);
    private static final Method removeNotifyMethod = XToolkit.getMethod(TrayIcon.class, "removeNotify", null);

    private static final int SCREEN = 0;
    private static final XAtom _NET_SYSTEM_TRAY = XAtom.get("_NET_SYSTEM_TRAY_S" + SCREEN);
    private static final XAtom _XEMBED_INFO = XAtom.get("_XEMBED_INFO");
    private static final XAtom _NET_SYSTEM_TRAY_OPCODE = XAtom.get("_NET_SYSTEM_TRAY_OPCODE");
    private static final XAtom _NET_WM_ICON = XAtom.get("_NET_WM_ICON");
    private static final long SYSTEM_TRAY_REQUEST_DOCK = 0;

    XSystemTrayPeer(SystemTray target) {
        this.target = target;
        peerInstance = this;

        selection.addSelectionListener(this); 
 
        long selection_owner = selection.getOwner(SCREEN); 
        available = (selection_owner != XConstants.None); 
 
        log.fine(" check if system tray is available. selection owner: " + selection_owner); 
    }

    public void ownerChanged(int screen, XMSelection sel, long newOwner, long data, long timestamp) { 
        if (screen != SCREEN) { 
            return; 
        }
        if (!available) {
            available = true;
        } else {
            removeTrayPeers();
        }
        createTrayPeers();
    }

    public void ownerDeath(int screen, XMSelection sel, long deadOwner) {
        if (screen != SCREEN) {
            return;
        }
        if (available) {
            available = false;
            removeTrayPeers();
        }
    }

    public void selectionChanged(int screen, XMSelection sel, long owner, XPropertyEvent event) {
    }
    
    public Dimension getTrayIconSize() {
        return new Dimension(XTrayIconPeer.TRAY_ICON_HEIGHT, XTrayIconPeer.TRAY_ICON_WIDTH);
    }

    boolean isAvailable() {
        return available;
    }

    void dispose() {
        selection.removeSelectionListener(this);
    }
    
    // ***********************************************************************
    // ***********************************************************************

    void addTrayIcon(XTrayIconPeer tiPeer) throws AWTException {
        long selection_owner = selection.getOwner(SCREEN); 

        log.fine(" send SYSTEM_TRAY_REQUEST_DOCK message to owner: " + selection_owner); 

        if (selection_owner == XConstants.None) {
            throw new AWTException("TrayIcon couldn't be displayed.");
        }

        long tray_window = tiPeer.getWindow();
        long data[] = new long[] {XEmbedHelper.XEMBED_VERSION, XEmbedHelper.XEMBED_MAPPED};
        long data_ptr = Native.card32ToData(data);

        _XEMBED_INFO.setAtomData(tray_window, data_ptr, data.length);

        sendMessage(selection_owner, SYSTEM_TRAY_REQUEST_DOCK, tray_window, 0, 0);
    }

    void sendMessage(long win, long msg, long data1, long data2, long data3) {
        XClientMessageEvent xev = new XClientMessageEvent();

        try {
            xev.set_type(XlibWrapper.ClientMessage);
            xev.set_window(win);
            xev.set_format(32);
            xev.set_message_type(_NET_SYSTEM_TRAY_OPCODE.getAtom());
            xev.set_data(0, 0);
            xev.set_data(1, msg);
            xev.set_data(2, data1);
            xev.set_data(3, data2);
            xev.set_data(4, data3);

            XToolkit.awtLock();
            try {
                XlibWrapper.XSendEvent(XToolkit.getDisplay(), win, false,
                                       XlibWrapper.NoEventMask, xev.pData);
            } finally {
                XToolkit.awtUnlock();
            }
        } finally {
            xev.dispose();
        }      
    }

    static XSystemTrayPeer getPeerInstance() {
        return peerInstance;
    }
    
    private void createTrayPeers() {
        invokeOnEachTrayIcon(addNotifyMethod);
    }

    private void removeTrayPeers() {
        invokeOnEachTrayIcon(removeNotifyMethod);
    }

    private void invokeOnEachTrayIcon(final Method method) {
        Runnable runnable = new Runnable() {
                public void run() {
                    TrayIcon[] icons = target.getTrayIcons();
                    for (TrayIcon ti : icons) {
                        invokeMethod(method, ti, (Object[]) null);
                    }
                }
            };
        invokeOnEachAppContext(runnable);
    }

    private void invokeMethod(Method method, Object obj, Object[] args) {
        try{
            method.invoke(obj, args);
        } catch (InvocationTargetException e){
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }

    private void invokeOnEachAppContext(Runnable runnable) {
        for (AppContext appContext : AppContext.getAppContexts()) {
            SunToolkit.invokeLaterOnAppContext(appContext, runnable);
        }
    }

}
