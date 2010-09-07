/*
 * @(#)EventMulticaster.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.plugin.util;

import java.lang.reflect.Array;
import java.util.EventListener;
import sun.net.ProgressEvent;
import sun.net.ProgressListener;


/**
 * A class is based on java.awt.AWTEventMulticaster which implements 
 * efficient and thread-safe multi-cast event dispatching for the Plugin
 * events defined. This class will manage an immutable structure 
 * consisting of a chain of event listeners and will dispatch events 
 * to those listeners.  Because the structure is immutable, it is safe 
 * to use this API to add/remove listeners during the process of an event 
 * dispatch operation.
 *
 * An example of how this class could be used to implement a new
 * component which fires "action" events:
 *
 * <pre><code>
 * public myComponent extends Component {
 *     ProgressListener progressListener = null;
 *
 *     public synchronized void addProgressListener(ProgressListener l) {
 *	   progressListener = EventMulticaster.add(progressListener, l);
 *     }
 *     public synchronized void removeProgressListener(ProgressListener l) {
 *  	   progressListener = EventMulticaster.remove(progressListener, l);
 *     }
 *     public void processEvent(PluginEvent e) {
 *         // when event occurs which causes "action" semantic
 *         if (progressListener != null) {
 *             progressListener.actionPerformed(new PluginEvent());
 *         }         
 * }
 * </code></pre>
 *
 * @author      Stanley Man-Kit Ho
 */

public class EventMulticaster implements ProgressListener
{

    protected final EventListener a, b;

    /**
     * Creates an event multicaster instance which chains listener-a
     * with listener-b. Input parameters <code>a</code> and <code>b</code> 
     * should not be <code>null</code>, though implementations may vary in 
     * choosing whether or not to throw <code>NullPointerException</code> 
     * in that case.
     * @param a listener-a
     * @param b listener-b
     */ 
    protected EventMulticaster(EventListener a, EventListener b) {
	this.a = a; this.b = b;
    }

    /**
     * Removes a listener from this multicaster and returns the
     * resulting multicast listener.
     * @param oldl the listener to be removed
     */
    protected EventListener remove(EventListener oldl) {
	if (oldl == a)  return b;
	if (oldl == b)  return a;
	EventListener a2 = removeInternal(a, oldl);
	EventListener b2 = removeInternal(b, oldl);
	if (a2 == a && b2 == b) {
	    return this;	// it's not here
	}
	return addInternal(a2, b2);
    }

    /**
     * Handles the progress start event by invoking the
     * progressStart methods on listener-a and listener-b.
     *
     * @param evt Progress event object.
     */
    public void progressStart(ProgressEvent evt)
    {
	((ProgressListener)a).progressStart(evt);
	((ProgressListener)b).progressStart(evt);    
    }
    
    /**
     * Handles the progress update event by invoking the
     * progressUpdate methods on listener-a and listener-b.
     *
     * @param evt Progress event object.
     */
    public void progressUpdate(ProgressEvent evt)
    {
	((ProgressListener)a).progressUpdate(evt);
	((ProgressListener)b).progressUpdate(evt);    
    }

    /**
     * Handles the progress finish event by invoking the
     * progressFinish methods on listener-a and listener-b.
     *
     * @param evt Progress event object.
     */
    public void progressFinish(ProgressEvent evt)
    {
	((ProgressListener)a).progressFinish(evt);
	((ProgressListener)b).progressFinish(evt);    
    }

    /**
     * Adds progress-listener-a with progress-listener-b and
     * returns the resulting multicast listener.
     * @param a progress-listener-a
     * @param b progress-listener-b
     */
    public static ProgressListener add(ProgressListener a, ProgressListener b) {
        return (ProgressListener) addInternal(a, b);
    }

    /**
     * Removes the old progress-listener from progress-listener-l and
     * returns the resulting multicast listener.
     * @param l progress-listener-l
     * @param oldl the progress-listener being removed
     */
    public static ProgressListener remove(ProgressListener l, ProgressListener oldl) {
	return (ProgressListener) removeInternal(l, oldl);
    }

    /** 
     * Returns the resulting multicast listener from adding listener-a
     * and listener-b together.  
     * If listener-a is null, it returns listener-b;  
     * If listener-b is null, it returns listener-a
     * If neither are null, then it creates and returns
     * a new EventMulticaster instance which chains a with b.
     * @param a event listener-a
     * @param b event listener-b
     */
    protected static EventListener addInternal(EventListener a, EventListener b) {
	if (a == null)  return b;
	if (b == null)  return a;
	return new EventMulticaster(a, b);
    }

    /** 
     * Returns the resulting multicast listener after removing the
     * old listener from listener-l.
     * If listener-l equals the old listener OR listener-l is null, 
     * returns null.
     * Else if listener-l is an instance of EventMulticaster, 
     * then it removes the old listener from it.
     * Else, returns listener l.
     * @param l the listener being removed from
     * @param oldl the listener being removed
     */
    protected static EventListener removeInternal(EventListener l, EventListener oldl) {
	if (l == oldl || l == null) {
	    return null;
	} else if (l instanceof EventMulticaster) {
	    return ((EventMulticaster)l).remove(oldl);
	} else {
	    return l;		// it's not here
	}
    }
          
    private static int getListenerCount(EventListener l) { 
        if (l instanceof EventMulticaster) { 
            EventMulticaster mc = (EventMulticaster)l; 
            return getListenerCount(mc.a) + getListenerCount(mc.b); 
        }
        // Delete nulls. 
        else { 
            return (l == null) ? 0 : 1; 
        } 
    }
    
    private static int populateListenerArray(EventListener[] a, EventListener l, int index) { 
        if (l instanceof EventMulticaster) { 
            EventMulticaster mc = (EventMulticaster)l; 
            int lhs = populateListenerArray(a, mc.a, index); 
            return populateListenerArray(a, mc.b, lhs); 
        }
        else if (l != null) { 
            a[index] = l; 
            return index + 1; 
        } 
        // Delete nulls. 
        else { 
            return index; 
        }
    }
    
    /**
     * Returns an array of all the objects chained as
     * <code><em>Foo</em>Listener</code>s by the specified
     * <code>java.util.EventListener</code>.
     * <code><em>Foo</em>Listener</code>s are chained by the
     * <code>EventMulticaster</code> using the
     * <code>add<em>Foo</em>Listener</code> method. 
     * If a <code>null</code> listener is specified, this method returns an
     * empty array. If the specified listener is not an instance of
     * <code>EventMulticaster</code>, this method returns an array which
     * contains only the specified listener. If no such listeners are chanined,
     * this method returns an empty array.
     *
     * @param l the specified <code>java.util.EventListener</code>
     * @param listenerType the type of listeners requested; this parameter
     *          should specify an interface that descends from
     *          <code>java.util.EventListener</code>
     * @return an array of all objects chained as
     *          <code><em>Foo</em>Listener</code>s by the specified multicast
     *          listener, or an empty array if no such listeners have been
     *          chained by the specified multicast listener
     * @exception ClassCastException if <code>listenerType</code>
     *          doesn't specify a class or interface that implements
     *          <code>java.util.EventListener</code>
     *
     * @since 1.4
     */
    public static EventListener[] getListeners(EventListener l, 
                                               Class listenerType) {  
        int n = getListenerCount(l); 
        EventListener[] result = (EventListener[])Array.newInstance(listenerType, n); 
        populateListenerArray(result, l, 0); 
        return result; 
    }
}
