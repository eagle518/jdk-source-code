/*
 * @(#)ConditionLock.java	1.22 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/**
 * ConditionLock is a Lock with a built in state variable.  This class
 * provides the ability to wait for the state variable to be set to a
 * desired value and then acquire the lock.<p>
 *
 * The lockWhen() and unlockWith() methods can be safely intermixed
 * with the lock() and unlock() methods. However if there is a thread
 * waiting for the state variable to become a particular value and you
 * simply call Unlock(), that thread will not be able to acquire the
 * lock until the state variable equals its desired value. <p>
 *
 * @version 	1.22, 12/19/03
 * @author 	Peter King
 */
public final
class ConditionLock extends Lock {
    private int state = 0;

    /**
     * Creates a ConditionLock.
     */
    public ConditionLock () {
    }

    /**
     * Creates a ConditionLock in an initialState.
     */
    public ConditionLock (int initialState) {
	state = initialState;
    }

    /**
     * Acquires the lock when the state variable equals the desired state.
     *
     * @param desiredState the desired state
     * @exception  java.lang.InterruptedException if another thread has
     *               interrupted this thread.
     */
    public synchronized void lockWhen(int desiredState) 
	throws InterruptedException
    {
	while (state != desiredState) {
	    wait();
	}
	lock();
    }

    /**
     * Releases the lock, and sets the state to a new value.
     * @param newState the new state
     */
    public synchronized void unlockWith(int newState) {
	state = newState;
	unlock();
    }
}
