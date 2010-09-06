/*
 * @(#)SunDragSourceContextPeer.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.dnd;

import java.awt.AWTEvent;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.EventQueue;
import java.awt.Image;
import java.awt.Point;

import java.awt.datatransfer.DataFlavor;
import java.awt.datatransfer.Transferable;

import java.awt.dnd.DnDConstants;
import java.awt.dnd.DragSource;
import java.awt.dnd.DragSourceContext;
import java.awt.dnd.DragSourceEvent;
import java.awt.dnd.DragSourceDropEvent;
import java.awt.dnd.DragSourceDragEvent;
import java.awt.dnd.DragGestureEvent;
import java.awt.dnd.InvalidDnDOperationException;

import java.awt.dnd.peer.DragSourceContextPeer;

import java.awt.event.InputEvent;
import java.awt.event.MouseEvent;

import java.util.Map;
import java.util.SortedMap;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import sun.awt.SunToolkit;
import sun.awt.datatransfer.DataTransferer;

/**
 * <p>
 * TBC
 * </p>
 *
 * @version 1.16.12/19/03
 * @since JDK1.3.1
 *
 */

public abstract class SunDragSourceContextPeer implements DragSourceContextPeer {

    private DragGestureEvent  trigger;
    private Component         component;
    private Cursor            cursor;
    private long              nativeCtxt;
    private DragSourceContext dragSourceContext;
    private int               sourceActions;

    private static boolean    dragDropInProgress = false;
    private static boolean    discardingMouseEvents = false;

    /*
     * dispatch constants
     */

    protected final static int DISPATCH_ENTER   = 1;
    protected final static int DISPATCH_MOTION  = 2;
    protected final static int DISPATCH_CHANGED = 3;
    protected final static int DISPATCH_EXIT    = 4;
    protected final static int DISPATCH_FINISH  = 5;
    protected final static int DISPATCH_MOUSE_MOVED  = 6;

    /**
     * construct a new SunDragSourceContextPeer
     */

    public SunDragSourceContextPeer(DragGestureEvent dge) {
        trigger = dge;
        if (trigger != null) {
            component = trigger.getComponent();
        } else {
            component = null;
        }
    }

    /**
     * initiate a DnD operation ...
     */

    public void startDrag(DragSourceContext dsc, Cursor c, Image di, Point p) 
      throws InvalidDnDOperationException {

        /* Fix for 4354044: don't initiate a drag if event sequence provided by
         * DragGestureRecognizer is empty */
        if (getTrigger().getTriggerEvent() == null) {
            throw new InvalidDnDOperationException("DragGestureEvent has a null trigger");
        }

        dragSourceContext = dsc; 
        cursor            = c;
        sourceActions     = getDragSourceContext().getSourceActions();

        Transferable transferable  = getDragSourceContext().getTransferable();
        SortedMap formatMap = DataTransferer.getInstance().getFormatsForTransferable
	     (transferable, DataTransferer.adaptFlavorMap
	         (getTrigger().getDragSource().getFlavorMap()));
        long[] formats = DataTransferer.getInstance().
	    keysToLongArray(formatMap);        
        startDrag(transferable, formats, formatMap);

        /*
         * Fix for 4613903.
         * Filter out all mouse events that are currently on the event queue.
         */
        discardingMouseEvents = true;
        EventQueue.invokeLater(new Runnable() {
                public void run() {
                    discardingMouseEvents = false;
                }
            });
    }

    protected abstract void startDrag(Transferable trans, 
                                      long[] formats, Map formatMap);

    /**
     * set cursor
     */

    public void setCursor(Cursor c) throws InvalidDnDOperationException {
        synchronized (this) {
            if (cursor == null || !cursor.equals(c)) {
                cursor = c;
                // NOTE: native context can be null at this point.
                // setNativeCursor() should handle it properly.
                setNativeCursor(getNativeContext(), c, 
                                c != null ? c.getType() : 0);
            }
        }
    }

    /**
     * return cursor
     */

    public Cursor getCursor() { 
        return cursor; 
    }

    /**
     * downcall into native code
     */


    protected abstract void setNativeCursor(long nativeCtxt, Cursor c, 
                                            int cType);

