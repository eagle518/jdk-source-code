/*
 * @(#)NativeThreadSet.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;


// Special-purpose data structure for sets of native threads


class NativeThreadSet {

    private long[] elts;
    private int used = 0;

    NativeThreadSet(int n) {
	elts = new long[n];
    }

    // Adds the current native thread to this set, returning its index so that
    // it can efficiently be removed later.
    //
    int add() {
	long th = NativeThread.current();
	if (th == 0)
	    return -1;
	synchronized (this) {
	    int start = 0;
	    if (used > elts.length) {
		int on = elts.length;
		int nn = on * 2;
		long[] nelts = new long[nn];
		System.arraycopy(elts, 0, nelts, 0, on);
		start = on;
	    }
	    for (int i = start; i < elts.length; i++) {
		if (elts[i] == 0) {
		    elts[i] = th;
		    used++;
		    return i;
		}
	    }
	    assert false;
	    return -1;
	}
    }

    // Removes the thread at the given index.
    //
    void remove(int i) {
	if (i < 0)
	    return;
	synchronized (this) {
	    elts[i] = 0;
	    used--;
	}
    }

    // Signals all threads in this set.
    //
    void signal() {
	synchronized (this) {
	    int u = used;
	    int n = elts.length;
	    for (int i = 0; i < n; i++) {
		long th = elts[i];
		if (th == 0)
		    continue;
		NativeThread.signal(th);
		if (--u == 0)
		    break;
	    }
	}
    }

}
