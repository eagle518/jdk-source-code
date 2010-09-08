/*
 * @(#)XDragAndDropProtocols.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

/**
 * This class is a registry for the supported drag and drop protocols.
 *
 * @since 1.5
 */
final class XDragAndDropProtocols {
    private final static List dragProtocols;
    private final static List dropProtocols;

    public static final String XDnD = "XDnD";
    public static final String MotifDnD = "MotifDnD";

    static {
        // Singleton listener for all drag source protocols.
        XDragSourceProtocolListener dragSourceProtocolListener = 
            XDragSourceContextPeer.getXDragSourceProtocolListener();
        // Singleton listener for all drop target protocols.
        XDropTargetProtocolListener dropTargetProtocolListener = 
            XDropTargetContextPeer.getXDropTargetProtocolListener();

        List tDragSourceProtocols = new ArrayList();
        XDragSourceProtocol xdndDragSourceProtocol = 
            XDnDDragSourceProtocol.createInstance(dragSourceProtocolListener);
        tDragSourceProtocols.add(xdndDragSourceProtocol);
        XDragSourceProtocol motifdndDragSourceProtocol = 
            MotifDnDDragSourceProtocol.createInstance(dragSourceProtocolListener);
        tDragSourceProtocols.add(motifdndDragSourceProtocol);

        List tDropTargetProtocols = new ArrayList();    
        XDropTargetProtocol xdndDropTargetProtocol = 
            XDnDDropTargetProtocol.createInstance(dropTargetProtocolListener);
        tDropTargetProtocols.add(xdndDropTargetProtocol);
        XDropTargetProtocol motifdndDropTargetProtocol = 
            MotifDnDDropTargetProtocol.createInstance(dropTargetProtocolListener);
        tDropTargetProtocols.add(motifdndDropTargetProtocol);

        dragProtocols = Collections.unmodifiableList(tDragSourceProtocols);
        dropProtocols = Collections.unmodifiableList(tDropTargetProtocols);
    }

    static Iterator getDragSourceProtocols() {
        return dragProtocols.iterator();
    }

    static Iterator getDropTargetProtocols() {
        return dropProtocols.iterator();
    }

    /*
     * Returns a XDragSourceProtocol whose name equals to the specified string
     * or null if no such protocol is registered.
     */
    public static XDragSourceProtocol getDragSourceProtocol(String name) {
        // Protocol name cannot be null.
        if (name == null) {
            return null;
        }

        Iterator dragProtocols = XDragAndDropProtocols.getDragSourceProtocols();
        while (dragProtocols.hasNext()) {
            XDragSourceProtocol dragProtocol =
                (XDragSourceProtocol)dragProtocols.next();
            if (dragProtocol.getProtocolName().equals(name)) {
                return dragProtocol;
            }
        }
        
        return null;
    }

    /*
     * Returns a XDropTargetProtocol which name equals to the specified string
     * or null if no such protocol is registered.
     */
    public static XDropTargetProtocol getDropTargetProtocol(String name) {
        // Protocol name cannot be null.
        if (name == null) {
            return null;
        }

        Iterator dropProtocols = XDragAndDropProtocols.getDropTargetProtocols();
        while (dropProtocols.hasNext()) {
            XDropTargetProtocol dropProtocol =
                (XDropTargetProtocol)dropProtocols.next();
            if (dropProtocol.getProtocolName().equals(name)) {
                return dropProtocol;
            }
        }
        
        return null;
    }
}
