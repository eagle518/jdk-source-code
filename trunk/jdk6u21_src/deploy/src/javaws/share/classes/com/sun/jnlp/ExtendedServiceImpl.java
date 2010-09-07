/*
 * @(#)ExtendedServiceImpl.java	1.17 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.io.*;
import java.security.PrivilegedAction;
import java.security.AccessController;
import javax.jnlp.*;
import javax.swing.*;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.javaws.JnlpxArgs;

public final class ExtendedServiceImpl implements ExtendedService {
    static private ExtendedServiceImpl _sharedInstance = null;

   
    private static int DEFAULT_FILESIZE = Integer.MAX_VALUE;

    private ExtendedServiceImpl() {
    }
    
    public static synchronized ExtendedServiceImpl getInstance() {
        if (_sharedInstance == null) {
	    _sharedInstance = new ExtendedServiceImpl();
        }
        return _sharedInstance;
    }
   
    public FileContents openFile(final java.io.File f) throws IOException {

	if (f == null) {
	    return null;
	}

	// we should create our own File object, instead of using the
	// one that got pass in from the caller, since they can
	// override File.getPath and cause problem
	final File file = new File(f.getPath());

	// just continue if user has permissions to access this file
	if (!CheckServicePermission.hasFileAccessPermissions(file.toString())) {
	    if (!JnlpxArgs.getFileReadWriteList().contains(file.toString())) {
	        // pop up dialog asking user for permission to open selected
	        // file
	        if (!askUser(file.getPath())) {
		    return null;
	        }
            }
	}

	// create FileContents object and return
	Object ret = AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {

		    try {
			return new FileContentsImpl(file, DEFAULT_FILESIZE);
		    } catch (IOException ioe) {
			return ioe;
		    }
		}
	    });

	if (ret instanceof IOException) {
	    throw (IOException)ret;
	} else {
	    return (FileContents)ret;
	}
    }

    synchronized boolean askUser(final String fileList) {

	if (CheckServicePermission.hasFileAccessPermissions()) {
	    return true;
	}

	ApiDialog apiDialog = new ApiDialog();
	String title = ResourceManager.getString("api.extended.open.title");
	String msg = ResourceManager.getString("api.extended.open.message");
	String lable = ResourceManager.getString("api.extended.open.label");

        return apiDialog.askUser(title, msg, null, lable, fileList, false);
    }
    

    public FileContents[] openFiles(final java.io.File[] f) throws IOException {
	if (f == null || f.length <= 0) { 
	    return null;
	}

	// we should create our own File object, instead of using the
	// one that got pass in from the caller, since they can
	// override File.getPath and cause problem
	final File[] files = new File[f.length];

	for (int i = 0; i < f.length; i++) {
	    files[i] = new File(f[i].getPath());
	}

	// see if user has permissions to all these files
	boolean allPermissions = true;
	for (int i=0; i<files.length; i++) {
	    if (!CheckServicePermission.hasFileAccessPermissions(
						files[i].toString())) {
		allPermissions = false;
		break;
	    }
	}

	// pop up dialog asking user for permission to open selected
	// file
	String fileList = "";
	for (int i = 0; i < files.length; i++) {
	    fileList = fileList + files[i].getPath() + "\n";
	}

	if (!allPermissions && !askUser(fileList)) {
	    return null;
	}

	Object[] fcs =  (Object[])AccessController.doPrivileged(new PrivilegedAction() {
		public Object run() {
		    Object[] fcontents = new FileContents[files.length];
		    try {
			for (int i = 0; i < files.length; i++) {
			    fcontents[i] = new FileContentsImpl(files[i], DEFAULT_FILESIZE);
			}
		    } catch (IOException ioe) {
			fcontents[0] = ioe;
		    }
		    return fcontents;
		}
	    });

	if (fcs[0] instanceof IOException) {
	    throw (IOException)fcs[0];
	} else {
	    return (FileContents[])fcs;
	}
    }

}
