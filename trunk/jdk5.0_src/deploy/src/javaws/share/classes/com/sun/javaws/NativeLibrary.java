/*
 * @(#)NativeLibrary.java	1.7 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.javaws;


/**
 * To load the platform specific native library that Java Web Start
 * depends on:
 * <pre>
 * NativeLibrary.getInstance().load();
 * </pre>
 */
public abstract class NativeLibrary
{
    private static NativeLibrary nativeLibrary;

    /**
     * Return a shared platform specific instance of this class.
     */
    public synchronized static NativeLibrary getInstance() {
	if (nativeLibrary == null) {
	    nativeLibrary = NativeLibraryFactory.newInstance();
	}
	return nativeLibrary;
    }

    /**
     * Load the native library, if there is one.  It's safe
     * to call this method more than once.
     */
    public void load() {
        /* Implementation is in platform-dependent subclass */
    }
}
