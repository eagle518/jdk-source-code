/*
 * @(#)AbstractEventSet.java	1.11 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

package com.sun.tools.example.debug.event;

import com.sun.jdi.*;
import com.sun.jdi.event.*;
import com.sun.jdi.request.*;

import java.util.*;

public abstract class AbstractEventSet extends EventObject implements EventSet {
    
    private final EventSet jdiEventSet;
    final Event oneEvent;

    /**
     */
    AbstractEventSet(EventSet jdiEventSet) {
        super(jdiEventSet.virtualMachine());
        this.jdiEventSet = jdiEventSet;
        this.oneEvent = eventIterator().nextEvent();
    }

    public static AbstractEventSet toSpecificEventSet(EventSet jdiEventSet) {
        Event evt = jdiEventSet.eventIterator().nextEvent();
        if (evt instanceof LocatableEvent) {
            if (evt instanceof ExceptionEvent) {
                return new ExceptionEventSet(jdiEventSet);
            } else if (evt instanceof WatchpointEvent) {
                if (evt instanceof AccessWatchpointEvent) {
                    return new AccessWatchpointEventSet(jdiEventSet);
                } else {
                    return new ModificationWatchpointEventSet(jdiEventSet);
                }
            } else {
                return new LocationTriggerEventSet(jdiEventSet);
            }
        } else if (evt instanceof ClassPrepareEvent) {
            return new ClassPrepareEventSet(jdiEventSet);
        } else if (evt instanceof ClassUnloadEvent) {
            return new ClassUnloadEventSet(jdiEventSet);
        } else if (evt instanceof ThreadDeathEvent) {
            return new ThreadDeathEventSet(jdiEventSet);
        } else if (evt instanceof ThreadStartEvent) {
            return new ThreadStartEventSet(jdiEventSet);
        } else if (evt instanceof VMDeathEvent) {
            return new VMDeathEventSet(jdiEventSet);
        } else if (evt instanceof VMDisconnectEvent) {
            return new VMDisconnectEventSet(jdiEventSet);
        } else if (evt instanceof VMStartEvent) {
            return new VMStartEventSet(jdiEventSet);
        } else {
            throw new IllegalArgumentException("Unknown event " + evt);
        }
    }

    public abstract void notify(JDIListener listener);

    // Implement Mirror

    public VirtualMachine virtualMachine() {
        return jdiEventSet.virtualMachine();
    }

    public VirtualMachine getVirtualMachine() {
        return jdiEventSet.virtualMachine();
    }

    // Implement EventSet

    /**
     * Returns the policy used to suspend threads in the target VM
     * for this event set. This policy is selected from the suspend
     * policies for each event's request. The one that suspends the 
     * most threads is chosen when the event occurs in the target VM
     * and that policy is returned here. See 
     * com.sun.jdi.request.EventRequest for the possible policy values.
     * 
     * @return the integer suspendPolicy
     */
    public int getSuspendPolicy() {
        return jdiEventSet.suspendPolicy();
    }

    public void resume() {
        jdiEventSet.resume();
    }

    public int suspendPolicy() {
        return jdiEventSet.suspendPolicy();
    }

    public boolean suspendedAll() {
        return jdiEventSet.suspendPolicy() == EventRequest.SUSPEND_ALL;
    }

    public boolean suspendedEventThread() {
        return jdiEventSet.suspendPolicy() == EventRequest.SUSPEND_EVENT_THREAD;
    }

    public boolean suspendedNone() {
        return jdiEventSet.suspendPolicy() == EventRequest.SUSPEND_NONE;
    }

    /**
     * Return an iterator specific to {@link Event} objects.
     */
    public EventIterator eventIterator() {
        return jdiEventSet.eventIterator();
    }


    // Implement java.util.Set (by pass through)

    /**
     * Returns the number of elements in this set (its cardinality).  If this
     * set contains more than <tt>Integer.MAX_VALUE</tt> elements, returns
     * <tt>Integer.MAX_VALUE</tt>.
     *
     * @return the number of elements in this set (its cardinality).
     */
    public int size() {
        return jdiEventSet.size();
    }

    /**
     * Returns <tt>true</tt> if this set contains no elements.
     *
     * @return <tt>true</tt> if this set contains no elements.
     */
    public boolean isEmpty() {
        return jdiEventSet.isEmpty();
    }

    /**
     * Returns <tt>true</tt> if this set contains the specified element.  More
     * formally, returns <tt>true</tt> if and only if this set contains an
     * element <code>e</code> such that <code>(o==null ? e==null :
     * o.equals(e))</code>.
     *
     * @return <tt>true</tt> if this set contains the specified element.
     */
    public boolean contains(Object o) {
        return jdiEventSet.contains(o);
    }

    /**
     * Returns an iterator over the elements in this set.  The elements are
     * returned in no particular order (unless this set is an instance of some
     * class that provides a guarantee).
     *
     * @return an iterator over the elements in this set.
     */
    public Iterator<Event> iterator() {
        return jdiEventSet.iterator();
    }

    /**
     * Returns an array containing all of the elements in this set.
     * Obeys the general contract of the <tt>Collection.toArray</tt> method.
     *
     * @return an array containing all of the elements in this set.
     */
    public Object[] toArray() {
        return jdiEventSet.toArray();
    }

    /**
     * Returns an array containing all of the elements in this set whose
     * runtime type is that of the specified array.  Obeys the general
     * contract of the <tt>Collection.toArray(Object[])</tt> method.
     *
     * @param a the array into which the elements of this set are to
     *		be stored, if it is big enough {
        return jdiEventSet.XXX();
    } otherwise, a new array of the
     * 		same runtime type is allocated for this purpose.
     * @return an array containing the elements of this set.
     * @throws    ArrayStoreException the runtime type of a is not a supertype
     * of the runtime type of every element in this set.
     */
    public <T> T[] toArray(T a[]) {
        return jdiEventSet.toArray(a);
    }

    // Bulk Operations

    /**
     * Returns <tt>true</tt> if this set contains all of the elements of the
     * specified collection.  If the specified collection is also a set, this
     * method returns <tt>true</tt> if it is a <i>subset</i> of this set.
     *
     * @param c collection to be checked for containment in this set.
     * @return <tt>true</tt> if this set contains all of the elements of the
     * 	       specified collection.
     */
    public boolean containsAll(Collection<?> c) {
        return jdiEventSet.containsAll(c);
    }


    // Make the rest of Set unmodifiable

    public boolean add(Event e){
        throw new UnsupportedOperationException();
    }
    public boolean remove(Object o) {
        throw new UnsupportedOperationException();
    }
    public boolean addAll(Collection<? extends Event> coll) {
        throw new UnsupportedOperationException();
    }
    public boolean removeAll(Collection<?> coll) {
        throw new UnsupportedOperationException();
    }
    public boolean retainAll(Collection<?> coll) {
        throw new UnsupportedOperationException();
    }
    public void clear() {
        throw new UnsupportedOperationException();
    }
}