    protected synchronized void setTrigger(DragGestureEvent dge) { 
        trigger = dge; 
        if (trigger != null) {
            component = trigger.getComponent();
        } else {
            component = null;
        }
    }

    protected DragGestureEvent getTrigger() { 
        return trigger; 
    }

    protected Component getComponent() { 
        return component; 
    }

    protected synchronized void setNativeContext(long ctxt) { 
        nativeCtxt = ctxt; 
    }

    protected synchronized long getNativeContext() { 
        return nativeCtxt; 
    }

    protected DragSourceContext getDragSourceContext() { 
        return dragSourceContext; 
    }

    /**
     * Notify the peer that the transferables' DataFlavors have changed.
     *
     * No longer useful as the transferables are determined at the time
     * of the drag.
     */

    public void transferablesFlavorsChanged() {
    }





    protected final void postDragSourceDragEvent(final int targetAction, 
                                                 final int modifiers,
                                                 final int x, final int y,
                                                 final int dispatchType) {

        final int dropAction =
            SunDragSourceContextPeer.convertModifiersToDropAction(modifiers,
                                                                  sourceActions); 

        DragSourceDragEvent event = 
            new DragSourceDragEvent(getDragSourceContext(), 
                                    dropAction, 
                                    targetAction & sourceActions, 
                                    modifiers, x, y);
        EventDispatcher dispatcher = new EventDispatcher(dispatchType, event);

        SunToolkit.executeOnEventHandlerThread(getComponent(), dispatcher);
    }

    /**
     * upcall from native code
     */

    private void dragEnter(final int targetActions, 
                           final int modifiers,
                           final int x, final int y) {
        postDragSourceDragEvent(targetActions, modifiers, x, y, DISPATCH_ENTER);
    }

    /**
     * upcall from native code
     */
 
    private void dragMotion(final int targetActions, 
                            final int modifiers,
                            final int x, final int y) {
        postDragSourceDragEvent(targetActions, modifiers, x, y, DISPATCH_MOTION);
    }

    /**
     * upcall from native code
     */

    private void operationChanged(final int targetActions, 
                                  final int modifiers,
                                  final int x, final int y) {
        postDragSourceDragEvent(targetActions, modifiers, x, y, DISPATCH_CHANGED);
    }

    /**
     * upcall from native code
     */

    protected final void dragExit(final int x, final int y) {
        DragSourceEvent event = 
            new DragSourceEvent(getDragSourceContext(), x, y);
        EventDispatcher dispatcher = 
            new EventDispatcher(DISPATCH_EXIT, event);

        SunToolkit.executeOnEventHandlerThread(getComponent(), dispatcher);
    }

    /**
     * upcall from native code
     */

    private void dragMouseMoved(final int targetActions, 
                                final int modifiers,
                                final int x, final int y) {
        postDragSourceDragEvent(targetActions, modifiers, x, y,
                                DISPATCH_MOUSE_MOVED);
    }

    /**
     * upcall from native code via implemented class (do)
     */

    protected final void dragDropFinished(final boolean success, 
                                          final int operations,
                                          final int x, final int y) {
        DragSourceEvent event = 
            new DragSourceDropEvent(getDragSourceContext(),
                                    operations & sourceActions,
                                    success, x, y);
        EventDispatcher dispatcher = 
            new EventDispatcher(DISPATCH_FINISH, event);

        SunToolkit.executeOnEventHandlerThread(getComponent(), dispatcher);

        setNativeContext(0);
    }

    public static void setDragDropInProgress(boolean b) 
      throws InvalidDnDOperationException {
        if (dragDropInProgress == b) {
            throw new InvalidDnDOperationException(getExceptionMessage(b));
        }

        synchronized (SunDragSourceContextPeer.class) {
            if (dragDropInProgress == b) {
                throw new InvalidDnDOperationException(getExceptionMessage(b));
            }
            dragDropInProgress = b;
        }
    }

    /**
     * Filters out all mouse events that were on the java event queue when
     * startDrag was called.
     */
    public static boolean checkEvent(AWTEvent event) {
        if (discardingMouseEvents && event instanceof MouseEvent) {
            MouseEvent mouseEvent = (MouseEvent)event;
            if (!(mouseEvent instanceof SunDropTargetEvent)) {
                return false;
            }
        }
        return true;
    }

