/*
 * @(#)NativePRNG.java	1.4 10/03/23 
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.provider;

/**
 * Native PRNG implementation for Windows. Currently a dummy, we do
 * not support a fully native PRNG on Windows.
 *
 * @since   1.5
 * @version 1.4, 03/23/10
 * @author  Andreas Sterbenz
 */
public final class NativePRNG {

    // return whether the NativePRNG is available
    static boolean isAvailable() {
	return false;
    }
    
}
