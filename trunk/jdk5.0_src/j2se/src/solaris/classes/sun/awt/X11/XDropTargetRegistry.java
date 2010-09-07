/*
 * @(#)XDropTargetRegistry.java	1.7 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.util.ArrayList;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Iterator;
import java.util.List;

import sun.awt.dnd.SunDropTargetContextPeer;
import sun.awt.dnd.SunDropTargetEvent;

/**
 * The class responsible for registration/deregistration of drop sites.
 *
 * @since 1.5
 */
final class XDropTargetRegistry {

    private static final long DELAYED_REGISTRATION_PERIOD = 200;

    private static final XDropTargetRegistry theInstance = 
        new XDropTargetRegistry(); 

    private final HashMap<Long, Runnable> delayedRegistrationMap = 
        new HashMap<Long, Runnable>();

    private XDropTargetRegistry() {}

    static XDropTargetRegistry getRegistry() {
        return theInstance;
    }

    /**
     * Returns the XID of the topmost window with WM_STATE set in the ancestor
     * heirarchy of the specified window or 0 if none found.
     */
    private long getToplevelWindow(long window) {
        long ret = 0;
        long root = 0;
        long parent = 0;

        /* Traverse the ancestor tree from window up to the root and find 
           the top-level client window nearest to the root. */
        do {
            WindowPropertyGetter wpg =
                new WindowPropertyGetter(window, XWM.XA_WM_STATE, 0, 0, false,
                                         XlibWrapper.AnyPropertyType);
            try {
                wpg.execute(XToolkit.IgnoreBadWindowHandler);

                if (wpg.getActualType() == XWM.XA_WM_STATE.getAtom()) {
                    ret = window;
                }
            } finally {
                wpg.dispose();
            }

            XQueryTree qt = new XQueryTree(window);
            try {
                if (qt.execute() == 0) {
                    return 0;
                }
                root = qt.get_root();
                parent = qt.get_parent();
            } finally {
                qt.dispose();
            }

            window = parent;
        } while (window != root);

        return ret;
    }

    static final long getDnDProxyWindow() {
        return XWindow.getXAWTRootWindow().getWindow();
    }

    private static final class EmbeddedDropSiteEntry {
        private final long root;
        private final long event_mask;
        private final List sites = new ArrayList();  

        public EmbeddedDropSiteEntry(long root, long event_mask) {
            this.root = root;
            this.event_mask = event_mask;
        }

