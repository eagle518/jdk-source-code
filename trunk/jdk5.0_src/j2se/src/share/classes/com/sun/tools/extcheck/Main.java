/*
 * @(#)Main.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.extcheck;

import java.io.*;

/** 
 * Main program of extcheck
 */

public final class Main {
    

    /**
     * Terminates with one of the following codes
     *  1 A newer (or same version) jar file is already installed
     *  0 No newer jar file was found
     *  -1 An internal error occurred
     */
    public static void main(String args[]){
	
	if (args.length < 1){
	    System.err.println("Usage: extcheck [-verbose] <jar file>");
	    System.exit(-1);
	}
	int argIndex = 0;
	boolean verboseFlag = false;
	if (args[argIndex].equals("-verbose")){
	    verboseFlag = true;
	    argIndex++;
	}
	String jarName = args[argIndex];
	argIndex++;
	File jarFile = new File(jarName);
	if (!jarFile.exists()){
	    ExtCheck.error("Jarfile " + jarName + " does not exist");
	}
	if (argIndex < args.length) {
	    ExtCheck.error("Extra command line argument :"+args[argIndex]);
	}
	ExtCheck jt = ExtCheck.create(jarFile,verboseFlag);
	boolean result = jt.checkInstalledAgainstTarget();
	if (result) {
	    System.exit(0);
	} else { 
	    System.exit(1);
	}
	    
    }

}

