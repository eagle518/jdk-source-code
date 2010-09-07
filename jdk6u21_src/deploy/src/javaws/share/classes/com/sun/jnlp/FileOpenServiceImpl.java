/*
 * @(#)FileOpenServiceImpl.java	1.32 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.jnlp;
import java.io.IOException;
import java.io.File;
import java.io.InputStream;
import java.io.FileInputStream;
import java.security.AccessController;
import java.security.PrivilegedAction;
import javax.swing.JFileChooser;
import javax.swing.JOptionPane;
import javax.swing.filechooser.FileFilter;
import javax.jnlp.FileOpenService;
import javax.jnlp.FileContents;
import com.sun.jnlp.JNLPFileFilter;
import com.sun.jnlp.CheckServicePermission;
import com.sun.javaws.Globals;
import javax.swing.filechooser.*;
import java.lang.reflect.Method;
import java.util.Vector;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.Config;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;

public final class FileOpenServiceImpl implements FileOpenService {
    // Referenece to single instance of the service
    static FileOpenServiceImpl _sharedInstance = null;
    // This service shares some state with the FileSaveService
    // (All common code is located in FileSaveServiceImpl)
    static FileSaveServiceImpl _fileSaveServiceImpl;
    private ApiDialog _apiDialog;

    
    /** Private constructor, use getInstance method to get the single instance */
    private FileOpenServiceImpl(FileSaveServiceImpl fssi) {
	_fileSaveServiceImpl = fssi;
	_apiDialog = new ApiDialog();
    }
    
    
    /** Returns the single instance of this service. Needs to get a FileSave Service impl too. */
    public static synchronized FileOpenService getInstance() {
	if (_sharedInstance == null) {
	    _sharedInstance = new FileOpenServiceImpl((FileSaveServiceImpl)FileSaveServiceImpl.getInstance());
	}
	return _sharedInstance;
    }
    
    public static FileSystemView getFileSystemView() {
	
	FileSystemView fsv = FileSystemView.getFileSystemView();
	
	if (Config.getInstance().useAltFileSystemView()) {
	    
	    String java_ver = System.getProperty("java.version");       
	    // only use this work around if it is running 1.3.x or below
	    // only need to check java version that starts with 1.2 or 1.3
	    // since java web start only support Java 2 (1.2 or above)
	    if (java_ver.startsWith("1.2") || java_ver.startsWith("1.3")) {
		fsv = new WindowsAltFileSystemView();
	    } 
	}
	return fsv;
    }
    /** Asks the user to choose a file. See description in service interface */
    public FileContents openFileDialog(final String pathHint, 
			final String[] extentions) throws IOException {
        // Ask user permission first
        if (!askUser()) {
	    return null;
	}
        return (FileContents) DeploySysRun.executePrivileged(
	    new DeploySysAction() {
	        public Object execute() {
		    JFileChooser jfc = null;
		    FileSystemView fsv = getFileSystemView();
		    if (pathHint != null) { 	// use pathHint if possible
		        jfc = new JFileChooser(pathHint, fsv);
		    } else {  			// or use lastOpenPath
		        jfc = new JFileChooser(
			    _fileSaveServiceImpl.getLastPath(), fsv);
		    }
                    if (extentions != null) {
                        jfc.addChoosableFileFilter(new JNLPFileFilter(extentions));
                    }
		    jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
		    jfc.setDialogType(JFileChooser.OPEN_DIALOG);
		    jfc.setMultiSelectionEnabled(false);
		    int state = jfc.showOpenDialog(null);
		    if (state == JFileChooser.CANCEL_OPTION) {
		        // User selected to cancle operation
		        return null;
		    }
		    File f = jfc.getSelectedFile();
		    if (f != null) {
		        try {
			    _fileSaveServiceImpl.setLastPath(f.getPath());
			    return new FileContentsImpl(f, FileSaveServiceImpl.
				computeMaxLength(f.length()));
		        } catch (IOException ioe) {
			    // Fall through
		        }
		    }
		    return null;
	        }
	    }, null);
    }
    
    /** Asks the user to choose one or more files. 
     *  See description in service interface 
     */
    public FileContents[] openMultiFileDialog(final String pathHint, 
		final String[] extentions) throws IOException {

        // Check permission with user first
        if (!askUser()) {
            return null;
	}

        return (FileContents[]) DeploySysRun.executePrivileged(
	    new DeploySysAction() {
	        public Object execute() {
		    JFileChooser jfc = null;
		    FileSystemView fsv = getFileSystemView();
		    if (pathHint != null) { 	// use pathHint if possible
		        jfc = new JFileChooser(pathHint, fsv);
		    } else {  			// or use lastOpenPath
		        jfc = new JFileChooser(
				_fileSaveServiceImpl.getLastPath(), fsv);

			if (extentions != null) {
			    jfc.addChoosableFileFilter(new JNLPFileFilter(extentions));
			}

		    }
		    jfc.setFileSelectionMode(JFileChooser.FILES_ONLY);
		    jfc.setDialogType(JFileChooser.OPEN_DIALOG);
		    jfc.setMultiSelectionEnabled(true);
		    int state = jfc.showOpenDialog(null);
		    if (state == JFileChooser.CANCEL_OPTION) {
		        // User selected to cancle operation
		        return null;
		    }
		    File[] fs = jfc.getSelectedFiles();
		    if (fs != null && fs.length > 0) {
		        FileContents[] fcontents = new FileContents[fs.length];
		        for(int i = 0; i < fs.length; i++) {
			    try {
			        fcontents[i] = new FileContentsImpl(fs[i], 
				    FileSaveServiceImpl.computeMaxLength(
					fs[i].length()));
			        _fileSaveServiceImpl.setLastPath(
				    fs[i].getPath());
			    } catch(IOException ioe) {
			    }
		        }
		        return fcontents;
		    }
		    return null;
	        }
	    }, null);
    }
    
    // This class is necessary due to an annoying bug on Windows NT where
    // instantiating a JFileChooser with the default FileSystemView will
    // cause a "drive A: not ready" error every time.  I grabbed the
    // Windows FileSystemView impl from the 1.3 SDK and modified it so
    // as to not use java.io.File.listRoots() to get fileSystem roots.
    // java.io.File.listRoots() does a SecurityManager.checkRead() which
    // causes the OS to try to access drive A: even when there is no disk,
    // causing an annoying "abort, retry, ignore" popup message every time
    // we instantiate a JFileChooser!
    // x
    // Instead of calling listRoots() we use a straightforward alternate
    // method of getting file system roots.
    
    static class WindowsAltFileSystemView extends FileSystemView {
	private static final Object[] noArgs = {};
	private static final Class[] noArgTypes = {};
	
	private static Method listRootsMethod = null;
	private static boolean listRootsMethodChecked = false;
	
	/**
	 * Returns true if the given file is a root.
	 */
	public boolean isRoot(File f) {
	    if(!f.isAbsolute()) {
		return false;
	    }
	    
	    String parentPath = f.getParent();
	    if(parentPath == null) {
		return true;
	    } else {
		File parent = new File(parentPath);
		return parent.equals(f);
	    }
	}
	
	/**
	 * creates a new folder with a default folder name.
	 */
	public File createNewFolder(File containingDir) throws IOException {
	    if(containingDir == null) {
		throw new IOException("Containing directory is null:");
	    }
	    File newFolder = null;
	    // Using NT's default folder name
	    newFolder = createFileObject(containingDir, "New Folder");
	    int i = 2;
	    while (newFolder.exists() && (i < 100)) {
		newFolder = createFileObject(containingDir, "New Folder (" + i + ")");
		i++;
	    }
	    
	    if(newFolder.exists()) {
		throw new IOException("Directory already exists:" + newFolder.getAbsolutePath());
	    } else {
		newFolder.mkdirs();
	    }
	    
	    return newFolder;
	}
	
	/**
	 * Returns whether a file is hidden or not. On Windows
	 * there is currently no way to get this information from
	 * io.File, therefore always return false.
	 */
	public boolean isHiddenFile(File f) {
	    return false;
	}
	
	/**
	 * Returns all root partitians on this system. On Windows, this
	 * will be the A: through Z: drives.
	 */
	public File[] getRoots() {
	    Vector rootsVector = new Vector();
	    
	    // Create the A: drive whether it is mounted or not
	    FileSystemRoot floppy = new FileSystemRoot("A" + ":" + "\\");
	    rootsVector.addElement(floppy);
	    
	    // Run through all possible mount points and check
	    // for their existance.
	    for (char c = 'C'; c <= 'Z'; c++) {
		char device[] = {c, ':', '\\'};
		String deviceName = new String(device);
		File deviceFile = new FileSystemRoot(deviceName);
		if (deviceFile != null && deviceFile.exists()) {
		    rootsVector.addElement(deviceFile);
		}
	    }
	    File[] roots = new File[rootsVector.size()];
	    rootsVector.copyInto(roots);
	    return roots;
	}
	
	class FileSystemRoot extends File {
	    public FileSystemRoot(File f) {
		super(f, "");
	    }
	    
	    public FileSystemRoot(String s) {
		super(s);
	    }
	    
	    public boolean isDirectory() {
		return true;
	    }
	}
    }
   
    synchronized boolean askUser() {
	if (CheckServicePermission.hasFileAccessPermissions()) {
	    return true;
	}
        // Ask user
	return _apiDialog.askUser(
            ResourceManager.getString("api.file.open.title"),
            ResourceManager.getString("api.file.open.message"),
            ResourceManager.getString("api.file.open.always"));
    }


}

