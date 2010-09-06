/*
 * @(#)WinNativeLibrary.java	1.10 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws;

import java.io.File;
import com.sun.deploy.config.Config;

/**
 * Loads javawspl.dll
 */
public class WinNativeLibrary extends NativeLibrary
{
    private static boolean isLoaded = false;

    public synchronized void load() {
	if (!isLoaded) {
	    String path = Config.getJavaHome() + File.separator + "bin" +
		File.separator + "deploy.dll";
	    System.load(path);
	    isLoaded = true;
	}
    }
}
