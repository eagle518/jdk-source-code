/*
 * @(#)MDialogPeer.java	1.62 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.motif;

import java.util.Vector;
import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import sun.awt.motif.MInputMethodControl;
import sun.awt.im.*;

class MDialogPeer extends MWindowPeer implements DialogPeer, MInputMethodControl {

    static Vector allDialogs = new Vector();

    MDialogPeer(Dialog target) {

        /* create MWindowPeer object */
        super();

        winAttr.nativeDecor = !target.isUndecorated();
        winAttr.initialFocus = true;
        winAttr.isResizable =  target.isResizable();
        winAttr.initialState = MWindowAttributes.NORMAL;
        winAttr.title = target.getTitle();
        winAttr.icon = null;
        if (winAttr.nativeDecor) {
            winAttr.decorations = winAttr.AWT_DECOR_ALL |
                                  winAttr.AWT_DECOR_MINIMIZE |
                                  winAttr.AWT_DECOR_MAXIMIZE;
        } else {
            winAttr.decorations = winAttr.AWT_DECOR_NONE;
        }
        /* create and init native component */
        init(target);
        allDialogs.addElement(this);
    }

    public void setTitle(String title) {
        pSetTitle(title);
    }

    protected void disposeImpl() {
        allDialogs.removeElement(this);
        super.disposeImpl();
    }

    // NOTE: This method is called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleMoved(int x, int y) {
        postEvent(new ComponentEvent(target, ComponentEvent.COMPONENT_MOVED));
    }

    public void show() {
        pShowModal( ((Dialog)target).isModal() );
        updateAlwaysOnTop(alwaysOnTop);
    }       


    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleIconify() {
// Note: These routines are necessary for Coaleseing of native implementations
//       As Dialogs do not currently send Iconify/DeIconify messages but
//       Windows/Frames do.  If this should be made consistent...to do so
//       uncomment the postEvent.
//       postEvent(new WindowEvent((Window)target, WindowEvent.WINDOW_ICONIFIED));
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void handleDeiconify() {
// Note: These routines are necessary for Coaleseing of native implementations
//       As Dialogs do not currently send Iconify/DeIconify messages but
//       Windows/Frames do. If this should be made consistent...to do so
//       uncomment the postEvent.
//       postEvent(new WindowEvent((Window)target, WindowEvent.WINDOW_DEICONIFIED));
    }

    public void blockWindows(java.util.List<Window> toBlock) {
        // do nothing
    }

    @Override
    final boolean isTargetUndecorated() {
        return ((Dialog)target).isUndecorated();
    }
}
