/*
 * @(#)FileSystemImpl.java	1.3 04/03/10
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

import java.io.File;
import java.io.IOException;

/*
 * Windows implementation of sun.management.FileSystem
 */
public class FileSystemImpl extends FileSystem {

    public boolean supportsFileSecurity(File f) throws IOException {
	return isSecuritySupported0(f.getAbsolutePath());
    }

    public boolean isAccessUserOnly(File f) throws IOException {
	String path = f.getAbsolutePath();
	if (!isSecuritySupported0(path)) {
	    throw new UnsupportedOperationException("File system does not support file security");
	}
	return isAccessUserOnly0(path);
    }

    // Native methods

    static native void init0();

    static native boolean isSecuritySupported0(String path) throws IOException;

    static native boolean isAccessUserOnly0(String path) throws IOException;

    // Initialization

    static {
	java.security.AccessController
            .doPrivileged(new sun.security.action.LoadLibraryAction("management"));
	init0();
    }
}

