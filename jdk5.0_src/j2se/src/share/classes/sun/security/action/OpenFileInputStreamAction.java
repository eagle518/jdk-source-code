/*
 * @(#)OpenFileInputStreamAction.java	1.3, 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.action;

import java.io.*;

import java.security.PrivilegedExceptionAction;

/**
 * A convenience class for opening a FileInputStream as a privileged action.
 *
 * @version 1.3, 12/19/03
 * @author Andreas Sterbenz
 */
public class OpenFileInputStreamAction implements PrivilegedExceptionAction {

    private final File file;

    public OpenFileInputStreamAction(File file) {
        this.file = file;
    }

    public OpenFileInputStreamAction(String filename) {
        this.file = new File(filename);
    }

    public Object run() throws Exception {
        return new FileInputStream(file);
    }
}
