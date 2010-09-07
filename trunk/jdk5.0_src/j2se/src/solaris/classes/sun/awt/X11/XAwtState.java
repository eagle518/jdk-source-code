/*
 * @(#)XAwtState.java	1.6 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * This class is a placeholder for all internal static objects that represent
 * system state. We keep our representation up-to-date with actual system
 * state by tracking events, such as X Focus, Component under cursor etc. 
 * All attributes should be static private with accessors to simpify change
 * tracking.
 */
package sun.awt.X11;

import java.awt.Component;
import java.lang.ref.WeakReference;

class XAwtState {
    /**
     * The mouse is over this component. 
     * If the component is not disabled, it received MOUSE_ENTERED but no MOUSE_EXITED.
     */
    private static WeakReference componentMouseEnteredRef = null;
    
    static void setComponentMouseEntered(Component component) {
        synchronized (XToolkit.getAWTLock()) {
            if (component == null) {
                componentMouseEnteredRef = null;
                return;
            }
            if (component != getComponentMouseEntered()) {
                componentMouseEnteredRef = new WeakReference(component);
            }
        }
    }

    static Component getComponentMouseEntered() {
        synchronized (XToolkit.getAWTLock()) {
            if (componentMouseEnteredRef == null) {
                return null;
            }
            return (Component)componentMouseEnteredRef.get();
        }
    }
    
    /**
     * The XBaseWindow is created with OwnerGrabButtonMask 
     * (see X vol. 1, 8.3.3.2) so inside the app Key, Motion, and Button events
     * are received by the window they actualy happened on, not the grabber. 
     * Then XBaseWindow dispatches them to the grabber. As a result 
     * XAnyEvent.get_window() returns actual window the event is originated, 
     * though the event is dispatched by  the grabber.
     */     
    private static boolean inManualGrab = false;
    
    static boolean isManualGrab() {
        return inManualGrab;
    }
 
    private static WeakReference grabWindowRef = null;
    
    /**
     * The X Active Grab overrides any other active grab by the same
     * client see XGrabPointer, XGrabKeyboard
     */
    static void setGrabWindow(XBaseWindow grabWindow) {
        setGrabWindow(grabWindow, false);
    }
    
    /**
     * Automatic passive grab doesn't override active grab see XGrabButton
     */
    static void setAutoGrabWindow(XBaseWindow grabWindow) {
        setGrabWindow(grabWindow, true);
    }
    
    private static void setGrabWindow(XBaseWindow grabWindow, boolean isAutoGrab) {
        synchronized (XToolkit.getAWTLock()) {        
            if (inManualGrab && isAutoGrab) {
                return;
            }
            inManualGrab = grabWindow != null && !isAutoGrab;
            if (grabWindow == null) {
                grabWindowRef = null;
                return;
            }
            if (grabWindow != getGrabWindow()) {
                grabWindowRef = new WeakReference(grabWindow);
            }
        }
    }

    static XBaseWindow getGrabWindow() {
        synchronized (XToolkit.getAWTLock()) {
            if (grabWindowRef == null) {
                return null;
            }
            XBaseWindow xbw = (XBaseWindow)grabWindowRef.get();
            if( xbw != null && xbw.isDisposed() ) {
                xbw = null;
                grabWindowRef = null;
            }else if( xbw == null ) {
                grabWindowRef = null;
            }    
            return xbw;
        }
    }    
}
