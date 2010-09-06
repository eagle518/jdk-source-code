/*
 * @(#)NativeLibraryFactory.java	1.5 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package com.sun.javaws;

/**
 * Creates an instance of WinNativeLibrary
 */
public class NativeLibraryFactory 
{
    public static NativeLibrary newInstance() {
	return new WinNativeLibrary();
    }
}
