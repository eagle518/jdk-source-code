/*
 * @(#)ExtendedServiceImpl.java	1.8 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jnlp;

import java.io.*;
import java.security.PrivilegedAction;
import java.security.AccessController;
import javax.jnlp.*;
import javax.swing.*;
import com.sun.deploy.resources.ResourceManager;

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
   
    public FileContents openFile(final java.io.File file) throws IOException {
	// pop up dialog asking user for permission to open selected
	// file
	if (!askUser(file.getPath())) return null;

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

    synchronized boolean askUser(String fileList) {
	SmartSecurityDialog securityDialog = new SmartSecurityDialog();

        // setup textarea to list the files
        JTextArea jta = new JTextArea(4,30);
        jta.setFont(ResourceManager.getUIFont());
        jta.setEditable(false);
        
        jta.append(fileList);
	
	JScrollPane jsp = new JScrollPane(jta);
	
	String message1 = ResourceManager.getString("APIImpl.extended.fileOpen.message1");
	
	String message2 = ResourceManager.getString("APIImpl.extended.fileOpen.message2");
	
        Object [] objs = { message1, jsp, message2};
	
	// Ask user
	return securityDialog.showDialog(objs);
    }
    

    public FileContents[] openFiles(final java.io.File[] files) throws IOException {
	if (files == null || files.length <= 0) return null;

	// pop up dialog asking user for permission to open selected
	// file
	String fileList = "";
	for (int i = 0; i < files.length; i++) {
	    fileList = fileList + files[i].getPath() + "\n";
	}

	if (!askUser(fileList)) return null;

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
