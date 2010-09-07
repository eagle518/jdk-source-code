/*
 * @(#)NativeThread.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;


// Signalling operations on native threads
//
// On some operating systems (e.g., Linux), closing a channel while another
// thread is blocked in an I/O operation upon that channel does not cause that
// thread to be released.  This class provides access to the native threads
// upon which Java threads are built, and defines a simple signal mechanism
// that can be used to release a native thread from a blocking I/O operation.
// On systems that do not require this type of signalling, the current() method
// always returns zero and the signal(long) method has no effect.


class NativeThread {

    // Returns an opaque token representing the native thread underlying the
    // invoking Java thread.  On systems that do not require signalling, this
    // method always returns zero.
    //
    static native long current();

    // Signals the given native thread so as to release it from a blocking I/O
    // operation.  On systems that do not require signalling, this method has
    // no effect.
    //
    static native void signal(long nt);

    static native void init();

    static {
	Util.load();
	init();
    }

}
