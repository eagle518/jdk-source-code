/*
 * @(#)AtomicLongLockImpl.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/**
 * Java lock implementation of AtomicLong.
 *
 * written by: Doug Lea  (dl@cs.oswego.edu)
 */

class AtomicLongLockImpl extends AtomicLong { 

    private volatile long value;

    protected AtomicLongLockImpl(long initialValue) {
        value = initialValue;
    }

    public long get() { 
        return value; 
    }

    public synchronized boolean attemptSet(long newValue) { 
        value = newValue; 
        return true;
    }

    public synchronized boolean attemptUpdate(long oldValue, long newValue) {
        if (value == oldValue) {
            value = newValue;
            return true;
        }
        else
            return false;
    }

    public synchronized boolean attemptIncrememt() {
        ++value;
        return true;
    }

    public synchronized boolean attemptAdd(long k) {
        value += k;
        return true;
    }        
}
