/* 
 * @(#)XMenuPeer.java	1.42 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;

import java.lang.reflect.Field;
import java.util.Vector;
import java.util.logging.*;            

public class XMenuPeer extends XMenuItemPeer implements MenuPeer {

    /************************************************
     *
     * Data members
     *
     ************************************************/
    private static Logger log = Logger.getLogger("sun.awt.X11.XMenuPeer");

    /**
     * Window that correspond to this menu
     */
    XMenuWindow menuWindow;


    /*
     * Menu's fields & methods
     */
    private final static Field f_items;

    static {
        f_items = XToolkit.getField(Menu.class, "items");
    }

    /************************************************
     *
     * Construction
     *
     ************************************************/
    XMenuPeer(Menu target) {
        super(target);
    }

    /** 
     * This function is called when menu is bound
     * to its container window. Creates submenu window
     * that fills its items vector while construction
     */
    void setContainer(XBaseMenuWindow container) {
        super.setContainer(container);
        menuWindow = new XMenuWindow(this);        
    }


    /************************************************
     *
     * Implementaion of interface methods
     *
     ************************************************/

    /*
     * From MenuComponentPeer
     */

    /**
     * Disposes menu window if needed
     */
    public void dispose() {
        if (menuWindow != null) {
            menuWindow.dispose();
        }
        super.dispose();
    }

    /**
     * Resets text metrics for this item, for its menu window
     * and for all descendant menu windows
     */
    public void setFont(Font font) {
        //TODO:We can decrease count of repaints here
        //and get rid of recursion
        resetTextMetrics();
        XMenuWindow menuWindow = getMenuWindow();
        if (menuWindow != null) {
            XMenuItemPeer[] items = menuWindow.copyItems();
            int itemCnt = items.length;
            for (int i = 0; i < itemCnt; i++) {
                items[i].setFont(font);
            }
        }
        repaintIfShowing();
    }

    /*
     * From MenuPeer
     */
    /**
     * addSeparator routines are not used 
     * in peers. Shared code invokes addItem("-") 
     * for adding separators
     */
    public void addSeparator() {
        if (log.isLoggable(Level.FINER)) log.finer("addSeparator is not implemented");
    }

    public void addItem(MenuItem item) {
        XMenuWindow menuWindow = getMenuWindow();
        if (menuWindow != null) {
            menuWindow.addItem(item);
        } else {
            if (log.isLoggable(Level.FINE)) {
                log.fine("Attempt to use XMenuWindowPeer without window");
            }
        }
    }

    public void delItem(int index) {
        XMenuWindow menuWindow = getMenuWindow();
        if (menuWindow != null) {
            menuWindow.delItem(index);
        } else {
            if (log.isLoggable(Level.FINE)) {
                log.fine("Attempt to use XMenuWindowPeer without window");
            }
        }
    }

    /************************************************
     *
     * Access to target's fields
     *
     ************************************************/
    Vector getTargetItems() {
        try {
            return (Vector)f_items.get(getTarget());
        } catch (IllegalAccessException iae) {
            iae.printStackTrace();
            return null;
        }
    }

    /************************************************
     *
     * Overriden behaviour
     *
     ************************************************/
    boolean isSeparator() {
        return false;
    }
    
    //Fix for 6180416: Shortcut keys are displayed against Menus on XToolkit  
    //Menu should always return null as shortcutText
    String getShortcutText() {
        return null;
    }

    /************************************************
     *
     * Utility functions
     *
     ************************************************/

    /**
     * Returns menu window of this menu or null
     * it this menu has no container and so its
     * window can't be created.
     */
    XMenuWindow getMenuWindow() {
        return menuWindow;
    }
       
}
