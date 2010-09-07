/*
 * @(#)XModalStrategy.java	1.6 04/03/16
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.util.*;
import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;

/**
 * Abstract class incapsulating strategy for blocking windows by modal dialogs
 * Every modal dialog associates its behavior with one of the instances of this class
 * and when some windows is being asked to take focus it asks current active dialogs'
 * modal strategies if it is allowed to receive focus. Inbetween there is a modal 
 * protocol which might decide to override modal strategy decision based on system
 * prefernces and protocol specifics.
 * @since 1.5
 */
abstract class XModalStrategy {
    public static final int 
        MODALITY_APPLICATION_MODAL = 1,
        MODALITY_TREE_MODAL = 2,
        MODALITY_APPCONTEXT_MODAL = 4,
        MODALITY_SYSTEM_MODAL = 8,       
        MODALITY_TREE_SENSITIVE = 128,
        MODALITY_MOTIF_COMPAT = MODALITY_APPLICATION_MODAL;
        
    private static XModalStrategy appModal;

    private int hints;
    public int getHints() {
        return hints;
    }
    protected void setHints(int hints) {
        this.hints = hints;
    }


    /**
     * Returns instance of modal strategy supporting the requeste modality
     * flavors
     * @param strategy bit mask of modality flavors
     * @return modal strategy supporting requested flavors
     */
    static synchronized XModalStrategy getModalStrategy(int strategy) {
        // We currently support only one strategy - motif-compatible 
        // application modal strategy
        if (strategy == MODALITY_APPLICATION_MODAL) {
            if (appModal == null) {
                appModal = new XApplicationModalityTreeUnsensetive();
            }
            return appModal;
        }
        return null;
    }


    public abstract boolean isModalBlocked(XDialogPeer dialog, XWindowPeer win);
    public abstract void modalityStarts(XDialogPeer dialog);
    public abstract void modalityEnds(XDialogPeer dialog);

    /**
     * Represents modal strategy which blocks all windows
     * shown before this dialog. All windows shown after this dialog 
     * no matter to which hierarchy they belong are enabled.
     * @since 1.5
     */
    static class XApplicationModalityTreeUnsensetive extends XModalStrategy {
        protected XApplicationModalityTreeUnsensetive() {
            setHints(MODALITY_APPLICATION_MODAL);
        }

        public void modalityStarts(XDialogPeer dialog) {
            /**
             * Get the list of unblocked windows and block them marking
             * their blocker as <code>dialog</code>
             */
            synchronized(XToolkit.getAWTLock()) {
                Collection windowList = XWindowPeer.getAllUnblockedWindows();
                Iterator iter = windowList.iterator();
                while (iter.hasNext()) {
                    XWindowPeer win = (XWindowPeer)iter.next();
                    if (win == dialog || win.isModalExclude() || !win.isVisible() || dialog.isParentOf(win)) {
                        continue;
                    }
                    win.setModalBlocked(dialog, true);
                    dialog.addModalBlocked(win);
                }
            }
        }

        public void modalityEnds(XDialogPeer dialog) {
            /**
             * Get the list of windows blocked by this dialog and release them 
             */
            synchronized(XToolkit.getAWTLock()) {
                Collection windowList = dialog.getBlockedWindows();
                Iterator iter = windowList.iterator();
                while (iter.hasNext()) {
                    ((XWindowPeer)iter.next()).setModalBlocked(dialog, false);
                }
                dialog.clearBlockedWindows();
            }
        }

        public boolean isModalBlocked(XDialogPeer dialog, XWindowPeer win) {
            // We keep all windows updated of their blocked state so
            // all we need is to query this state
            return win.isModalBlocked();
        }
    }
}
