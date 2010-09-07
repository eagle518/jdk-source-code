/*
 * @(#)WindowsHelper.java	1.7 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import sun.plugin2.ipc.*;
import sun.plugin2.ipc.windows.*;

/** This class contains Windows-specific helper routines which are
    intended to be browser-independent. */

public class WindowsHelper {
    private WindowsHelper() {}

    /** Waits the specified duration for the given (Windows) Event to
        be signalled, pumping all Windows messages in the interim. If
        the duration is 0, returns immediately. If it is less than
        zero, waits indefinitely. The isModalDialogActive flag
        indicates whether the client has raised a modal dialog, and
        changes the behavior of the underlying message pump. */
    public static void runMessagePump(Event event, long timeout, boolean isModalDialogActive) {
        long handle = ((WindowsEvent) event).getEventHandle();
        if (handle != 0) {
            runMessagePump0(handle, timeout, isModalDialogActive);
        }
    }

    private static native void runMessagePump0(long hEvent, long timeout, boolean isModalDialogActive);

    //----------------------------------------------------------------------
    // Blocking of browser window when a Java modal dialog is active
    //

    static class Hooks {
        long modalFilterHook;
        long mouseHook;

        Hooks(long modalFilterHook,
              long mouseHook) {
            this.modalFilterHook = modalFilterHook;
            this.mouseHook = mouseHook;
        }
    }
    private static final ThreadLocal/*<Hooks>*/ installedHooks = new ThreadLocal/*<Hooks>*/();

    /** Registers our hook procedures for modal dialog handling on the
        current thread; returns true if successful, false if already
        registered. */
    public static boolean registerModalDialogHooks(long hWndControlWindow, int appletID) {
        if (installedHooks.get() != null)
            return false;
        long modalFilterHook = installModalFilterHook(hWndControlWindow, appletID);
        long mouseHook = installMouseHook(hWndControlWindow, appletID);
        Hooks hooks = new Hooks(modalFilterHook, mouseHook);
        installedHooks.set(hooks);
        return true;
    }

    /** Unregisters our hook procedures for modal dialog handling from
        the current thread. */
    public static void unregisterModalDialogHooks(long hWndControlWindow) {
        Hooks hooks = (Hooks) installedHooks.get();
        if (hooks != null) {
            installedHooks.set(null);
            uninstallHook(hooks.modalFilterHook, hWndControlWindow);
            uninstallHook(hooks.mouseHook, hWndControlWindow);
        }
    }

    private static native long installModalFilterHook(long hWndControlWindow, int appletID);
    private static native long installMouseHook(long hWndControlWindow, int appletID);
    private static native long uninstallHook(long hook, long hWndControlWindow);

    // Called from native code
    public static void reactivateCurrentModalDialog() {
        Integer appletID = ModalitySupport.getAppletBlockingBrowser();
        if (appletID != null) {
            JVMManager.getManager().synthesizeWindowActivation(new AppletID(appletID.intValue()),
                                                               true);
        }
    }
}
