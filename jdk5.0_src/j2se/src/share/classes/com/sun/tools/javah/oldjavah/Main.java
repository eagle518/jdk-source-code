/*
 * @(#)Main.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javah.oldjavah;

import sun.tools.util.CommandLine;
import java.io.IOException;
import java.io.FileNotFoundException;

import java.io.File;

/**
 * Javah generates support files for native methods.
 *
 * @version 1.10, 12/03/01
 */
public class Main {

    /**
     * Generator used by javah.  Default is JNI.
     */
    Gen g = new JNI();

    /**
     * Entry point.
     */
    public static void main(String[] args) {
	/* Preprocess @file arguments */
	try {
	    args = CommandLine.parse(args);
	} catch (FileNotFoundException e) {
	    Util.error("at.args.cant.read", e.getMessage());
	} catch (IOException e) {
	    Util.error("at.args.io.exception", e.getMessage());
	}
	
	new Main(args).run();
	System.exit(0);
    }

    /**
     * Parse options.
     */
    public Main(String[] args) {
	if (args.length == 0) {
	    Util.usage(1);
	}
	
	/* Default values for options, overridden by user options. */
	String  bootcp      = System.getProperty("sun.boot.class.path");
	String  usercp      = System.getProperty("env.class.path");
	String  odir        = null;
	String  ofile       = null;
	boolean stubs       = false;
	boolean jni         = false;
	boolean old         = false;
	boolean llni        = false;
	boolean doubleAlign = false;
	boolean force       = false;
	String  genclass    = null;
	
	int i = 0;
	for (; i < args.length; i++) {
	    if (args[i].equals("-o")) {
		ofile = args[++i];
	    } else if (args[i].equals("-d")) {
		odir = args[++i];
	    } else if (args[i].equals("-td")) {
		/* Ignored.  Generate tmp files to memory. */
		i++;
		if (i == args.length)
		    Util.usage(1);
	    } else if (args[i].equals("-stubs")) {
		stubs = true;
	    } else if (args[i].equals("-v") || args[i].equals("-verbose")) {
		Util.verbose = true;
	    } else if (args[i].equals("-help") || args[i].equals("--help") ||
		       args[i].equals("-?") || args[i].equals("-h")) {
		Util.usage(0);
	    } else if (args[i].equals("-trace")) {
		System.err.println(Util.getText("tracing.not.supported"));
	    } else if (args[i].equals("-version")) {
		Util.version();
	    } else if (args[i].equals("-jni")) {
		jni = true;
	    } else if (args[i].equals("-force")) {
		force = true;
	    } else if (args[i].equals("-old")) {
		old = true;
	    } else if (args[i].equals("-Xllni")) {
		llni = true;
	    } else if (args[i].equals("-llni")) {
		llni = true;
	    } else if (args[i].equals("-llniDouble")) {
		llni = true; doubleAlign = true;
	    } else if (args[i].equals("-classpath")) {
		usercp = args[++i];
	    } else if (args[i].equals("-bootclasspath")) {
		bootcp = args[++i];
	    } else if (args[i].charAt(0) == '-') {
		Util.error("unknown.option", args[i], null, true);
	    } else {
		break; /* The rest must be classes. */
	    }
	}

	/*
	 * Select native interface.
	 */

	if (old && jni)   Util.error("old.jni.mixed");
	if (old && llni)  Util.error("old.llni.mixed");
	if (jni && llni)  Util.error("jni.llni.mixed");
	if (old && stubs)
	    g = new OldStubs();
	else if (old)
	    g = new OldHeaders();
	if (llni)
	    g = new LLNI(doubleAlign);
	if (g instanceof JNI && stubs)  Util.error("jni.no.stubs");

	/*
	 * Choose classpath.
	 */
	StringBuffer cp = new StringBuffer();
	if (bootcp == null)
	    cp.append(System.getProperty("java.class.path")); /* oldjava */
	else {
	    if (usercp == null)
		usercp = ".";
	    cp.append(bootcp);
	    cp.append(java.io.File.pathSeparator);
	    cp.append(usercp);
	}
	if (Util.verbose)
	    Util.log("[Search path = " + cp + "]");
	g.setClasspath(cp.toString());

	/*
	 * Arrange for output destination.
	 */
	if (odir != null && ofile != null)
	    Util.error("dir.file.mixed");
	if (odir != null)
	    g.setOutDir(odir);
	if (ofile != null)
	    g.setOutFile(ofile);

	/*
	 * Force set to false will turn off smarts about checking file
	 * content before writing.
	 */
	g.setForce(force);
	
	/* 
	 * Grab the rest of argv[] ... this must be the classes.
	 */
	String[] classes = new String[args.length - i];
	System.arraycopy(args, i, classes, 0, args.length - i);
	
	if (classes.length == 0) {
	    Util.error("no.classes.specified");
	}

	g.setClasses(classes);
    }
    
    public void run() {
	try {
	  g.run();
	} catch (ClassNotFoundException cnfe) {
	    Util.error("class.not.found", cnfe.getMessage());
	} catch (IOException ioe) {
	    Util.error("io.exception", ioe.getMessage());
	}
    }
}


