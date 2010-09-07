/*
 * @(#)WDragSourceContextPeer.java	1.30 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.windows;

import java.awt.Component;
import java.awt.Cursor;

import java.awt.datatransfer.Transferable;

import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.InvalidDnDOperationException;

import java.awt.event.InputEvent;

import java.util.Map;

import sun.awt.Mutex;

import sun.awt.dnd.SunDragSourceContextPeer;

/**
 * <p>
 * TBC
 * </p>
 *
 * @version 1.30
 * @since JDK1.2
 *
 */

final class WDragSourceContextPeer extends SunDragSourceContextPeer {

    private static final WDragSourceContextPeer theInstance = 
        new WDragSourceContextPeer(null);

    /**
     * construct a new WDragSourceContextPeer. package private
     */

    private WDragSourceContextPeer(DragGestureEvent dge) {
    	super(dge);
    }

    static WDragSourceContextPeer createDragSourceContextPeer(DragGestureEvent dge) throws InvalidDnDOperationException {
	theInstance.setTrigger(dge);
        return theInstance;
    }

    protected void startDrag(Transferable trans, 
                             long[] formats, Map formatMap) {

        long nativeCtxtLocal = 0;

        nativeCtxtLocal = createDragSource(getTrigger().getComponent(),
                                           trans,
                                           getTrigger().getTriggerEvent(),
                                           getTrigger().getSourceAsDragGestureRecognizer().getSourceActions(),
                                           formats,
                                           formatMap);
            
        if (nativeCtxtLocal == 0) {
            throw new InvalidDnDOperationException("failed to create native peer");
        }

        setNativeContext(nativeCtxtLocal);

        WDropTargetContextPeer.setCurrentJVMLocalSourceTransferable(trans);
        
        doDragDrop(getNativeContext(), getCursor());
    }

    /**
     * downcall into native code
     */

    native long createDragSource(Component component,
                                 Transferable transferable,
                                 InputEvent nativeTrigger, 
                                 int actions,
                                 long[] formats,
                                 Map formatMap);

    /**
     * downcall into native code
     */

    native void doDragDrop(long nativeCtxt, Cursor cursor);

    protected native void setNativeCursor(long nativeCtxt, Cursor c, int cType);

}


