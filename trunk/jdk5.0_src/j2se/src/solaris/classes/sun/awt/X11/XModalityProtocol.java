/*
 * @(#)XModalityProtocol.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.awt.X11;

public interface XModalityProtocol {
    /**
     * Sets modality mode on the dialog.
     * Returns true if the call to this function made dialog
     * visible, otherwise returns false
     */
    boolean setModal(XDialogPeer dialog, boolean modal);

    /**
     * Returns whether or not modal dialog blocks win.
     * If dialog is null checks whether or not this windows
     * is blocked by any dialog
     */
    boolean isBlocked(XDialogPeer dialog, XWindowPeer win);
}
