/*
 * @(#)ModalityHelper.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.awt.Dialog;
import java.awt.Toolkit;
import java.lang.reflect.*;
import java.security.*;
import java.util.*;
import sun.awt.*;

import sun.plugin2.applet.*;
import sun.plugin2.os.windows.*;
import sun.plugin2.util.SystemUtil;

/** Helps abstract away certain modality-related operations that may
    not be available on all JRE versions or supported platforms. */

public class ModalityHelper {
    private ModalityHelper() {}

    /** Attempts to install a ModalityListener in order to support
        calling the given ModalityInterface. The ModalityListener is a
        SunToolkit-specific concept introduced only in Java SE 6 so
        fallback mechanisms need to be used if it is not available.
        Returns true if the installation succeeded, false if not
        (indicating therefore that the fallback mechanism of
        subclassing the concrete EmbeddedFrame class and overriding
        notifyModalBlocked must be used). */
    public static boolean installModalityListener(ModalityInterface modality) {
        // We do this in a try/catch because these APIs aren't present on older JDKs (yet)
        try {
            Toolkit tk = Toolkit.getDefaultToolkit();
            if (tk instanceof SunToolkit) {
                SunToolkit stk = (SunToolkit) tk;
                stk.addModalityListener(new PluginModalityListener(modality));
                return true;
            }
        } catch (Throwable t) {
        }
        return false;
    }

    //----------------------------------------------------------------------
    // Support for determining which manager to block when a system
    // modal dialog is shown
    //

    // Indicates which manager has most recently performed an
    // operation, such as showing a security dialog, upon whose behalf
    // we should block the browser; we need this because these dialogs
    // are shown in a different AppContext
    private static LinkedList/*<Plugin2Manager>*/ managersShowingSystemDialogs = new LinkedList();

    public static void pushManagerShowingSystemDialog() {
        Plugin2Manager manager = Plugin2Manager.getCurrentManager();
        synchronized(managersShowingSystemDialogs) {
            managersShowingSystemDialogs.addFirst(manager);
        }
    }

    public static Plugin2Manager getManagerShowingSystemDialog() {
        synchronized(managersShowingSystemDialogs) {
            if (!managersShowingSystemDialogs.isEmpty()) {
                return (Plugin2Manager) managersShowingSystemDialogs.getFirst();
            }
        }
        return null;
    }

    public static void popManagerShowingSystemDialog() {
        Plugin2Manager manager = Plugin2Manager.getCurrentManager();
        synchronized(managersShowingSystemDialogs) {
            for (Iterator iter = managersShowingSystemDialogs.iterator(); iter.hasNext(); ) {
                if (iter.next() == manager) {
                    iter.remove();
                    break;
                }
            }
        }
    }

    //----------------------------------------------------------------------
    // Modal dialog re-activation support
    //

    private static Reactivator reactivator;

    /** "Re-activates" the given modal dialog. */
    public static void reactivateDialog(Dialog dialog) {
        getReactivator().reactivate(dialog);
    }

    private static Reactivator getReactivator() {
        if (reactivator == null) {
            if (SystemUtil.getOSType() == SystemUtil.WINDOWS) {
                reactivator = new WindowsReactivator();
            } else {
                // FIXME: would want to implement this in a platform-specific manner for other OSs
                reactivator = new NoopReactivator();
            }
        }
        return reactivator;
    }

    interface Reactivator {
        public void reactivate(Dialog dialog);
    }

    static class NoopReactivator implements Reactivator {
        public void reactivate(Dialog dialog) {}
    }

    static class WindowsReactivator implements Reactivator {
        private Method getHWndMethod;

        public void reactivate(Dialog dialog) {
            if (getHWndMethod == null) {
                try {
                    AccessController.doPrivileged(new PrivilegedAction() {
                            public Object run() {
                                try {
                                    Class c = Class.forName("sun.awt.windows.WComponentPeer");
                                    Method m = c.getDeclaredMethod("getHWnd", null);
                                    m.setAccessible(true);
                                    getHWndMethod = m;
                                } catch (Exception e) {
                                }
                                return null;
                            }
                        });
                } catch (Exception e) {
                }
            }

            long hWnd = 0;
            try {
                hWnd = ((Long) getHWndMethod.invoke(dialog.getPeer(), null)).longValue();

                Windows.MessageBeep(Windows.MB_OK);
                // some heuristics: 3 times x 64 milliseconds
                FLASHWINFO info = FLASHWINFO.create();
                info.cbSize(FLASHWINFO.size());
                info.hwnd(hWnd);
                info.dwFlags(Windows.FLASHW_CAPTION);
                info.uCount(3);
                info.dwTimeout(64);
                Windows.FlashWindowEx(info);
            } catch (Exception e) {
            }
        }
    }

    static class PluginModalityListener implements ModalityListener {
        private ModalityInterface modality;

        PluginModalityListener(ModalityInterface modality) {
            this.modality = modality;
        }

        public void modalityPushed(ModalityEvent ev) {
            modality.modalityPushed((Dialog) ev.getSource());
        }

        public void modalityPopped(ModalityEvent ev) {
            modality.modalityPopped((Dialog) ev.getSource());
        }
    }
}
