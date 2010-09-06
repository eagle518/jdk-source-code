/*
 * @(#)FileSaveServiceImpl.java	1.30 04/06/13
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;

import java.io.File;
import java.io.InputStream;
import java.io.FileOutputStream;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.security.AccessController;
import java.security.PrivilegedAction;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileFilter;
import javax.swing.LookAndFeel;
import javax.jnlp.FileSaveService;
import javax.jnlp.FileContents;
import javax.swing.filechooser.FileSystemView;

import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DialogFactory;


public final class FileSaveServiceImpl implements FileSaveService {
    // Referenece to single instance of the service
    static FileSaveServiceImpl _sharedInstance = null;
    
    private SmartSecurityDialog _securityDialog = null;
    private String _lastPath;
    
    /** Private constructor, use getInstance method to get the single instance */
    private FileSaveServiceImpl() {
	_securityDialog = new SmartSecurityDialog(ResourceManager.getString("APIImpl.file.save.message"));
    }
    
    /** Returns the single instance of this service */
    public static synchronized FileSaveService getInstance() {
	if (_sharedInstance == null) {
	    _sharedInstance = new FileSaveServiceImpl();
	}
	return _sharedInstance;
    }
    
    // Shared state with the FileOpenService
    String getLastPath() { return _lastPath; }
    void setLastPath(String filePath) { _lastPath = filePath; }
    
    public FileContents saveAsFileDialog(final String pathHint, final String[] extensions,
					 final FileContents fileContents) throws IOException {
	return saveFileDialog(pathHint, extensions, fileContents.getInputStream(), fileContents.getName());
    }
    
    /** Asks the users to save a file. See description in interface */
    public FileContents saveFileDialog(final String pathHint, final String[] extensions,
				       final InputStream stream, final String name) throws IOException {
        try {
            // Ask user permission first

	    if (!askUser()) return null;
    	
	    Object res = AccessController.doPrivileged(new PrivilegedAction() {
		    public Object run() {
			JFileChooser  jfc = null;
			FileSystemView fsv = FileOpenServiceImpl.getFileSystemView();
			// use pathHint if possible
			if (pathHint != null) {
			  
			    jfc = new JFileChooser(pathHint, fsv);
			
			}
			else {   // use lastSavePath
			   
			    jfc = new JFileChooser(getLastPath(), fsv);
			 
			}
			
			jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
			jfc.setDialogType(JFileChooser.SAVE_DIALOG);
			jfc.setMultiSelectionEnabled(false);
			int state = jfc.showSaveDialog(null);
			if (state == JFileChooser.CANCEL_OPTION) {
			    return null;
			}
			File f = jfc.getSelectedFile();
			if (f != null) {
			    /* Check if the file exists or not. */
			    if (!fileChk(f)) {
				return null;
			    }
			    try {
				byte[] buffer = new byte[8*1024];
				BufferedOutputStream bos = new BufferedOutputStream(new FileOutputStream(f));
				BufferedInputStream  bis = new BufferedInputStream(stream);
				int n = bis.read(buffer);
				while(n != -1) {
				    bos.write(buffer, 0, n);
				    n = bis.read(buffer);
				}
				bos.close();
				setLastPath(f.getPath());
				return new FileContentsImpl(f, computeMaxLength(f.length()));
			    } catch(IOException ioe) {
				// Return exception
				return ioe;
			    }
			}
			return null;
		    }
		});
	    if (res instanceof IOException) {
	        throw (IOException)res;
	    } else {
	        return (FileContents)res;
	    }
	} finally {
	}
    }
    
    synchronized boolean askUser() {
	if (CheckServicePermission.hasFileAccessPermissions()) return true;
	// Ask user
	return _securityDialog.showDialog();
    }
    
    
    /** Lets say times 3 just to have a reasonable big value */
    static long computeMaxLength(long value) {
	return value * 3;
    }

    /* Check if filename already exists and ask to overwrite */
    static boolean fileChk(File filetotest) {
	
	if ( filetotest.exists() ) {
	    String message = ResourceManager.getString(
		"APIImpl.file.save.fileExist", filetotest.getPath());

	    String title = ResourceManager.getMessage(
		"APIImpl.file.save.fileExistTitle");

	    int ret = DialogFactory.showConfirmDialog(message, title);
	    return (ret == DialogFactory.YES_OPTION);
	} else {
	    // File doesn't exist	    
	    return true;
	}
    }

}


