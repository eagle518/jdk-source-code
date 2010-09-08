/*
 * @(#)SequencedEvent.java	1.7 03/01/23
 *
 * Copyright 2003 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package java.awt;

import java.awt.AWTEvent;
import java.awt.ActiveEvent;
import java.util.LinkedList;
import sun.awt.AppContext;
import sun.awt.SunToolkit;

/**
 * A mechanism for ensuring that a series of AWTEvents are executed in a
 * precise order, even across multiple AppContexts. The nested events will be
 * dispatched in the order in which their wrapping SequencedEvents were
 * constructed. The only exception to this rule is if the peer of the target of
 * the nested event was destroyed (with a call to Component.removeNotify)
 * before the wrapping SequencedEvent was able to be dispatched. In this case,
 * the nested event is never dispatched.
 *
 * @version 1.7, 01/23/03
 * @author David Mendenhall
 */
class SequencedEvent extends AWTEvent implements ActiveEvent {
    private static final int ID =
	java.awt.event.FocusEvent.FOCUS_LAST + 1;
    private static final LinkedList list = new LinkedList();

    private final AWTEvent nested;
    private AppContext appContext;
    private boolean disposed;

    /**
     * Constructs a new SequencedEvent which will dispatch the specified
     * nested event.
     *
     * @param nested the AWTEvent which this SequencedEvent's dispatch()
     *        method will dispatch
     */
    SequencedEvent(AWTEvent nested) {
	super(nested.getSource(), ID);
	this.nested = nested;
	synchronized (SequencedEvent.class) {
	    list.add(this);
	}
    }

    /**
     * Dispatches the nested event after all previous nested events have been
     * dispatched or disposed. If this method is invoked before all previous nested events
     * have been dispatched, then this method blocks until such a point is
     * reached.
     * While waiting disposes nested events to disposed AppContext
     *
     * NOTE: Locking protocol.  Since dispose() can get EventQueue lock, 
     * dispatch() shall never call dispose() while holding the lock on the list, 
     * as EventQueue lock is held during dispatching.  The locks should be acquired 
     * in the same order.
     */
    public final void dispatch() {
        try {
            appContext = AppContext.getAppContext();

            if (getFirst() != this) {
                if (EventQueue.isDispatchThread()) {
                    EventDispatchThread edt = (EventDispatchThread)
                        Thread.currentThread();
                    edt.pumpEvents(SentEvent.ID, new Conditional() {
                        public boolean evaluate() {
                            return !SequencedEvent.this.isFirstOrDisposed();
                        }
                    });
                } else {
                    while(!isFirstOrDisposed()) {
                        synchronized (SequencedEvent.class) {
                            try {                             
                                SequencedEvent.class.wait(1000);
                            } catch (InterruptedException e) {
                                break;
                            }
                        }
                    }
                }
            }

            if (!disposed) {
                KeyboardFocusManager.getCurrentKeyboardFocusManager().
                    setCurrentSequencedEvent(this);
                Toolkit.getEventQueue().dispatchEvent(nested);
            }
        } finally {
            dispose();
        }
    }
   
    /**
     * true only if event exists and nested source appContext is disposed.
     */
    private final static boolean isOwnerAppContextDisposed(SequencedEvent se) {
        if (se != null) {
            Object target = se.nested.getSource();
            if (target instanceof Component) {
                return ((Component)target).appContext.isDisposed();
            }
        }
        return false;
    }

    /**
     * Sequenced events are dispatched in order, so we cannot dispatch
     * until we are the first sequenced event in the queue (i.e. it's our
     * turn).  But while we wait for our turn to dispatch, the event
     * could have been disposed for a number of reasons.
     */    
    public final boolean isFirstOrDisposed() {
        if (disposed) {
            return true;
        }
        // getFirstWithContext can dispose this
        return this == getFirstWithContext() || disposed;
    }

    private final synchronized static SequencedEvent getFirst() {
        return (SequencedEvent)list.getFirst();
    }

    /* Disposes all events from disposed AppContext 
     * return first valid event
     */
    private final static SequencedEvent getFirstWithContext() {
        SequencedEvent first = getFirst();
        while(isOwnerAppContextDisposed(first)) {
            first.dispose();
            first = getFirst();
        }        
        return first;
    }

    /**
     * Disposes of this instance. This method is invoked once the nested event
     * has been dispatched and handled, or when the peer of the target of the
     * nested event has been disposed with a call to Component.removeNotify.
     *
     * NOTE: Locking protocol.  Since SunToolkit.postEvent can get EventQueue lock, 
     * it shall never be called while holding the lock on the list, 
     * as EventQueue lock is held during dispatching and dispatch() will get 
     * lock on the list. The locks should be acquired in the same order.
     */
    final void dispose() {
      synchronized (SequencedEvent.class) {
            if (disposed) {
                return;
            }
            if (KeyboardFocusManager.getCurrentKeyboardFocusManager().
                    getCurrentSequencedEvent() == this) {
                KeyboardFocusManager.getCurrentKeyboardFocusManager().
                    setCurrentSequencedEvent(null);
            }
            disposed = true;
        }
        // Wake myself up
        if (appContext != null) {
            SunToolkit.postEvent(appContext, new SentEvent());
        }
        
        SequencedEvent next = null;
        
        synchronized (SequencedEvent.class) {
          SequencedEvent.class.notifyAll();

          if (list.getFirst() == this) {
              list.removeFirst();

              if (!list.isEmpty()) {
                    next = (SequencedEvent)list.getFirst();
              }
          } else {
              list.remove(this);
          }
      }
        // Wake up waiting threads
        if (next != null && next.appContext != null) {
            SunToolkit.postEvent(next.appContext, new SentEvent());
        }
    }
}