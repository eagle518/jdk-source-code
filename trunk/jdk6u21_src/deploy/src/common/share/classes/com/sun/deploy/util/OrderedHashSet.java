/*
 * @(#)OrderedHashSet.java	1.5 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.util.*;

/** 
 * Ordered hash set implementation of the Collection interface, 
 * with predictable iteration order, where new elements are added to the end.
 *
 * This implementation is backwards compatible with Java 1.3.
 *
 * This implementation differs from LinkedHashSet<E> (Java 1.4):
 *  <UL>
 *  <LI> Java 1.3 compatible
 *  <LI> add(Object o) will move a pre-existing object to the end of the list
 *  <LI> does not implement Serializable yet.
 *  </UL>
 *
 * Performance notes
 *
 * o(1) is achieved for 
 * <UL>
 * <LI> adding new elements
 * <LI> trying to remove non existent elements
 * </UL>
 *
 * o(n) is paid for 
 * <UL>
 * <LI> reordering pre existent elements,
 *      i.e. elements existing, but not at the end of the list
 * <LI> removing existing elements
 * </UL>
*/

public class OrderedHashSet
    implements Collection, Cloneable
{
    HashSet    objectsHash = new HashSet();
    LinkedList objects     = new LinkedList();

    public OrderedHashSet() {
        clear();
    }

    // CLONEABLE

    public Object clone()
                    throws CloneNotSupportedException
    {
        LinkedList clonedList = (LinkedList)objects.clone();

        OrderedHashSet newObj = new OrderedHashSet();
        newObj.addAll(clonedList);

        return newObj;
    }

    // COLLECTION

    public void   clear() {
        objects.clear();
        objectsHash.clear();
    }

    /**
     * adds the object to the end of the list,
     * if it already exists and is at the wrong position, relocate it
     *
     * o(1) is achieved for new elements
     * o(n) is paid for reordering pre existent elements,
     * i.e. elements existing, but not at the end of the list.
     *
     * @return true if the object was not already in our list,
     *         false otherwise, even if it has been relocated to the end
     */
    public boolean add(Object o) {
        boolean newToList = objectsHash.add(o);
        if(!newToList) {
            // object is already in list
            // remove it first, before adding it at the end
            objects.remove(o);
        }
        // new add to the end
        objects.add(o);

        return newToList;
    }

    public boolean remove(Object o) {
        if ( objectsHash.remove(o) ) {
            objects.remove(o);
            return true;
        }
        return false;
    }

    public boolean addAll(Collection c) {
        boolean mod = false;
        for (Iterator iter = c.iterator(); iter.hasNext(); ) {
            mod = add(iter.next()) || mod;
        }
        return mod;
    }

    public boolean contains(Object o) {
        return objectsHash.contains(o);
    }

    public boolean containsAll(Collection c) {
        for (Iterator iter = c.iterator(); iter.hasNext(); ) {
            if (! this.contains(iter.next()) ) {
                return false;
            }
        }
        return true;
    }

    public boolean removeAll(Collection c) {
        boolean mod = false;
        for (Iterator iter = c.iterator(); iter.hasNext(); ) {
            mod = this.remove(iter.next()) || mod;
        }
        return mod;
    }

    public boolean retainAll(Collection c) {
        boolean mod = false;
        for (Iterator iter = this.iterator(); iter.hasNext(); ) {
            Object o = iter.next();
            if (! c.contains(o) ) {
                mod = this.remove(o) || mod;
            }
        }
        return mod;
    }

    /**
     * @return true if all entries must exist and in the same order
     * Performance: o(1)
     */
    public boolean equals(Object o) {
        if ( !(o instanceof OrderedHashSet) ) {
            return false;
        }
        return objects.equals(((OrderedHashSet)o).objects);
    }

    // Performance: o(1)
    public int hashCode() {
        return objectsHash.hashCode();
    }

    public boolean isEmpty() {
        return objects.isEmpty();
    }

    public Iterator iterator() {
        return objects.iterator();
    }

    public int size() {
        return objects.size();
    }

    public Object[] toArray() {
        return objects.toArray();
    }

    public Object[] toArray(Object[] a) {
        return objects.toArray(a);
    }


    // rudimentary List support for free,
    // as long we don't mess with the order

    public Object get(int index) {
        return objects.get(index);
    }

    public int indexOf(Object o) {
        return objects.indexOf(o);
    }

    public List toList() {
        return objects;
    }
}