        public long getRoot() {
            return root;
        }
        public long getEventMask() {
            return event_mask;
        }
        public void addSite(long window) {
            Long lWindow = new Long(window);
            if (!sites.contains(lWindow)) {
                sites.add(lWindow);
            }
        }
        public void removeSite(long window) {
            Long lWindow = new Long(window);
            sites.remove(lWindow);
        }
        public boolean hasSites() {
            return sites.isEmpty();
        }
        public long[] getSites() {
            long[] ret = new long[sites.size()];
            Iterator iter = sites.iterator();
            int index = 0;
            while (iter.hasNext()) {
                Long l = (Long)iter.next();
                ret[index++] = l.longValue();
            }
            return ret;
        }
        public long getSite(int x, int y) {
            assert Thread.holdsLock(XToolkit.getAWTLock());

            Iterator iter = sites.iterator();
            while (iter.hasNext()) {
                Long l = (Long)iter.next();
                long window = l.longValue();
                XTranslateCoordinates xtc = 
                    new XTranslateCoordinates(getRoot(), window, x, y);
                try {
                    int status = xtc.execute(XToolkit.IgnoreBadWindowHandler);
                    if (status == 0 || 
                        (XToolkit.saved_error != null && 
                         XToolkit.saved_error.get_error_code() != XlibWrapper.Success)) {
                        continue;
                    }

                    int dest_x = xtc.get_dest_x();
                    int dest_y = xtc.get_dest_y();
                    if (dest_x >= 0 && dest_y >= 0) {
                        XWindowAttributes wattr = new XWindowAttributes();
                        try {
                            XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
                            status = XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                                      window, wattr.pData);
                            XToolkit.RESTORE_XERROR_HANDLER();

                            if (status == 0 || 
                                (XToolkit.saved_error != null && 
                                 XToolkit.saved_error.get_error_code() != XlibWrapper.Success)) {
                                continue;
                            }

                            if (wattr.get_map_state() != XlibWrapper.IsUnmapped
                                && dest_x < wattr.get_width() 
                                && dest_y < wattr.get_height()) {
                                return window;
                            }
                        } finally {
                            wattr.dispose();
                        }
                    }
                } finally {
                    xtc.dispose();
                }
            }
            return 0;
        }
    }

    private final HashMap embeddedDropSiteRegistry = new HashMap();

    private EmbeddedDropSiteEntry registerEmbedderDropSite(long embedder) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        long proxyWindow = getDnDProxyWindow();

        Iterator dropTargetProtocols = 
            XDragAndDropProtocols.getDropTargetProtocols();
        // The list of protocols supported by the embedder.
        List embedderProtocols = new ArrayList();
        
        while (dropTargetProtocols.hasNext()) {
            XDropTargetProtocol dropTargetProtocol = 
                (XDropTargetProtocol)dropTargetProtocols.next();
            if (dropTargetProtocol.isProtocolSupported(embedder)) {
                embedderProtocols.add(dropTargetProtocol);
            }
        }

        /*
         * By default, we register a drop site that supports all dnd
         * protocols. This approach is not appropriate in plugin
         * scenario if the browser supports Motif DnD and doesn't support
         * XDnD. If we forcibly set XdndAware on the browser toplevel, any drag
         * source that supports both protocols and prefers XDnD will be unable
         * to drop anything on the browser. 
         * The solution for this problem is not to register XDnD drop site 
         * if the browser supports only Motif DnD.
         * In general, if the browser already supports some protocols, we
         * register the embedded drop site only for those protocols. Otherwise
         * we register the embedded drop site for all protocols.
         */
        if (!embedderProtocols.isEmpty()) {
            dropTargetProtocols = embedderProtocols.iterator();
        } else {
            dropTargetProtocols =
                XDragAndDropProtocols.getDropTargetProtocols();
        }

        /* Grab server, since we are working with the window that belongs to 
           another client. */
        XlibWrapper.XGrabServer(XToolkit.getDisplay());
        try {
            while (dropTargetProtocols.hasNext()) {
                XDropTargetProtocol dropTargetProtocol = 
                    (XDropTargetProtocol)dropTargetProtocols.next();
                dropTargetProtocol.registerEmbedderDropSite(embedder);
            }            

            long root = 0;
            long event_mask = 0;
            XWindowAttributes wattr = new XWindowAttributes();
            try {
                XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
                int status = XlibWrapper.XGetWindowAttributes(XToolkit.getDisplay(),
                                                              embedder, wattr.pData);
                XToolkit.RESTORE_XERROR_HANDLER();

                if (status == 0 || 
                    (XToolkit.saved_error != null && 
                     XToolkit.saved_error.get_error_code() != XlibWrapper.Success)) {
                    throw new XException("XGetWindowAttributes failed");
                }
             
                event_mask = wattr.get_your_event_mask();
                root = wattr.get_root();
            } finally {
                wattr.dispose();
            }

            if ((event_mask & XlibWrapper.PropertyChangeMask) == 0) {
                XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
                XlibWrapper.XSelectInput(XToolkit.getDisplay(), embedder,
                                         event_mask | XlibWrapper.PropertyChangeMask);
                XToolkit.RESTORE_XERROR_HANDLER();
                
                if (XToolkit.saved_error != null && 
                    XToolkit.saved_error.get_error_code() != XlibWrapper.Success) {
                    throw new XException("XSelectInput failed");
                }
            }

            return new EmbeddedDropSiteEntry(root, event_mask);
        } finally {
            XlibWrapper.XUngrabServer(XToolkit.getDisplay());
        }
    }

    public void updateEmbedderDropSite(long embedder) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        Long lToplevel = new Long(embedder);
        synchronized (this) {
            if (embeddedDropSiteRegistry.get(lToplevel) == null) {
                return;
            }
        }

        Iterator dropTargetProtocols = 
            XDragAndDropProtocols.getDropTargetProtocols();
        // The list of protocols supported by the embedder.
        List embedderProtocols = new ArrayList();
        
        while (dropTargetProtocols.hasNext()) {
            XDropTargetProtocol dropTargetProtocol = 
                (XDropTargetProtocol)dropTargetProtocols.next();
            if (dropTargetProtocol.isProtocolSupported(embedder)) {
                embedderProtocols.add(dropTargetProtocol);
            }
        }

        /*
         * By default, we register a drop site that supports all dnd
         * protocols. This approach is not appropriate in plugin
         * scenario if the browser supports Motif DnD and doesn't support
         * XDnD. If we forcibly set XdndAware on the browser toplevel, any drag
         * source that supports both protocols and prefers XDnD will be unable
         * to drop anything on the browser. 
         * The solution for this problem is not to register XDnD drop site 
         * if the browser supports only Motif DnD.
         * In general, if the browser already supports some protocols, we
         * register the embedded drop site only for those protocols. Otherwise
         * we register the embedded drop site for all protocols.
         */
        if (!embedderProtocols.isEmpty()) {
            dropTargetProtocols = embedderProtocols.iterator();
        } else {
            dropTargetProtocols =
                XDragAndDropProtocols.getDropTargetProtocols();
        }

        /* Grab server, since we are working with the window that belongs to 
           another client. */
        XlibWrapper.XGrabServer(XToolkit.getDisplay());
        try {
            while (dropTargetProtocols.hasNext()) {
                XDropTargetProtocol dropTargetProtocol = 
                    (XDropTargetProtocol)dropTargetProtocols.next();
                dropTargetProtocol.registerEmbedderDropSite(embedder);
            }            
        } finally {
            XlibWrapper.XUngrabServer(XToolkit.getDisplay());
        }
    }

    private void unregisterEmbedderDropSite(long embedder, 
                                            EmbeddedDropSiteEntry entry) {
        assert Thread.holdsLock(XToolkit.getAWTLock());

        Iterator dropTargetProtocols = 
            XDragAndDropProtocols.getDropTargetProtocols();

        /* Grab server, since we are working with the window that belongs to 
           another client. */
        XlibWrapper.XGrabServer(XToolkit.getDisplay());
        try {
            while (dropTargetProtocols.hasNext()) {
                XDropTargetProtocol dropTargetProtocol = 
                    (XDropTargetProtocol)dropTargetProtocols.next();
                dropTargetProtocol.unregisterEmbedderDropSite(embedder);
            }            

            long event_mask = entry.getEventMask();

            /* Restore the original event mask for the embedder. */
            if ((event_mask & XlibWrapper.PropertyChangeMask) == 0) {
                XToolkit.WITH_XERROR_HANDLER(XToolkit.IgnoreBadWindowHandler);
                XlibWrapper.XSelectInput(XToolkit.getDisplay(), embedder,
                                         event_mask);
                XToolkit.RESTORE_XERROR_HANDLER();
                
                if (XToolkit.saved_error != null && 
                    XToolkit.saved_error.get_error_code() != XlibWrapper.Success) {
                    throw new XException("XSelectInput failed");
                }
            }
        } finally {
            XlibWrapper.XUngrabServer(XToolkit.getDisplay());
        }
    }

    private void registerEmbeddedDropSite(long toplevel, long window) {
        Long lToplevel = new Long(toplevel);
        EmbeddedDropSiteEntry entry = null;
        synchronized (this) {
            entry =
                (EmbeddedDropSiteEntry)embeddedDropSiteRegistry.get(lToplevel);
            if (entry == null) {
                entry = registerEmbedderDropSite(toplevel);
                embeddedDropSiteRegistry.put(lToplevel, entry);
            }
        }
        
        assert entry != null;

        entry.addSite(window);        
    }

    private void unregisterEmbeddedDropSite(long toplevel, long window) {
        Long lToplevel = new Long(toplevel);
        EmbeddedDropSiteEntry entry = null;
        synchronized (this) {
            entry =
                (EmbeddedDropSiteEntry)embeddedDropSiteRegistry.get(lToplevel);
            assert entry != null;
            entry.removeSite(window);
            if (!entry.hasSites()) {
                embeddedDropSiteRegistry.remove(lToplevel);
                unregisterEmbedderDropSite(toplevel, entry);
            }
        }
    }

    /*
     * Returns a drop site that is embedded in the specified embedder window and
     * contains the point with the specified root coordinates.
     */
    public long getEmbeddedDropSite(long embedder, int x, int y) {
        Long lToplevel = new Long(embedder);
        EmbeddedDropSiteEntry entry =
            (EmbeddedDropSiteEntry)embeddedDropSiteRegistry.get(lToplevel); 
        if (entry == null) {
            return 0;
        }
        return entry.getSite(x, y);
    }

    public void registerDropSite(long window) {
        if (window == 0) {
            throw new IllegalArgumentException();
        }        

        XDropTargetEventProcessor.activate();

        synchronized (XToolkit.getAWTLock()) {
            long toplevel = getToplevelWindow(window);

            /*
             * No window with WM_STATE property is found.
             * Since the window can be a plugin window reparented to the browser
             * toplevel, we cannot determine which window will eventually have
             * WM_STATE property set. So we schedule a timer callback that will
             * periodically attempt to find an ancestor with WM_STATE and
             * register the drop site appropriately. 
             */
            if (toplevel == 0) {
                addDelayedRegistrationEntry(window);
                return;
            }

            if (toplevel == window) {
                Iterator dropTargetProtocols = 
                    XDragAndDropProtocols.getDropTargetProtocols();
        
                while (dropTargetProtocols.hasNext()) {
                    XDropTargetProtocol dropTargetProtocol = 
                        (XDropTargetProtocol)dropTargetProtocols.next();
                    dropTargetProtocol.registerDropTarget(toplevel);
                }
            } else {
                registerEmbeddedDropSite(toplevel, window);
            }
        }
    }

    public void unregisterDropSite(long window) {
        if (window == 0) {
            throw new IllegalArgumentException();
        }

        synchronized (XToolkit.getAWTLock()) {
            Iterator dropProtocols = XDragAndDropProtocols.getDropTargetProtocols();

            removeDelayedRegistrationEntry(window);
        
            while (dropProtocols.hasNext()) {
                XDropTargetProtocol dropProtocol = (XDropTargetProtocol)dropProtocols.next();
                dropProtocol.unregisterDropTarget(window);
            }
        }
    }

    /**************** Delayed drop site registration *******************************/

    private void addDelayedRegistrationEntry(final long window) {
        Long lWindow = new Long(window);
        Runnable runnable = new Runnable() {
                public void run() {
                    removeDelayedRegistrationEntry(window);
                    registerDropSite(window);
                }
            };

        synchronized (XToolkit.getAWTLock()) {
            removeDelayedRegistrationEntry(window);
            delayedRegistrationMap.put(lWindow, runnable);
            XToolkit.schedule(runnable, DELAYED_REGISTRATION_PERIOD);
        }
    }

    private void removeDelayedRegistrationEntry(long window) {
        Long lWindow = new Long(window);

        synchronized (XToolkit.getAWTLock()) {
            Runnable runnable = delayedRegistrationMap.remove(lWindow);
            if (runnable != null) {
                XToolkit.remove(runnable);
            }
        }
    }
    /*******************************************************************************/
}

