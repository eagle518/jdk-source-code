/*
 * @(#)XDialogPeer.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.util.*;
import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;

class XDialogPeer extends XDecoratedPeer implements DialogPeer {

    private Collection blocked = new LinkedList();
    private XModalStrategy modalStrategy;
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
        winAttr.isResizable =  true; //target.isResizable();
        winAttr.initialResizability =  target.isResizable();
        winAttr.title = target.getTitle();
        winAttr.initialState = XWindowAttributesData.NORMAL;
        winAttr.icon = null;

        // Should be dependant on target when different modal strategies API will be added
        modalStrategy = XModalStrategy.getModalStrategy(XModalStrategy.MODALITY_MOTIF_COMPAT);
    }    

    protected String getWMName() {
        if (winAttr.title == null || winAttr.title.trim().equals("")) {
            return " ";
        } else {
            return winAttr.title;
        }
    }

    public void setVisible(boolean vis) {        
        if (vis) {
            // For CDE should set modality before mapping
            if (((Dialog)target).isModal()) {
                Iterator iter = XWM.getWM().getProtocols(XModalityProtocol.class).iterator();
                while (iter.hasNext()) {
                    XModalityProtocol proto = (XModalityProtocol)iter.next();
                    if (proto.setModal(this, true)) {
                        // Already visible
                        return;
                    }
                }
            }
            super.setVisible(true);
        } else {
            if (((Dialog)target).isModal()) {
                Iterator iter = XWM.getWM().getProtocols(XModalityProtocol.class).iterator();
                while (iter.hasNext()) {
                    XModalityProtocol proto = (XModalityProtocol)iter.next();
                    if (proto.setModal(this, false)) {
                        // Already invisible
                        return;
                    }
                }
            }
            super.setVisible(false);
        }
    }
    public void handleMapNotifyEvent(long ptr) {
        super.handleMapNotifyEvent(ptr);
        // For _NET should set modality AFTER mapping
        if (((Dialog)target).isModal()) {
            Iterator iter = XWM.getWM().getProtocols(XModalityProtocol.class).iterator();
            while (iter.hasNext()) {
                XModalityProtocol proto = (XModalityProtocol)iter.next();
                if (proto.setModal(this, true)) {
                    // Already visible
                    return;
                }
            }
        }
    }
    public void addModalBlocked(XWindowPeer win) {
        blocked.add(win);
    }

    public Collection getBlockedWindows() {
        return Collections.unmodifiableCollection(blocked);
    }

    public void clearBlockedWindows() {
        blocked.clear();
    }

    public XModalStrategy getModalStrategy() {
        return modalStrategy;
    }

    protected Insets guessInsets() {
        if (isTargetUndecorated()) {
            return new Insets(0, 0, 0, 0);
        } else {
            return super.guessInsets();
        }
    }

    private boolean isTargetUndecorated() {
        if (undecorated != null) {
            return undecorated.booleanValue();
        } else {
            return ((Dialog)target).isUndecorated();
        }
    }

}
