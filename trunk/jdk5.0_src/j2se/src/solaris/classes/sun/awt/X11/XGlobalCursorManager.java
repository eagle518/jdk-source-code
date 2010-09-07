/*
 * @(#)XGlobalCursorManager.java	1.8 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.lang.reflect.Field;

import sun.awt.GlobalCursorManager;
import sun.awt.GlobalCursorManager.*;

public final class XGlobalCursorManager extends GlobalCursorManager {

    private static Field  field_pData;
    private static Field  field_type;
    private static Class  cursorClass;

    static {
        cursorClass = java.awt.Cursor.class;   
        field_pData = XToolkit.getField(cursorClass, "pData");
        field_type  = XToolkit.getField(cursorClass, "type");
        if (field_pData == null || field_type == null) {
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

    Cursor cur = useCache ? cursor : getCapableCursor(comp); 

    Component nc = useCache ? nativeContainer : getNativeContainer(comp);

    if (nc != null && nc.isDisplayable() && (nc.getPeer() instanceof XComponentPeer)) {
        nativeContainer = nc;
            
            ((XComponentPeer)nc.getPeer()).pSetCursor(cur);
    }
    }
    
    private Component getNativeContainer(Component comp) {
    while (comp != null && comp.isLightweight()) {
        comp = comp.getParent();
    }
    return comp;
    }
    
    protected void getCursorPos(Point p) {
      
        try {
            XToolkit.awtLock();  
            long display = XToolkit.getDisplay();
            long root_window = XlibWrapper.RootWindow(display,
                    XlibWrapper.DefaultScreen(display)); 

            XlibWrapper.XQueryPointer(display,root_window,XlibWrapper.larg1,XlibWrapper.larg2,XlibWrapper.larg3,XlibWrapper.larg4,XlibWrapper.larg5,XlibWrapper.larg6,XlibWrapper.larg7);

            p.x = (int) XlibWrapper.unsafe.getInt(XlibWrapper.larg3);
            p.y = (int) XlibWrapper.unsafe.getInt(XlibWrapper.larg4);
        }
        finally {
            XToolkit.awtUnlock();
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

    private Cursor getCapableCursor(Component comp) {
    Component c = comp; 
    while ((c != null) && !(c instanceof Window) && 
           c.isEnabled() && c.isVisible() && c.isDisplayable()) {
        c = c.getParent();
    } 
    if (c instanceof Window) {
        return (c.isEnabled() && c.isVisible() && c.isDisplayable() && comp.isEnabled()) ? 
            comp.getCursor() : 
            Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR);
    } else if (c == null) {
        return null;
    }
    return getCapableCursor(c.getParent());
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
        
        try {
            XToolkit.awtLock();
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
            field_pData.setLong(c,pData);
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
 
    }
}
