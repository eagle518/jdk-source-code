/**
 * @(#)Pair.java	1.15 04/04/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;

/** A generic class for pairs.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Pair<A, B> {

    public final A fst;
    public final B snd;

    public Pair(A fst, B snd) {
	this.fst = fst;
	this.snd = snd;
    }

    public String toString() {
        return "Pair[" + fst + "," + snd + "]";
    }

    private static boolean equals(Object x, Object y) {
	return (x == null && y == null) || (x != null && x.equals(y));
    }

    public boolean equals(Object other) {
   	return
	    other instanceof Pair &&
	    equals(fst, ((Pair)other).fst) &&
	    equals(snd, ((Pair)other).snd);
    }

    public int hashCode() {
	if (fst == null) return (snd == null) ? 0 : snd.hashCode() + 1;
	else if (snd == null) return fst.hashCode() + 2;
	else return fst.hashCode() * 17 + snd.hashCode();
    }
}





