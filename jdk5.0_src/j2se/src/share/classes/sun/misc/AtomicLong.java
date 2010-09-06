/*
 * @(#)AtomicLong.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/**
 * Atomic access to a long value which may be read and updated by multiple
 * threads without explicit sychronization.
 *
 * Depending on the capability of the hardware we are running on, this class
 * may be implemented using Java-level locks, an 8-byte compare-and-swap or a
 * load-reserved/store-conditional pair.
 *
 * written by: Doug Lea  (dl@cs.oswego.edu)
 */

abstract public class AtomicLong { 

    private static native boolean VMSupportsCS8();

    /** 
     * Factory method to create instance of underlying implementation.
     *
     */
    public static AtomicLong newAtomicLong(long initialValue) {
        if (VMSupportsCS8())
            return new AtomicLongCSImpl(initialValue);
        else
            return new AtomicLongLockImpl(initialValue);
    }

    /** 
     * Get the current value, with VOLATILE-ACQUIRE memory semantics.
     */
    public abstract long get();

    /** 
     * Attempt to set the value to newValue only if it is currently oldValue.
     * This will always fail if another thread changes the value
     * to a different value between commencement and completion
     * of the attempted update, and MAY succeed otherwise. 
     * If committed, the operation has VOLATILE-RELEASE semantics.
     *
     * Note:
     * This can be implemented using CAS and with locks in the
     * obvious way. It can be implemented using LL/SC via
     *    if (LoadLLinked(&value) == oldValue)
     *       StoreConditional(&value, newValue)
     */
    public abstract boolean attemptUpdate(long oldValue, long newValue);

    /** 
     * Behaviorally equivalent to:
     * <pre>
     * attemptUpdate(get(), newValue);
     * </pre>
     */
    public abstract boolean attemptSet(long newValue);

    /** 
     * Behaviorally equivalent to:
     * <pre>
     * long v = get();
     * attemptUpdate(v, v+1);
     * </pre>
     *
     * Rationale: Automates and speeds up a very common usage.
     **/
    public abstract boolean attemptIncrememt();

    /** 
     * Behaviorally equivalent to:
     * <pre>
     * long v = get();
     * attemptUpdate(v, v+k);
     * </pre>
     *
     **/
    public abstract boolean attemptAdd(long k);

}
