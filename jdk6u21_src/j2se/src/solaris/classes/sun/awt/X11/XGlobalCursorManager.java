/*
 * @(#)XGlobalCursorManager.java	1.15 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.lang.reflect.Field;
import java.lang.reflect.Method;

import sun.awt.GlobalCursorManager;
import sun.awt.GlobalCursorManager.*;

public final class XGlobalCursorManager extends GlobalCursorManager {

    private static Field  field_pData;
    private static Field  field_type;
    private static Class  cursorClass;
    private static Method method_setPData;
    static {
        cursorClass = java.awt.Cursor.class;   
        field_pData = XToolkit.getField(cursorClass, "pData");
        field_type  = XToolkit.getField(cursorClass, "type");
        method_setPData = XToolkit.getMethod(cursorClass, "setPData", new Class[] {long.class});
        if (field_pData == null || field_type == null || method_setPData == null) {
            System.out.println("Unable to initialize XGlobalCursorManager: ");
            Thread.dumpStack(); 

        }
    }


    // cached nativeContainer
    private Component nativeContainer;  


    /**
     * The XGlobalCursorManager is a singleton.
     */
    private static XGlobalCursorManager manager;


    static GlobalCursorManager getCursorManager() {
        if (manager == null) {
            manager = new XGlobalCursorManager();
        }
        return manager;
    }

    /**
     * Should be called in response to a native mouse enter or native mouse
     * button released message. Should not be called during a mouse drag.
     */
    static void nativeUpdateCursor(Component heavy) {
        XGlobalCursorManager.getCursorManager().updateCursorLater(heavy);
    }


    protected void setCursor(Component comp, Cursor cursor, boolean useCache) {
        if (comp == null) {
            return;
        }

        CapableCursorAndComponent capable = null;
        Cursor cur = useCache ? cursor : (capable = CapableCursorAndComponent.getNew(comp)).cursor;
        Component capableComponent = (capable!=null ? capable.component : null);
        Component nc = useCache ? nativeContainer : getNativeContainer(comp);

        if (nc != null && nc.isDisplayable() && (nc.getPeer() instanceof XComponentPeer)) {
            nativeContainer = nc;

            ((XComponentPeer)nc.getPeer()).pSetCursor(cur, capableComponent);
            // in case of grab we do for Swing we need to update keep cursor updated
            // (we don't need this in case of AWT menus).  Window Manager consider
            // the grabber as a current window and use its cursor.  So we need to
            // change cursor on the grabber too.
            updateGrabbedCursor(cur);
        }
    }

    /**
     * Updates cursor on the grabber if it is window peer (i.e. current grab is for
     * Swing, not for AWT.
     */
    private static void updateGrabbedCursor(Cursor cur) {
        XBaseWindow target = XAwtState.getGrabWindow();
        if (target instanceof XWindowPeer) {
            XWindowPeer grabber = (XWindowPeer) target;
            grabber.pSetCursor(cur);
        }
    }

    protected void updateCursorOutOfJava() {
        // in case we have grabbed input for Swing we need to reset cursor
        // when mouse pointer is out of any java toplevel.
        // let's use default cursor for this.
        updateGrabbedCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
    }

    private Component getNativeContainer(Component comp) {
        while (comp != null && comp.isLightweight()) {
            comp = comp.getParent();
        }
        return comp;
    }
    
    protected void getCursorPos(Point p) {

        if (!((XToolkit)Toolkit.getDefaultToolkit()).getLastCursorPos(p)) {
            XToolkit.awtLock();
            try {
                long display = XToolkit.getDisplay();
                long root_window = XlibWrapper.RootWindow(display,
                                                          XlibWrapper.DefaultScreen(display)); 

                XlibWrapper.XQueryPointer(display, root_window, 
                                          XlibWrapper.larg1,
                                          XlibWrapper.larg2,
                                          XlibWrapper.larg3,
                                          XlibWrapper.larg4,
                                          XlibWrapper.larg5,
                                          XlibWrapper.larg6,
                                          XlibWrapper.larg7);

                p.x = (int) XlibWrapper.unsafe.getInt(XlibWrapper.larg3);
                p.y = (int) XlibWrapper.unsafe.getInt(XlibWrapper.larg4);
            } finally {
                XToolkit.awtUnlock();
            }
        }
    }
    protected  Component findHeavyweightUnderCursor() {
        return XAwtState.getComponentMouseEntered();
    }

    /*
     * two native methods to call corresponding methods in Container and
     * Component
     */
    protected  Component findComponentAt(Container con, int x, int y) {
        return con.findComponentAt(x,y);   
    }
   
    protected  Point getLocationOnScreen(Component c) {
        return c.getLocationOnScreen();
    }
 
    protected Component findHeavyweightUnderCursor(boolean useCache) {  
        return findHeavyweightUnderCursor();
    }


    private static final class CapableCursorAndComponent {
        final Cursor cursor;
        final Component component;

        private CapableCursorAndComponent( Cursor cursor, Component component ) {
            this.cursor = cursor;
            this.component = component;
        }

        static CapableCursorAndComponent getNew( Component comp ) {
            Component c = comp;
            while ((c != null) && !(c instanceof Window) &&
                c.isEnabled() && c.isVisible() && c.isDisplayable()) {
                c = c.getParent();
            }
            if (c instanceof Window) {
                if (c.isEnabled() && c.isVisible() && c.isDisplayable() && comp.isEnabled()) {
                    return new CapableCursorAndComponent( comp.getCursor(), comp );
                } else {
                    return new CapableCursorAndComponent(
                        Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR),
                        null);
                }
            } else if (c == null) {
                return new CapableCursorAndComponent( null, null );
            }
            return getNew(c.getParent());
        }
    }


    /* This methods needs to be called from within XToolkit.awtLock / XToolkit.awtUnlock section. */

    static long getCursor(Cursor c) {

        long pData = 0;
        int type = 0;
        try {
            pData = field_pData.getLong(c);
            type = field_type.getInt(c);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }

        if (pData != 0) return pData;
        
        long cursorType = 0;
        switch (type) {
          case Cursor.DEFAULT_CURSOR:
              cursorType = XlibWrapper.XC_left_ptr;
              break;
          case Cursor.CROSSHAIR_CURSOR:
              cursorType = XlibWrapper.XC_crosshair;
              break;
          case Cursor.TEXT_CURSOR:
              cursorType = XlibWrapper.XC_xterm;
              break;
          case Cursor.WAIT_CURSOR:
              cursorType = XlibWrapper.XC_watch;
              break;
          case Cursor.SW_RESIZE_CURSOR:
              cursorType = XlibWrapper.XC_bottom_left_corner;
              break;
          case Cursor.NW_RESIZE_CURSOR:
              cursorType = XlibWrapper.XC_top_left_corner;
              break;
          case Cursor.SE_RESIZE_CURSOR:
              cursorType = XlibWrapper.XC_bottom_right_corner;
              break;
          case Cursor.NE_RESIZE_CURSOR:
              cursorType = XlibWrapper.XC_top_right_corner;
              break;
          case Cursor.S_RESIZE_CURSOR:
              cursorType = XlibWrapper.XC_bottom_side;
              break;
          case Cursor.N_RESIZE_CURSOR:
              cursorType = XlibWrapper.XC_top_side;
              break;
          case Cursor.W_RESIZE_CURSOR:
              cursorType = XlibWrapper.XC_left_side;
              break;
          case Cursor.E_RESIZE_CURSOR:
              cursorType = XlibWrapper.XC_right_side;
              break;
          case Cursor.HAND_CURSOR:
              cursorType = XlibWrapper.XC_hand2;
              break;
          case Cursor.MOVE_CURSOR:
              cursorType = XlibWrapper.XC_fleur;
              break;
        }
        
        XToolkit.awtLock();
        try {
            pData =(long) XlibWrapper.XCreateFontCursor(XToolkit.getDisplay(),(int) cursorType);
        }
        finally {
            XToolkit.awtUnlock();
        }

        setPData(c,pData);
        return pData;
    }


    static void setPData(Cursor c, long pData) {
        try {
            method_setPData.invoke(c, pData);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
 
    }
}
