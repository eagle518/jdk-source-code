/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)File.java	1.13 03/12/19
 */


/*
 * I wanted to create a package (refer to the Java language
 * specification for more information on packages) for all of the classes
 * used in this example.  Using the package statment in all of the
 * related classes, we can force all of these classes to be associated
 * together.
 *
 */
package demo;


/**
 * The File superclass defines an interface for manipulating path
 * and file names.  
 *
 */
public
class File {

    /**
     * The file path.  We want to use an abstract path separator
     * in Java that is converted to the system dependent path
     * separator.
     *
     */
    protected String path;

    /**
     * The class File's notion of a path separator character.  This
     * will be the Java path separator.  Note that this will be
     * converted to the system dependent path separator at runtime
     * by code in the native library.
     *
     */
    public static final char separatorChar = ':';

    /**
     * The constructor, initializes the class with the given path.  Note
     * that we use the String class found in the Java core classes.
     *
     */
    public File(String path) {
	if (path == null) {
	    throw new NullPointerException();
	}
	this.path = path;
    }	

    /**
     * Get the name of the file, not including the directory path.
     *
     */
    public String getFileName() {
	int index = path.lastIndexOf(separatorChar);
	return (index < 0) ? path : path.substring(index + 1);
    }

    /**
     * Get the name of the file including the full directory path.
     *
     */
    public String getPath() {
	return path;
    }
}
