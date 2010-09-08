/*
 * @(#)XCheckboxMenuItemPeer.java	1.14 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

class XCheckboxMenuItemPeer extends XMenuItemPeer implements CheckboxMenuItemPeer {

    /************************************************
     *
     * Data members
     *
     ************************************************/

    /*
     * CheckboxMenuItem's fields
     */
    private final static Field f_state;
    static {
        f_state = XToolkit.getField(CheckboxMenuItem.class, "state");
    }

    /************************************************
     *
     * Construction
     *
     ************************************************/
    XCheckboxMenuItemPeer(CheckboxMenuItem target) {
        super(target);
    }

    /************************************************
     *
     * Implementaion of interface methods
     *
     ************************************************/

    //Prom CheckboxMenuItemtPeer
    public void setState(boolean t) {
        repaintIfShowing();
    }

    /************************************************
     *
     * Access to target's fields
     *
     ************************************************/
    boolean getTargetState() {
        MenuItem target = getTarget();
        if (target == null) {
            return false;
        }
        try {
            return f_state.getBoolean(target);
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        return false; 
    }

    /************************************************
     *
     * Utility functions
     *
     ************************************************/
    
    /**
     * Toggles state and generates ItemEvent
     */
    void action(final long when) {
        XToolkit.executeOnEventHandlerThread((CheckboxMenuItem)getTarget(), new Runnable() {
                public void run() {
                    doToggleState(when);
                }
            });
    }

    
    /************************************************
     *
     * Private
     *
     ************************************************/
    private void doToggleState(long when) {
        CheckboxMenuItem cb = (CheckboxMenuItem)getTarget();
        boolean newState = !getTargetState();
        cb.setState(newState);
        ItemEvent e = new ItemEvent(cb, 
                                    ItemEvent.ITEM_STATE_CHANGED,
                                    getTargetLabel(), 
                                    getTargetState() ? ItemEvent.SELECTED : ItemEvent.DESELECTED);
        XWindow.postEventStatic(e);
        //WToolkit does not post ActionEvent when clicking on menu item
        //MToolkit _does_ post.
        //Fix for 5005195 MAWT: CheckboxMenuItem fires action events  
        //Events should not be fired
        //XWindow.postEventStatic(new ActionEvent(cb, ActionEvent.ACTION_PERFORMED,
        //                                        getTargetActionCommand(), when,
        //                                        0));
    }

} // class XCheckboxMenuItemPeer
