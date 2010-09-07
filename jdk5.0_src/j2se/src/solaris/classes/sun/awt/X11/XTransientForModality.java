/*
 * @(#)XTransientForModality.java	1.2 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.awt.X11;

import java.util.logging.*;
import java.util.*;


/**
 * Represents implementation of modal protocol based on X WM_TRANSIENT_FOR property. 
 * Since this property is being set automatically for every window with non-null owner
 * protocol essentially does nothing other than passes appropriate requests to particular
 * modal strategies.
 * @since 1.5
 */
public class XTransientForModality extends XProtocol implements XModalityProtocol {
    private static Logger log = Logger.getLogger("sun.awt.X11.XTransientForModality");
    /**
     * Sets modality mode on the dialog.
     * Returns true if the call to this function already made window
     * visible and modal, otherwise returns false
     * @return: true if <code>dialog</code> has been made visible, false otherwise
     * @since 1.5
     */
    public boolean setModal(XDialogPeer dialog, boolean modal) {
        if (log.isLoggable(Level.FINE)) log.fine("Setting modality " + modal + " for " + dialog);
        XModalStrategy strat = dialog.getModalStrategy();
        if (modal) {
            strat.modalityStarts(dialog);
            return false;
        } else {
            strat.modalityEnds(dialog);
            return false;
        }
    }

    /**
     * Finds whether this window is blocked by any modal protocol
     * @return: true if window is blocked by this dialog, false otherwise
     * @since 1.5
     */
    public boolean isBlocked(XDialogPeer dialog, XWindowPeer win) {
        /**
         * Implementation depends both on modal strategy of active
         * modal dialogs and on system modal dialogs implementation
         * Grab modality blocks windows based on owner-child relation
         * which is assumed to be sub-case of any modal
         * strategy. Therefore, we leave actual checks to strategies.
         */
        if (dialog != null) {
            return dialog.getModalStrategy().isModalBlocked(dialog, win);
        } else {
            return win.isModalBlocked();
        }
    }

}
