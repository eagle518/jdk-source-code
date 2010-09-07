/*
 * @(#)XDragAndDropProtocols.java	1.3 04/01/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
}
