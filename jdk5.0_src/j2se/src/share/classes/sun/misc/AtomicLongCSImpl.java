/*
 * @(#)AtomicLongCSImpl.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/**
 * Compare-and-swap implementation of AtomicLong.
 */

class AtomicLongCSImpl extends AtomicLong { 

    private volatile long value;

    protected AtomicLongCSImpl(long initialValue) {
        value = initialValue;
    }

    public long get() { 
        return value; 
    }

    public native boolean attemptUpdate(long oldValue, long newValue);

    public boolean attemptSet(long newValue) { 
        return attemptUpdate(value, newValue);
    }

    public synchronized boolean attemptIncrememt() {
        return attemptUpdate(value, value+1);
    }

    public synchronized boolean attemptAdd(long k) {
        return attemptUpdate(value, value+k);
    }        
}
