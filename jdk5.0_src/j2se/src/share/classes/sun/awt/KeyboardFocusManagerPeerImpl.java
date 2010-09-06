/*
 * @(#)KeyboardFocusManagerPeerImpl.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt;

import java.awt.event.*;
import java.awt.*;
import java.util.logging.*;
import java.lang.reflect.*;
import java.awt.peer.*;

public class KeyboardFocusManagerPeerImpl implements KeyboardFocusManagerPeer {
    static native Window getNativeFocusedWindow();
    static native Component getNativeFocusOwner();
    static native void clearNativeGlobalFocusOwner(Window activeWindow);

    KeyboardFocusManagerPeerImpl(KeyboardFocusManager manager) {
    }

    public void setCurrentFocusedWindow(Window win) {
    }

    public Window getCurrentFocusedWindow() {
        return getNativeFocusedWindow();
    }
    
    public void setCurrentFocusOwner(Component comp) {
    }

    public Component getCurrentFocusOwner() {
        return getNativeFocusOwner();
    }    
    public void clearGlobalFocusOwner(Window activeWindow) {
        clearNativeGlobalFocusOwner(activeWindow);
    }

    static Method m_removeLastFocusRequest = null;
    public static void removeLastFocusRequest(Component heavyweight) {
        try {
            if (m_removeLastFocusRequest == null) {
                m_removeLastFocusRequest = SunToolkit.getMethod(KeyboardFocusManager.class, "removeLastFocusRequest",
                                                              new Class[] {Component.class});
            }
            m_removeLastFocusRequest.invoke(null, new Object[]{heavyweight});
        } catch (InvocationTargetException ite) {
            ite.printStackTrace();
        } catch (IllegalAccessException ex) {
            ex.printStackTrace();
        }        
    }
}
