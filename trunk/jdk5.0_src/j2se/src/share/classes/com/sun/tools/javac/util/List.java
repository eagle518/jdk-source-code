/**
 * @(#)List.java	1.26 04/06/06
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;

import java.util.Collection;
import java.util.Iterator;

/** A class for generic linked lists. Links are supposed to be
 *  immutable, the only exception being the incremental construction of
 *  lists via ListBuffers.  List is the main container class in
 *  GJC. Most data structures and algorthms in GJC use lists rather
 *  than arrays.
 *
 *  <p>Lists are always trailed by a sentinel element, whose head and tail
 *  are both null.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public final class List<A> implements Collection<A> {

    /** The first element of the list, supposed to be immutable.
     */
    public A head;

    /** The remainder of the list except for its first element, supposed
     *  to be immutable.
     */
    public List<A> tail;

    /** Construct a list given its head and tail.
     */
    public List(A head, List<A> tail) {
	this.tail = tail;
	this.head = head;
    }

    /** Construct an empty list.
     */
    public List() {
	this(null, null);
    }

    /** Construct an empty list.
     */
    public static <A> List<A> make() {
	return new List<A>();
    }

    /** Construct a list consisting of given element.
     */
    public static <A> List<A> make(A x1) {
	return new List<A>(x1, new List<A>());
    }

    /** Construct a list consisting of given elements.
     */
    public static <A> List<A> make(A x1, A x2) {
	return new List<A>(x1, new List<A>(x2, new List<A>()));
    }

    /** Construct a list consisting of given elements.
     */
    public static <A> List<A> make(A x1, A x2, A x3) {
	return new List<A>(x1, new List<A>(x2, new List<A>(x3, new List<A>())));
    }

    /** Construct a list consisting all elements of given array.
     */
    public static <A> List<A> make(A[] vec) {
	List<A> xs = new List<A>();
	for (int i = vec.length - 1; i >= 0; i--)
	    xs = new List<A>(vec[i], xs);
	return xs;
    }

    /** Construct a list consisting of a given number of identical elements.
     *  @param len    The number of elements in the list.
     *  @param init   The value of each element.
     */
    public static <A> List<A> make(int len, A init) {
	List<A> l = new List<A>();
	for (int i = 0; i < len; i++) l = new List<A>(init, l);
	return l;
    }

    /** Does list have no elements?
     */
    public boolean isEmpty() {
	return tail == null;
    }

    /** Does list have elements?
     */
    public boolean nonEmpty() {
	return tail != null;
    }

    /** Return the number of elements in this list.
     */
    public int length() {
	List<A> l = this;
	int len = 0;
	while (l.tail != null) {
	    l = l.tail;
	    len++;
	}
	return len;
    }
    public int size() {
        return length();
    }

    /** Prepend given element to front of list, forming and returning
     *  a new list.
     */
    public List<A> prepend(A x) {
	return new List<A>(x, this);
    }

    /** Prepend given list of elements to front of list, forming and returning
     *  a new list.
     */
    public List<A> prependList(List<A> xs) {
	if (this.isEmpty()) return xs;
	if (xs.isEmpty()) return this;
	// return this.prependList(xs.tail).prepend(xs.head);
	List<A> result = this;
	xs = xs.reverse();
	while (xs.nonEmpty()) {
	    List<A> h = xs;
	    xs = xs.tail;
	    h.tail = result;
	    result = h;
	}
	return result;
    }

    /** Reverse list, forming and returning a new list.
     */
    public List<A> reverse() {
	List<A> rev = new List<A>();
	for (List<A> l = this; l.nonEmpty(); l = l.tail)
	    rev = new List<A>(l.head, rev);
	return rev;
    }

    /** Append given element at length, forming and returning
     *  a new list.
     */
    public List<A> append(A x) {
	return make(x).prependList(this);
    }

    /** Append given list at length, forming and returning
     *  a new list.
     */
    public List<A> appendList(List<A> x) {
	return x.prependList(this);
    }

    /** Copy successive elements of this list into given vector until
     *  list is exhausted or end of vector is reached.
     */
    public <T> T[] toArray(T[] vec) {
	int i = 0;
	List<A> l = this;
	Object[] dest = vec;
	while (l.nonEmpty() && i < vec.length) {
	    dest[i] = l.head;
	    l = l.tail;
	    i++;
	}
	return vec;
    }
    public Object[] toArray() {
        return toArray(new Object[size()]);
    }

    /** Form a string listing all elements with given separator character.
     */
    public String toString(String sep) {
        if (isEmpty()) {
            return "";
	} else {
	    StringBuffer buf = new StringBuffer();
	    buf.append(head);
	    for (List<A> l = tail; l.nonEmpty(); l = l.tail) {
		buf.append(sep);
		buf.append(l.head);
	    }
	    return buf.toString();
	}
    }
	
    /** Form a string listing all elements with comma as the separator character.
     */
    public String toString() {
	return toString(",");
    }

    /** Compute a hash code, overrides Object
     */
    public int hashCode() {
	List<A> l = this;
	int h = 0;
	while (l.tail != null) {
	    h = h * 41 + (head != null ? head.hashCode() : 0);
	    l = l.tail;
	}
	return h;
    }

    /** Is this list the same as other list?
     */
    public boolean equals(Object other) {
	return other instanceof List && equals(this, (List)other);
    }

    /** Are the two lists the same?
     */
    public static boolean equals(List xs, List ys) {
	while (xs.tail != null && ys.tail != null) {
	    if (xs.head == null) {
		if (ys.head != null) return false;
	    } else {
		if (!xs.head.equals(ys.head)) return false;
	    }
	    xs = xs.tail;
	    ys = ys.tail;
	}
	return xs.tail == null && ys.tail == null;
    }

    /** Does the list contain the specified element?
     */
    public boolean contains(Object x) {
	List<A> l = this;
	while (l.tail != null) {
	    if (x == null) {
		if (l.head == null) return true;
	    } else {
		if (l.head.equals(x)) return true;
	    }
	    l = l.tail;
	}
	return false;
    }

    /** The last element in the list, if any, or null.
     */
    public A last() {
	A last = null;
	List<A> t = this;
	while (t.tail != null) {
	    last = t.head;
	    t = t.tail;
	}
	return last;
    }

    private static Iterator EMPTYITERATOR = new Iterator() {
            public boolean hasNext() {
                return false;
            }
            public Object next() {
                throw new java.util.NoSuchElementException();
            }
	    public void remove() {
		throw new UnsupportedOperationException();
	    }
        };

    public Iterator<A> iterator() {
        if (tail == null)
            return EMPTYITERATOR; // NOTE: unchecked conversion
	return new Iterator<A>() {
	    List<A> elems = List.this;
	    public boolean hasNext() {
		return elems.tail != null;
	    }
	    public A next() {
		A result = elems.head;
		elems = elems.tail;
		return result;
	    }
	    public void remove() {
		throw new UnsupportedOperationException();
	    }
	};
    }

    public boolean add(A a) {
        throw new UnsupportedOperationException();
    }
    public boolean remove(Object o) {
        throw new UnsupportedOperationException();
    }
    public boolean containsAll(Collection<?> c) {
        throw new UnsupportedOperationException();
    }
    public boolean addAll(Collection<? extends A> c) {
        throw new UnsupportedOperationException();
    }
    public boolean removeAll(Collection<?> c) {
        throw new UnsupportedOperationException();
    }
    public boolean retainAll(Collection<?> c) {
        throw new UnsupportedOperationException();
    }
    public void clear() {
        throw new UnsupportedOperationException();
    }
}
