/*
 * @(#)NativePRNG.java	1.2 03/12/19 
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider;

/**
 * Native PRNG implementation for Windows. Currently a dummy, we do
 * not support a fully native PRNG on Windows.
 *
 * @since   1.5
 * @version 1.2, 12/19/03
 * @author  Andreas Sterbenz
 */
public final class NativePRNG {

    // return whether the NativePRNG is available
    static boolean isAvailable() {
	return false;
    }
    
}