    public static void checkDragDropInProgress() 
      throws InvalidDnDOperationException {
        if (dragDropInProgress) {
            throw new InvalidDnDOperationException(getExceptionMessage(true));
        }
    }

    private static String getExceptionMessage(boolean b) {
        return b ? "Drag and drop in progress" : "No drag in progress";
    }

    public static int convertModifiersToDropAction(final int modifiers, 
                                                   final int supportedActions) {
        int dropAction = DnDConstants.ACTION_NONE;

        /*
         * Fix for 4285634.
         * Calculate the drop action to match Motif DnD behavior.
         * If the user selects an operation (by pressing a modifier key),
         * return the selected operation or ACTION_NONE if the selected
         * operation is not supported by the drag source. 
         * If the user doesn't select an operation search the set of operations
         * supported by the drag source for ACTION_MOVE, then for 
         * ACTION_COPY, then for ACTION_LINK and return the first operation
         * found.
         */
        switch (modifiers & (InputEvent.SHIFT_DOWN_MASK |
                             InputEvent.CTRL_DOWN_MASK)) { 
        case InputEvent.SHIFT_DOWN_MASK | InputEvent.CTRL_DOWN_MASK:
            dropAction = DnDConstants.ACTION_LINK; break;
        case InputEvent.CTRL_DOWN_MASK:
            dropAction = DnDConstants.ACTION_COPY; break;
        case InputEvent.SHIFT_DOWN_MASK:
            dropAction = DnDConstants.ACTION_MOVE; break;
        default:
            if ((supportedActions & DnDConstants.ACTION_MOVE) != 0) {
                dropAction = DnDConstants.ACTION_MOVE; 
            } else if ((supportedActions & DnDConstants.ACTION_COPY) != 0) {
                dropAction = DnDConstants.ACTION_COPY; 
            } else if ((supportedActions & DnDConstants.ACTION_LINK) != 0) {
                dropAction = DnDConstants.ACTION_LINK; 
            }
        }

        return dropAction & supportedActions;
    }

    private void cleanup() {
        trigger = null;
        component = null;
        cursor = null;
        dragSourceContext = null;
        SunDropTargetContextPeer.setCurrentJVMLocalSourceTransferable(null);
        SunDragSourceContextPeer.setDragDropInProgress(false);
    }

    private class EventDispatcher implements Runnable {

        private final int dispatchType;

        private final DragSourceEvent event;

        EventDispatcher(int dispatchType, DragSourceEvent event) {
            switch (dispatchType) {
            case DISPATCH_ENTER:
            case DISPATCH_MOTION:
            case DISPATCH_CHANGED:
            case DISPATCH_MOUSE_MOVED: 
                if (!(event instanceof DragSourceDragEvent)) {
                    throw new IllegalArgumentException("Event: " + event);
                }
                break;
            case DISPATCH_EXIT:
                break;
            case DISPATCH_FINISH:
                if (!(event instanceof DragSourceDropEvent)) {
                    throw new IllegalArgumentException("Event: " + event);
                }
                break;
            default:
                throw new IllegalArgumentException("Dispatch type: " +
                                                   dispatchType);
            }

            this.dispatchType  = dispatchType;
            this.event         = event;
        }

        public void run() {
            DragSourceContext dragSourceContext = 
                SunDragSourceContextPeer.this.getDragSourceContext();

            switch (dispatchType) {
            case DISPATCH_ENTER:  
                dragSourceContext.dragEnter((DragSourceDragEvent)event); 
                break;
            case DISPATCH_MOTION: 
                dragSourceContext.dragOver((DragSourceDragEvent)event); 
                break;
            case DISPATCH_CHANGED: 
                dragSourceContext.dropActionChanged((DragSourceDragEvent)event); 
                break;
            case DISPATCH_EXIT: 
                dragSourceContext.dragExit(event); break;
            case DISPATCH_MOUSE_MOVED: 
                dragSourceContext.dragMouseMoved((DragSourceDragEvent)event); 
                break;
            case DISPATCH_FINISH:
                try {
                    dragSourceContext.dragDropEnd((DragSourceDropEvent)event);
                } finally {
                    SunDragSourceContextPeer.this.cleanup();
                }
                break;
            default:
                throw new IllegalStateException("Dispatch type: " + 
                                                dispatchType);
            }
        }
    }
}
