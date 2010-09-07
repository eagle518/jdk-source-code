/*
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * @(#)InputFile.java	1.15 03/12/19
 */



/*
 * Since we want this file to be part of the "demo" package we direct
 * the compiler to associate this class with the other "demo" package
 * classes.
 *
 */
package demo;


/**
 * Define class InputFile that presents a simple readonly input file 
 * abstraction.  Note that we use native or non-Java methods to
 * implement some of the methods.
 *
 */
public
class InputFile extends File {

    /**
     * Link in the native library that we depends on.  If we cannot
     * link this in, an exception is generated and the class loading
     * fails.  We have arbitrarily named the library "file" at the
     * Java level (or libfile.so at the solaris level).  Additionally,
     * the Linker call is part of the static initializer for the class.
     * Thus, the library is loaded as part of this class being loaded.
     *
     */
    static {
        System.loadLibrary("file");
    }

    /**
     * Holds the system dependent handle to the file resource.
     *
     */
    protected int fd;

    /**
     * Constructor for the input file object.  Initializes the
     * parent class with the path name.
     *
     */
    public InputFile(String path) {
	super(path);
    }

    /**
     * Attempts to open the file for reading.  Returns
     * TRUE on success and FALSE on failure.  Alternatively, we could
     * throw an exception and catch it.
     *
     */
    public native boolean open();

    /**
     * Attempts to close the previously opened file.  Has
     * no return value.
     */
    public native void close();

    /**
     * Reads some number of bytes from the opened file.  Returns
     * the number of bytes read or -1 when the file is empty.
     *
     */
    public native int read(byte b[], int len);
}
