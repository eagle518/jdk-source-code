/*
 * @(#)ModalitySupport.java	1.5 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.util.*;

/** Helps enforce modality of Java modal dialogs over the web browser. */

public class ModalitySupport {

    /** Initializes modality support for the given applet. It is
        expected that this will be called from the plugin's main
        thread. */
    public static synchronized void initialize(int appletID, Plugin plugin) {
        pluginInfoMap.put(new Integer(appletID), new PerPluginInfo(plugin));
    }

    /** Shuts down modality support for the given applet. */
    public static synchronized void shutdown(int appletID) {
        PerPluginInfo info = (PerPluginInfo) pluginInfoMap.remove(new Integer(appletID));
        // Wake up plugin just in case something strange goes wrong during shutdown
        info.getPlugin().notifyMainThread();
    }

    /** Notifies the modality support that a modal dialog was either
        raised (pushed) or lowered (popped) for the given applet. This
        method may be called from any thread. */
    public static void modalityChanged(final int appletID,
                                       boolean modalityPushed) {
        final PerPluginInfo info = (PerPluginInfo) pluginInfoMap.get(new Integer(appletID));
        if (info == null) {
            return;
        }

        final AppletID id = new AppletID(appletID);
        if (modalityPushed) {
            final int depth = info.modalityPushed();
            final Plugin plugin = info.getPlugin();
            plugin.invokeLater(new Runnable() {
                    public void run() {
                    
                        if(info.getPlugin().getActiveJSCounter() > 0){
                            info.modalityPopped();
                            return;
                        }
                    
                        while (!JVMManager.getManager().appletExited(id) &&
                               info.getModalityDepth() >= depth) {
                            plugin.waitForSignalWithModalBlocking();
                        }
                        
                        // We just consumed a notification of the event. Notify it again
                        // in case there are callers higher up on the stack waiting for it.
                        plugin.notifyMainThread();
                    }
                });
        } else {
            info.modalityPopped();
        }
    }

    /** Indicates whether the given applet ID should block the browser
        window, under the assumption that if two applets are running
        on the same browser / plugin main thread, and one has a modal
        dialog open, that if the other one starts a subordinate
        message pump (due to a JavaScript-to-Java call, for example),
        it should block the browser's main thread. It is expected that
        this method will be called from the plugin's main
        thread. FIXME: this entire notion may be too
        Windows-centric. */
    public static synchronized boolean appletShouldBlockBrowser(int appletID) {
        Thread currentThread = Thread.currentThread();
        // Quick check to see whether we're blocking the browser
        PerPluginInfo info = (PerPluginInfo) pluginInfoMap.get(new Integer(appletID));
        if (info != null && info.getModalityDepth() > 0) {
            return true;
        }
        for (Iterator iter = pluginInfoMap.values().iterator(); iter.hasNext(); ) {
            info = (PerPluginInfo) iter.next();
            if (info.getPluginMainThread() == currentThread &&
                info.getModalityDepth() > 0) {
                return true;
            }
        }
        return false;
    }

    /** Helper method for appletShouldBlockBrowser above. If the
        passed appletID is null, returns false immediately. */
    public static boolean appletShouldBlockBrowser(AppletID appletID) {
        if (appletID == null) {
            return false;
        }
        return appletShouldBlockBrowser(appletID.getID());
    }

    /** Fetches an (arbitrary) applet ID associated with a
        currently-active modal dialog blocking this thread in the
        browser. This applet's EmbeddedFrame will be
        reactivated. Returns null if no applet is currently blocking
        the current thread in the browser. */
    public static synchronized Integer getAppletBlockingBrowser() {
        Thread currentThread = Thread.currentThread();
        for (Iterator iter = pluginInfoMap.keySet().iterator(); iter.hasNext(); ) {
            Integer appletID = (Integer) iter.next();
            PerPluginInfo info = (PerPluginInfo) pluginInfoMap.get(appletID);
            if (info.getPluginMainThread() == currentThread &&
                info.getModalityDepth() > 0) {
                return appletID;
            }
        }
        return null;
    }

    //----------------------------------------------------------------------
    // Internals only below this point
    //

    private ModalitySupport() {}

    static class PerPluginInfo {
        private Plugin plugin;
        private Thread pluginMainThread;
        // 0 if no modal dialogs are up; greater than zero if one or more is
        private int modalityDepth;

        public PerPluginInfo(Plugin plugin) {
            this.plugin = plugin;
            pluginMainThread = Thread.currentThread();
        }

        public Plugin getPlugin() {
            return plugin;
        }

        public Thread getPluginMainThread() {
            return pluginMainThread;
        }

        public synchronized int modalityPushed() {
            return ++modalityDepth;
        }

        public synchronized void modalityPopped() {
            --modalityDepth;
            plugin.notifyMainThread();
        }

        public synchronized int getModalityDepth() {
            return modalityDepth;
        }
    }

    // Map from applet ID to per-plugin (and therefore per-applet) info
    private static Map/*<Integer, PerPluginInfo>*/ pluginInfoMap = new HashMap();
}
