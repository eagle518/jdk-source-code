/*
 * @(#)FileSystemImpl.java	1.1 04/03/08
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.io.File;
import java.io.IOException;

/*
 * Solaris/Linux implementation of sun.management.FileSystem
 */
public class FileSystemImpl extends FileSystem {

    public boolean supportsFileSecurity(File f) throws IOException {
	return true;
    }

    public boolean isAccessUserOnly(File f) throws IOException {
	return isAccessUserOnly0(f.getPath());
    }

    // Native methods

    static native boolean isAccessUserOnly0(String path) throws IOException;

    // Initialization

    static {
	java.security.AccessController
            .doPrivileged(new sun.security.action.LoadLibraryAction("management"));
    }
}

