/*
 * @(#)XKeyboardFocusManagerPeer.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.event.*;
import java.awt.*;
import java.util.logging.*;
import java.lang.reflect.*;
import java.awt.peer.*;
import sun.awt.PeerEvent;

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
        synchronized(lock) {
            currentFocusedWindow = win;
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
                    FocusEvent fl = new FocusEvent(focusOwner, FocusEvent.FOCUS_LOST);
                    XWindow.sendEvent(fl);
                }
            }
        }
   }

    static boolean simulateMotifRequestFocus(Component lightweightChild, Component target, boolean temporary,
                                      boolean focusedWindowChangeAllowed, long time) 
    {
        if (lightweightChild == null) {
            lightweightChild = (Component)target;
        }
        Component currentOwner = XKeyboardFocusManagerPeer.getCurrentNativeFocusOwner();        
        if (currentOwner != null && currentOwner.getPeer() == null) {
            currentOwner = null;
        }
        if (focusLog.isLoggable(Level.FINER)) focusLog.finer("Simulating transfer from " + currentOwner + " to " + lightweightChild);
        FocusEvent  fg = new FocusEvent(lightweightChild, FocusEvent.FOCUS_GAINED, false, currentOwner );
        FocusEvent fl = null;
        if (currentOwner != null) {
            fl = new FocusEvent(currentOwner, FocusEvent.FOCUS_LOST, false, lightweightChild);
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
         boolean focusedWindowChangeAllowed, long time)
    {
        if (shouldNativelyFocusHeavyweightMethod == null) {
            Class[] arg_types = new Class[] { Component.class,
                                              Component.class,
                                              Boolean.TYPE,
                                              Boolean.TYPE,
                                              Long.TYPE };

            shouldNativelyFocusHeavyweightMethod =
                XToolkit.getMethod(KeyboardFocusManager.class,
                                   "shouldNativelyFocusHeavyweight",
                                   arg_types);
        }
        Object[] args = new Object[] { heavyweight,
                                       descendant,
                                       Boolean.valueOf(temporary),
                                       Boolean.valueOf(focusedWindowChangeAllowed),
                                       new Long(time)};

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
