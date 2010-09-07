/*
 * @(#)DefaultSet.java	1.16 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

import sun.plugin.converter.util.*;
import java.io.*;
import java.util.*;
import javax.swing.*;



public class DefaultSet implements ConvertSet {

    private File sourcePath;
    private File destPath;
    private File backupPath;
	
    private Vector files = new Vector();
    private String destPathSuffix = "_CONV";
    private String backupPathSuffix = "_BAK";
    private int maxFileCharLength = 27;
    private boolean debug = true;
    private FilenameFilter cFlexFilter;
    
    
    public DefaultSet() {
	try {
	    FlexFilter filter = new FlexFilter();
	    filter.addDescriptor("*.html");
	    filter.addDescriptor("*.htm");
	    filter.addDescriptor("*.asp");
	    filter.setFilesOnly(true);
	    //  Set up the default sourcePath
	    setSourcePath(getDefaultSourcePath(),filter);
	    setDestinationPath(getDefaultDestPath());
	    setBackupPath(getDefaultBackupPath());
	}
	catch(NotDirectoryException e) {
	    e.printStackTrace();
	}		
    }
		
    /*  Setup the source path */
    public void setSourcePath(java.io.File f, FilenameFilter filter) throws NotDirectoryException {
	if(!f.isDirectory())
	    throw new NotDirectoryException(f.getAbsolutePath());
	sourcePath = f;
	setFiles(filter);
	cFlexFilter = filter;
    }
 
    public java.io.File getSourcePath() {
	if(sourcePath == null)
	    return getDefaultSourcePath();
	else{
	    //for the purposes of getBackupPath, we should make sure
	    //it doesn't end in "/."
	    if(sourcePath.getAbsolutePath().endsWith("/.")){
		sourcePath = new File(sourcePath.getParent());
		return sourcePath;
	    }
	    return sourcePath; 
	}
    }

    public java.io.File getDefaultSourcePath() { 
	return new File(System.getProperty("user.dir"));
    }

    public void setBackupPath(java.io.File f){	
	backupPath = f;	
    }

    public java.io.File getBackupPath() 
    { 
	if(backupPath == null)
	    return getDefaultBackupPath();
	else{
    	    return backupPath; 
	}
    }

    public java.io.File getDefaultBackupPath() {
	//  Store the name of the source directory 
		
	String dir = getSourcePath().getName();
	//  Store the path leading up the the source directory
	String destDir = sourcePath.getParent();
	dir = truncateString(dir,maxFileCharLength);

	dir += backupPathSuffix;
	return (new File(destDir,dir));
    }
		
    public String truncateString(String dir,int length) {
	//  If the name of the source dir is too long to take the suffix chars,
	//  we crop the source name before adding the suffix
	if(dir.length()>length) dir = dir.substring(0,length-1);
	return dir;
    }
		
    /* Setup the destination path */
    public void setDestinationPath(java.io.File f) {
	destPath = f;
    }
		
    public java.io.File getDestinationPath() { return destPath; }
    public java.io.File getDefaultDestPath() {
	return sourcePath;
    }

    /*  Setup the files in the source path to be converted  */
    public void setFiles(FilenameFilter f)
    {
	String[] s = sourcePath.list(f);
	files = new Vector();
	for(int i = 0;i<s.length;i++)
	{
	    files.addElement(s[i]);
	}
    }

    public void setFile(File fileName)
    {
	sourcePath = fileName.getParentFile();
	files = new Vector();
	files.addElement(fileName.getName());		
    }

    public Enumeration getFilesEnumeration()
    { 
	return files.elements();
    }

    public Vector getFiles()
    {
	return files;
    }

    public int getFileCount()
    {
	return files.size();
    }

    public FilenameFilter getFilenameFilter() {
	return cFlexFilter;
    }

    public String toString() {
	String str = "\nSource Path:  "+sourcePath.toString();
	str += "\nFile count:  "+files.size();
	str += "\nFiles:  ";
	for(int i = 0;i<files.size();i++)
	    str += "\n     "+(String)files.elementAt(i);
	str += "\nDest. Path:  "+destPath.toString();
	str += "\nBackup Path:  "+backupPath.toString();
	str += "\nCopy All:  "+false;

	str += "\nDest Path Suffix:  "+destPathSuffix;
	str += "\nMax File Char Length:  "+27;
	str += "\ndebug:  "+true;
		
	return str;
    }
}


