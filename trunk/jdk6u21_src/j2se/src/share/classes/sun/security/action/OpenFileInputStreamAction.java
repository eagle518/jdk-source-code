/*
 * @(#)OpenFileInputStreamAction.java	1.6, 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.action;

import java.io.*;

import java.security.PrivilegedExceptionAction;

/**
 * A convenience class for opening a FileInputStream as a privileged action.
 *
 * @version 1.6, 03/23/10
 * @author Andreas Sterbenz
 */
public class OpenFileInputStreamAction
	implements PrivilegedExceptionAction<FileInputStream> {

    private final File file;

    public OpenFileInputStreamAction(File file) {
        this.file = file;
    }

    public OpenFileInputStreamAction(String filename) {
        this.file = new File(filename);
    }

    public FileInputStream run() throws Exception {
        return new FileInputStream(file);
    }
}
