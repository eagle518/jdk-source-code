/*
 * @(#)XMWMModality.java	1.2 03/12/19
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
public class XMWMModality extends XTransientForModality implements XModalityProtocol, MWMConstants {
    private static Logger log = Logger.getLogger("sun.awt.X11.XMWMModality");

    final XAtom XA_MWM_HINTS = XAtom.get(MWM_HINTS_ATOM_NAME);

    /**
     * Sets modality mode on the dialog.
     * Returns true if the call to this function already made window
     * visible and modal, otherwise returns false
     * @return: true if <code>dialog</code> has been made visible, false otherwise
     * @since 1.5
     */
    public boolean setModal(XDialogPeer dialog, boolean modal) {
        if (log.isLoggable(Level.FINE)) log.fine("Setting modality " + modal + " for " + dialog);
        if (modal) {
            XModalStrategy strat = dialog.getModalStrategy();
            if (strat.getHints() == XModalStrategy.MODALITY_APPLICATION_MODAL) {
                // Right now we support only this flavor of modality
                PropMwmHints hints = new PropMwmHints();
                if (XA_MWM_HINTS.getAtomData(dialog.getWindow(), hints.pData, PROP_MWM_HINTS_ELEMENTS)) {
                    hints.set_flags(hints.get_flags() | MWM_HINTS_INPUT_MODE);
                } else {
                    hints.set_flags(MWM_HINTS_INPUT_MODE);
                }
                log.log(Level.FINE, "Setting MWM_INPUT_FULL_APPLICATION_MODAL on {0}", new Object[] {dialog});
                hints.set_inputMode(MWM_INPUT_FULL_APPLICATION_MODAL);
        
                XA_MWM_HINTS.setAtomData(dialog.getWindow(), hints.pData, PROP_MWM_HINTS_ELEMENTS);
        
                hints.dispose();
            }
        } else {
            XModalStrategy strat = dialog.getModalStrategy();
            if (strat.getHints() == XModalStrategy.MODALITY_APPLICATION_MODAL) {
                PropMwmHints hints = new PropMwmHints();
                if (XA_MWM_HINTS.getAtomData(dialog.getWindow(), hints.pData, PROP_MWM_HINTS_ELEMENTS)) {
                    hints.set_flags(hints.get_flags() | MWM_HINTS_INPUT_MODE);
                } else {
                    hints.set_flags(MWM_HINTS_INPUT_MODE);
                }
                hints.set_inputMode(MWM_INPUT_MODELESS);
        
                log.log(Level.FINE, "Setting MWM_INPUT_FULL_APPLICATION_MODAL on {0}", new Object[] {dialog});
                XA_MWM_HINTS.setAtomData(dialog.getWindow(), hints.pData, PROP_MWM_HINTS_ELEMENTS);
        
                hints.dispose();                
            }
        }

        // Super will set blocked state based on strategy.
        super.setModal(dialog, modal);
        return false;
    }
}
