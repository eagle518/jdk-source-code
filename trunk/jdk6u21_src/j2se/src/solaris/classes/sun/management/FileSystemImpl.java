/*
 * @(#)FileSystemImpl.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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

