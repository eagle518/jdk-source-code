/*
 * @(#)FileSaveServiceImpl.java	1.37 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
import javax.jnlp.FileSaveService;
import javax.jnlp.FileContents;
import com.sun.jnlp.JNLPFileFilter;
import javax.swing.filechooser.FileSystemView;

import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.ui.UIFactory;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.Trace;


public final class FileSaveServiceImpl implements FileSaveService {
    // Referenece to single instance of the service
    static FileSaveServiceImpl _sharedInstance = null;
    
    private ApiDialog _apiDialog;
    private String _lastPath;
    
    /** Private constructor, use getInstance method to get the single instance */
    private FileSaveServiceImpl() {
	_apiDialog = new ApiDialog();
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
    public FileContents saveFileDialog(final String pathHint, 
		final String[] extensions, final InputStream stream, 
		final String name) throws IOException {

        // Ask user permission first
	if (!askUser()) return null;
    	
	Object res = DeploySysRun.executePrivileged(new DeploySysAction() {
	    public Object execute() {
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
                if (extensions != null) {
                    jfc.addChoosableFileFilter(new JNLPFileFilter(extensions));
                }
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
			BufferedOutputStream bos = 
			    new BufferedOutputStream(new FileOutputStream(f));
			BufferedInputStream bis = 
			    new BufferedInputStream(stream);
			int n = bis.read(buffer);
			while(n != -1) {
			    bos.write(buffer, 0, n);
			    n = bis.read(buffer);
			}
			bos.close();
			setLastPath(f.getPath());
			return new FileContentsImpl(f, 
			    computeMaxLength(f.length()));
		    } catch(IOException ioe) {
			// Return exception
Trace.ignored(ioe);
			return ioe;
		    }
		}
		return null;
	    }
	}, null);
	if (res instanceof IOException) {
	    throw (IOException)res;
	} else {
	    return (FileContents)res;
	}
    }
    
    synchronized boolean askUser() {
	if (CheckServicePermission.hasFileAccessPermissions()) {
	    return true;
	}
	// Ask user
	return _apiDialog.askUser(
	    ResourceManager.getString("api.file.save.title"),
	    ResourceManager.getString("api.file.save.message"),
	    ResourceManager.getString("api.file.save.always"));
    }
    
    
    /** Lets say times 3 just to have a reasonable big value */
    static long computeMaxLength(long value) {
	return value * 3;
    }

    /* Check if filename already exists and ask to overwrite */
    static boolean fileChk(File filetotest) {
	
	if ( filetotest.exists() ) {
	    String message = ResourceManager.getString(
		"api.file.save.fileExist", filetotest.getPath());

	    String title = ResourceManager.getMessage(
		"api.file.save.fileExistTitle");

	    int ret = UIFactory.showConfirmDialog(null, null, message, title);
	    return (ret == UIFactory.OK);
	} else {
	    // File doesn't exist	    
	    return true;
	}
    }

}


