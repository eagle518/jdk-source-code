/*
 * @(#)PlatformPCSC.java	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.smartcardio;

import java.security.AccessController;

import sun.security.action.LoadLibraryAction;

// Platform specific code and constants
class PlatformPCSC {
    
    static final Throwable initException;
    
    PlatformPCSC() {
	// empty
    }

    static {
	initException = loadLibrary();
    }
    
    private static Throwable loadLibrary() {
	try {
	    AccessController.doPrivileged(new LoadLibraryAction("j2pcsc"));
	    return null;
	} catch (Throwable e) {
	    return e;
	}
    }

    // PCSC constants defined differently under Windows and MUSCLE
    // Windows version
    final static int SCARD_PROTOCOL_T0     =  0x0001;
    final static int SCARD_PROTOCOL_T1     =  0x0002;
    final static int SCARD_PROTOCOL_RAW    =  0x10000;

    final static int SCARD_UNKNOWN         =  0x0000;
    final static int SCARD_ABSENT          =  0x0001;
    final static int SCARD_PRESENT         =  0x0002;
    final static int SCARD_SWALLOWED       =  0x0003;
    final static int SCARD_POWERED         =  0x0004;
    final static int SCARD_NEGOTIABLE      =  0x0005;
    final static int SCARD_SPECIFIC        =  0x0006;

}
