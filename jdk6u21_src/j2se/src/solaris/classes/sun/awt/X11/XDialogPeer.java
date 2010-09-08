/*
 * @(#)XDialogPeer.java	1.32 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.util.*;
import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;

import sun.awt.*;

class XDialogPeer extends XDecoratedPeer implements DialogPeer {

    private Boolean undecorated;

    XDialogPeer(Dialog target) {
        super(target);
    }

    public void preInit(XCreateWindowParams params) {
        super.preInit(params);

        Dialog target = (Dialog)(this.target);
        undecorated = Boolean.valueOf(target.isUndecorated());
        winAttr.nativeDecor = !target.isUndecorated();
        if (winAttr.nativeDecor) {
            winAttr.decorations = winAttr.AWT_DECOR_ALL;
        } else {
            winAttr.decorations = winAttr.AWT_DECOR_NONE;
        }
        winAttr.functions = MWM_FUNC_ALL;
        winAttr.isResizable =  true; //target.isResizable();
        winAttr.initialResizability =  target.isResizable();
        winAttr.title = target.getTitle();
        winAttr.initialState = XWindowAttributesData.NORMAL;
    }    

    public void setVisible(boolean vis) {
        XToolkit.awtLock();
        try {
            Dialog target = (Dialog)this.target;
            if (vis) {
                if (target.getModalityType() != Dialog.ModalityType.MODELESS) {
                    if (!isModalBlocked()) {
                        XBaseWindow.ungrabInput();
                    }
                }
            } else {
                restoreTransientFor(this);
                prevTransientFor = null;
                nextTransientFor = null;
            }
        } finally {
            XToolkit.awtUnlock();
        }

        super.setVisible(vis);
    }

    protected Insets guessInsets() {
        if (isTargetUndecorated()) {
            return new Insets(0, 0, 0, 0);
        } else {
            return super.guessInsets();
        }
    }

    @Override
    boolean isTargetUndecorated() {
        if (undecorated != null) {
            return undecorated.booleanValue();
        } else {
            return ((Dialog)target).isUndecorated();
        }
    }

    int getDecorations() {
        int d = super.getDecorations();
        // remove minimize and maximize buttons for dialogs
        if ((d & MWM_DECOR_ALL) != 0) {
            d |= (MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE);
        } else {
            d &= ~(MWM_DECOR_MINIMIZE | MWM_DECOR_MAXIMIZE);
        }
        return d;
    }

    int getFunctions() {
        int f = super.getFunctions();
        // remove minimize and maximize functions for dialogs
        if ((f & MWM_FUNC_ALL) != 0) {
            f |= (MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE);
        } else {
            f &= ~(MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE);
        }
        return f;
    }

    public void blockWindows(java.util.List<Window> toBlock) {
        Vector<XWindowPeer> javaToplevels = null;
        XToolkit.awtLock();
        try {
            javaToplevels = XWindowPeer.collectJavaToplevels();
            for (Window w : toBlock) {
                XWindowPeer wp = (XWindowPeer)ComponentAccessor.getPeer(w);
                if (wp != null) {
                    wp.setModalBlocked((Dialog)target, true, javaToplevels);
                }
            }
        } finally {
            XToolkit.awtUnlock();
        }
    }

}
