/*
 * @(#)ConverterProgressEvent.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

import sun.plugin.converter.ResourceHandler;

public class ConverterProgressEvent extends java.util.EventObject {


    public static final int PREPARING = -10;
    public static final int CONVERTING = -11;
    public static final int COPYING = -12;
    public static final int DONE = -13;
    public static final int ALL_DONE = -14;
    public static final int DEST_DIR_NOT_CREATED = -15;
    public static final int ERROR = -16;
	
    private String sourcePath = "";
    private String destinationPath = "";
    private String backupPath = "";
    private String currentFile = "";
    private int status;
    private int filesProcessed = 0;
    private int appletsFound = 0;
    private int errorsFound = 0;
	
    public ConverterProgressEvent(Object _source) {
	super(_source);
	setStatus(PREPARING);
    }
    public ConverterProgressEvent(Object _source, 
				  String _sourcePath,
				  String _destinationPath,
				  String _currentFile, 
				  int _status, 
				  int _filesProcessed, 
				  int _appletsFound, 
				  int _errorsFound) {
	super(_source);
	currentFile = _currentFile;
	status = _status;
	filesProcessed = _filesProcessed;
	appletsFound = _appletsFound;
	errorsFound = _errorsFound;
	sourcePath = _sourcePath;
	destinationPath = _destinationPath;
    }
	
    public String getCurrentFile() { return currentFile; }
    public void setCurrentFile(String s) { currentFile = s; }

    public int getStatus() { return status; }
    public String getStatusText() {
	switch(status) {
	case PREPARING:
	    return ResourceHandler.getMessage("progress_event.preparing");
	case CONVERTING:
	    return ResourceHandler.getMessage("progress_event.converting");
	case COPYING:
	    return ResourceHandler.getMessage("progress_event.copying");
	case DONE:
	case ALL_DONE:
	    return ResourceHandler.getMessage("progress_event.done");
	case DEST_DIR_NOT_CREATED:
	    return ResourceHandler.getMessage("progress_event.destdirnotcreated");
	case ERROR:
	    return ResourceHandler.getMessage("progress_event.error");
	}
	return "";
    }
			
    public void setStatus(int s) {
	if(s == CONVERTING || s == DONE || s == COPYING || s == ALL_DONE || s == DEST_DIR_NOT_CREATED || s == ERROR) status = s;
	else status = PREPARING;
    }

    public int getFilesProcessed() {  return filesProcessed; }
    public void setFilesProcessed(int i) { if(i>=0) filesProcessed = i; }

    public int getAppletsFound() { return appletsFound; }
    public void setAppletsFound(int i) { if(i>=0) appletsFound = i; }

    public int getErrorsFound() { return errorsFound; }
    public void setErrorsFound(int i) { if(i>=0) errorsFound = i; }
	
    public String getSourcePath() { return sourcePath; }
    public void setSourcePath(String s) { sourcePath = s; }
	
    public String getDestinationPath() { return destinationPath; }
    public void setDestinationPath(String s) { destinationPath = s; }
	
    public String getBackupPath() { return backupPath; }
    public void setBackupPath(String s) { backupPath = s; }

    public String toString() {
	String str  = "[ConverterProgressEvent";
	str += sourcePath;
	str += destinationPath;
	str += backupPath;
	str += currentFile;
	str += status;
	str += filesProcessed;
	str += appletsFound;
	str += errorsFound+"]";
		
	return str;
    }		

}


