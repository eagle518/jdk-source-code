/*
 * @(#)FileSystem.java	1.2 04/03/10
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.io.File;
import java.io.IOException;

/*
 * Utility class to support file system operations
 * 
 * @since 1.5
 */
public abstract class FileSystem {

    private static final Object lock = new Object();
    private static FileSystem fs;

    protected FileSystem() { }

    /**
     * Opens the file system
     */
    public static FileSystem open() {
	synchronized (lock) {
	    if (fs == null) {
		fs = new FileSystemImpl();
	    }
            return fs;
	}
    }

    /**
     * Tells whether or not the specified file is located on a 
     * file system that supports file security or not.
     *
     * @throws	IOException	if an I/O error occurs.
     */
    public abstract boolean supportsFileSecurity(File f) throws IOException;

    /**
     * Tell whether or not the specified file is accessible
     * by anything other than the file owner.
     *
     * @throws  IOException     if an I/O error occurs.
     * 
     * @throws  UnsupportedOperationException	
     * 		If file is located on a file system that doesn't support
     *		file security.
     */
    public abstract boolean isAccessUserOnly(File f) throws IOException;
}
