/*
 * @(#)IOStatus.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;


// Constants for reporting I/O status

final class IOStatus {

    private IOStatus() { }

    static final int EOF = -1;		    // End of file
    static final int UNAVAILABLE = -2;	    // Nothing available (non-blocking)
    static final int INTERRUPTED = -3;	    // System call interrupted
    static final int UNSUPPORTED = -4;	    // Operation not supported
    static final int THROWN = -5;	    // Exception thrown in JNI code
    static final int UNSUPPORTED_CASE = -6; // This case not supported

    // The following two methods are for use in try/finally blocks where a
    // status value needs to be normalized before being returned to the invoker
    // but also checked for illegal negative values before the return
    // completes, like so:
    //
    //     int n = 0;
    //     try {
    //         begin();
    //         n = op(fd, buf, ...);
    //         return IOStatus.normalize(n);    // Converts UNAVAILABLE to zero
    //     } finally {
    //         end(n > 0);
    //         assert IOStatus.check(n);	// Checks other negative values
    //     }
    //

    static int normalize(int n) {
	if (n == UNAVAILABLE)
	    return 0;
	return n;
    }

    static boolean check(int n) {
	return (n >= UNAVAILABLE);
    }

    static long normalize(long n) {
	if (n == UNAVAILABLE)
	    return 0;
	return n;
    }

    static boolean check(long n) {
	return (n >= UNAVAILABLE);
    }

    // Return true iff n is not one of the IOStatus values
    static boolean checkAll(long n) {
        return ((n > EOF) || (n < UNSUPPORTED_CASE));
    }

}
