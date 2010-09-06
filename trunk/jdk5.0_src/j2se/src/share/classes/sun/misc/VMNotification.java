/*
 * @(#)VMNotification.java	1.12 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

/** @deprecated */
@Deprecated
public interface VMNotification {

    // when the vm switches allocation states, we get notified
    // (possible semantics: if the state changes while in this
    // notification, don't recursively notify).
    // oldState and newState may be the same if we are just releasing
    // suspended threads.
    void newAllocState(int oldState, int newState,
		       boolean threadsSuspended);
}
