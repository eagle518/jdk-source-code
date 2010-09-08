/*
 * @(#)XKeyboardFocusManagerPeer.java	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.event.*;
import java.awt.*;
import java.util.logging.*;
import java.lang.reflect.*;
import java.awt.peer.*;
import sun.awt.PeerEvent;
import sun.awt.CausedFocusEvent;

public class XKeyboardFocusManagerPeer implements KeyboardFocusManagerPeer {
    private static final Logger focusLog = Logger.getLogger("sun.awt.X11.focus.XKeyboardFocusManagerPeer");
    KeyboardFocusManager manager;

    XKeyboardFocusManagerPeer(KeyboardFocusManager manager) {
        this.manager = manager;
    }

    private static Object lock = new Object() {};
    private static Component currentFocusOwner;
    private static Window currentFocusedWindow;

    static void setCurrentNativeFocusOwner(Component comp) {
        if (focusLog.isLoggable(Level.FINER)) focusLog.finer("Setting current native focus owner " + comp);
        synchronized(lock) {
            currentFocusOwner = comp;
        }
    }

    static void setCurrentNativeFocusedWindow(Window win) {
        if (focusLog.isLoggable(Level.FINER)) focusLog.finer("Setting current native focused window " + win);
        XWindowPeer from = null, to = null;

        synchronized(lock) {
            if (currentFocusedWindow != null) {
                from = (XWindowPeer)currentFocusedWindow.getPeer();
            }

            currentFocusedWindow = win;

            if (currentFocusedWindow != null) {
                to = (XWindowPeer)currentFocusedWindow.getPeer();
            }
        }

        if (from != null) {
            from.updateSecurityWarningVisibility();
        }
        if (to != null) {
            to.updateSecurityWarningVisibility();
        }
    }

    static Component getCurrentNativeFocusOwner() {
        synchronized(lock) {
            return currentFocusOwner;
        }        
    }

    static Window getCurrentNativeFocusedWindow() {
        synchronized(lock) {
            return currentFocusedWindow;
        }
    }

    public void setCurrentFocusedWindow(Window win) {
        setCurrentNativeFocusedWindow(win);
    }

    public Window getCurrentFocusedWindow() {
        return getCurrentNativeFocusedWindow();
    }
    
    public void setCurrentFocusOwner(Component comp) {
        setCurrentNativeFocusOwner(comp);
    }

    public Component getCurrentFocusOwner() {
        return getCurrentNativeFocusOwner();
    }

    public void clearGlobalFocusOwner(Window activeWindow) {
         if (activeWindow != null) {
            Component focusOwner = activeWindow.getFocusOwner();
            if (focusLog.isLoggable(Level.FINE)) focusLog.fine("Clearing global focus owner " + focusOwner);
            if (focusOwner != null) {
                XComponentPeer nativePeer = XComponentPeer.getNativeContainer(focusOwner);
                if (nativePeer != null) {
                    FocusEvent fl = new CausedFocusEvent(focusOwner, FocusEvent.FOCUS_LOST, false, null, 
                                                         CausedFocusEvent.Cause.CLEAR_GLOBAL_FOCUS_OWNER);
                    XWindow.sendEvent(fl);
                }
            }
        }
   }

    static boolean simulateMotifRequestFocus(Component lightweightChild, Component target, boolean temporary,
                                      boolean focusedWindowChangeAllowed, long time, CausedFocusEvent.Cause cause) 
    {
        if (lightweightChild == null) {
            lightweightChild = (Component)target;
        }
        Component currentOwner = XKeyboardFocusManagerPeer.getCurrentNativeFocusOwner();        
        if (currentOwner != null && currentOwner.getPeer() == null) {
            currentOwner = null;
        }
        if (focusLog.isLoggable(Level.FINER)) focusLog.finer("Simulating transfer from " + currentOwner + " to " + lightweightChild);
        FocusEvent  fg = new CausedFocusEvent(lightweightChild, FocusEvent.FOCUS_GAINED, false, currentOwner, cause);
        FocusEvent fl = null;
        if (currentOwner != null) {
            fl = new CausedFocusEvent(currentOwner, FocusEvent.FOCUS_LOST, false, lightweightChild, cause);
        }
        
        if (fl != null) {
            XWindow.sendEvent(fl);
        }
        XWindow.sendEvent(fg);
        return true;
    }

    static Method shouldNativelyFocusHeavyweightMethod;

    static int shouldNativelyFocusHeavyweight(Component heavyweight,
         Component descendant, boolean temporary,
         boolean focusedWindowChangeAllowed, long time, CausedFocusEvent.Cause cause)
    {
        if (shouldNativelyFocusHeavyweightMethod == null) {
            Class[] arg_types = 
                new Class[] { Component.class,
                              Component.class,
                              Boolean.TYPE,
                              Boolean.TYPE,
                              Long.TYPE,
                              CausedFocusEvent.Cause.class
            };

            shouldNativelyFocusHeavyweightMethod =
                XToolkit.getMethod(KeyboardFocusManager.class,
                                   "shouldNativelyFocusHeavyweight",
                                   arg_types);
        }
        Object[] args = new Object[] { heavyweight,
                                       descendant,
                                       Boolean.valueOf(temporary),
                                       Boolean.valueOf(focusedWindowChangeAllowed),
                                       Long.valueOf(time), cause};

        int result = XComponentPeer.SNFH_FAILURE;
        if (shouldNativelyFocusHeavyweightMethod != null) {
            try {
                result = ((Integer) shouldNativelyFocusHeavyweightMethod.invoke(null, args)).intValue();
            }
            catch (IllegalAccessException e) {
                assert false;
            }
            catch (InvocationTargetException e) {
                assert false;
            }
        }

        return result;
    }
}
