/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association.utility;


import java.net.URL;
import java.util.List;


/**
 * Containing funtions to retrieve association information
 *
 * @version 1.0
 */
public interface AppAssociationReader {

    /**
     * Returns the description associated with the given mime type.
     *
     * @param mimeType Given mime type
     * @return String
     */
    public abstract String getDescriptionByMimeType(String mimeType);
  
    /**
     * Returns the description associated with the given file extension.
     *
     * @param fileExt Given file extension
     * @return String
    */
    public abstract String getDescriptionByFileExt(String fileExt);

    /**
     * Returns the mime type associated with the given URL, by checking the content of 
     * the URL.
     * 
     * @param url The specified URL
     * @return String
     */
    public abstract String getMimeTypeByURL(URL url);

    /**
     * Returns the file extensione list associated with the given mime type.
     *
     * @param mimeType Given mime type
     * @return String
     */
    public abstract List getFileExtListByMimeType(String mimeType);
  
    /**
     * Returns the mime type associated with the given file extension.
     *
     * @param fileExt Given file extension
     * @return String
     */
    public abstract String getMimeTypeByFileExt(String fileExt);
  
    /**
     * Returns the icon file name associated with the given mime type.
     *
     * @param mimeType Given mime type.
     * @return icon file name
     */
    public abstract String getIconFileNameByMimeType(String mimeType);
  
    /**
     * Returns the icon file name associated with the given file extension.
     *
     * @param fileExt Given file extension.
     * @return icon file name
     */
    public abstract String getIconFileNameByFileExt(String fileExt);
 
    /**
     * Returns the action list associated with the given file extension.
     *
     * @param fileExt Given file extension
     * @return the action list
     */
    public abstract List getActionListByFileExt(String fileExt);

    /**
     * Returns the action list associated with the given mime type.
     *
     * @param mimeType Given mime type
     * @return the action list
     */
    public abstract List getActionListByMimeType(String mimeType);
    
	/**
	 * Returns true if the mime type exists in the system.
	 *
	 * @param mimeType given mimeType
	 * @return true if the mime type exists in the system
	 */
	public boolean isMimeTypeExist(String mimeType);

	/**
	 * Returns true if the file extension exists in the system.
	 * 
	 * @param fileExt given file extension 
	 * @return true if the file extension exists in the system
	 */
	public boolean isFileExtExist(String fileExt);
}
