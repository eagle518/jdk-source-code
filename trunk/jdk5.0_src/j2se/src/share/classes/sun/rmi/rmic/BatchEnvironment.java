/*
 * @(#)BatchEnvironment.java	1.27 04/06/21
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*****************************************************************************/
/*                    Copyright (c) IBM Corporation 1998                     */
/*                                                                           */
/* IBM Confidential                                           ClassType.java */
/*                                                                           */
/* OCO Source Materials                                                      */
/*                                                                           */
/* (C) Copyright IBM Corp. 1998                                              */
/*                                                                           */
/* The source code for this program is not published or otherwise            */
/* divested of its trade secrets, irrespective of what has been              */
/* deposited with the U.S. Copyright Office.                                 */
/*                                                                           */
/*****************************************************************************/

package sun.rmi.rmic;

import java.io.File;
import java.io.OutputStream;
import java.util.Enumeration;
import java.util.Vector;
import sun.tools.java.ClassPath;

/**
 * BatchEnvironment for rmic extends javac's version in four ways:
 * 1. It overrides errorString() to handle looking for rmic-specific
 * error messages in rmic's resource bundle
 * 2. It provides a mechanism for recording intermediate generated
 * files so that they can be deleted later.
 * 3. It holds a reference to the Main instance so that generators
 * can refer to it.
 * 4. It provides access to the ClassPath passed to the constructor.
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */

public class BatchEnvironment extends sun.tools.javac.BatchEnvironment {

    /** instance of Main which created this environment */
    private Main main;

    /**
     * Create a ClassPath object for rmic from a class path string.
     */
    public static ClassPath createClassPath(String classPathString) {
	ClassPath[] paths = classPaths(null, classPathString, null, null);
	return paths[1];
    }

    /**
     * Create a ClassPath object for rmic from the relevant command line
     * options for class path, boot class path, and extension directories.
     */
    public static ClassPath createClassPath(String classPathString,
					    String sysClassPathString,
					    String extDirsString)
    {
	ClassPath[] paths = classPaths(null,
				       classPathString,
				       sysClassPathString,
				       extDirsString);
	return paths[1];
    }

    /**
     * Create a BatchEnvironment for rmic with the given class path,
     * stream for messages and Main.
     */
    public BatchEnvironment(OutputStream out, ClassPath path, Main main) {
	super(out, new ClassPath(""), path);
				// use empty "sourcePath" (see 4666958)
	this.main = main;
    }

    /**
     * Get the instance of Main which created this environment.
     */
    public Main getMain() {
	return main;
    }

    /**
     * Get the ClassPath.
     */
    public ClassPath getClassPath() {
        return binaryPath;
    }

    /** list of generated source files created in this environment */
    private Vector generatedFiles = new Vector();

    /**
     * Remember a generated source file generated so that it
     * can be removed later, if appropriate.
     */
    public void addGeneratedFile(File file) {
	generatedFiles.addElement(file);
    }

    /**
     * Delete all the generated source files made during the execution
     * of this environment (those that have been registered with the
     * "addGeneratedFile" method).
     */
    public void deleteGeneratedFiles() {
	synchronized(generatedFiles) {
	    Enumeration enumeration = generatedFiles.elements();
	    while (enumeration.hasMoreElements()) {
		File file = (File) enumeration.nextElement();
		file.delete();
	    }
	    generatedFiles.removeAllElements();
	}
    }

    /**
     * Release resources, if any.
     */
    public void shutdown() {
        main = null;
        generatedFiles = null;
        super.shutdown();
    }

    /**
     * Return the formatted, localized string for a named error message
     * and supplied arguments.  For rmic error messages, with names that
     * being with "rmic.", look up the error message in rmic's resource
     * bundle; otherwise, defer to java's superclass method.
     */
    public String errorString(String err,
			      Object arg0, Object arg1, Object arg2)
    {
	if (err.startsWith("rmic.") || err.startsWith("warn.rmic.")) {
	    String result =  Main.getText(err,
					  (arg0 != null ? arg0.toString() : null),
					  (arg1 != null ? arg1.toString() : null),
					  (arg2 != null ? arg2.toString() : null));

	    if (err.startsWith("warn.")) {
		result = "warning: " + result;
	    }
	    return result;
	} else {
	    return super.errorString(err, arg0, arg1, arg2);
	}
    }
    public void reset() {
    }
}
