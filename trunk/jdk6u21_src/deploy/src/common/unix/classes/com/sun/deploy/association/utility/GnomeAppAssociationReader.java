/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association.utility;


import java.net.URL;
import java.util.List;

/**
 * Concrete implementation of the AppAssociationReader class for Gnome.
 */
public class GnomeAppAssociationReader implements AppAssociationReader {

    /**
     * Returns the description associated with the given mime type.
     *
     * @param mimeType Given mime type
     * @return String
     */
    public String getDescriptionByMimeType(String mimeType) {
        return GnomeAssociationUtil.getDescriptionByMimeType(mimeType);
    }
  
    /**
     * Returns the description associated with the given file extension.
     *
     * @param fileExt Given file extension
     * @return String
     */
    public String getDescriptionByFileExt(String fileExt) {
        // Removes the leading '.' character from the file extension if exists.
        fileExt = AppUtility.removeDotFromFileExtension(fileExt);        
        if (getMimeTypeByFileExt(fileExt) == null) {
            return null;
        } else {
            return getDescriptionByMimeType(getMimeTypeByFileExt(fileExt));
        }
    }

    /**
     * Returns the mime type associated with the given URL, by checking the content of 
     *
     * the URL.
     * @param url The specified URL
     * @return String
     */
    public String getMimeTypeByURL(URL url) {
        return GnomeAssociationUtil.getMimeTypeByURL(url);
    }        

    /**
     * Returns the file extensione list associated with the given mime type.
     *
     * @param mimeType Given mime type
     * @return String
     */
    public List getFileExtListByMimeType(String mimeType) {
        return GnomeAssociationUtil.getFileExtListByMimeType(mimeType);
    }
  
    /**
     * Returns the mime type associated with the given file extension.
     *
     * @param fileExt Given file extension
     * @return String
     */
    public String getMimeTypeByFileExt(String fileExt) {
        // Removes the leading '.' character from the file extension if exists.
        fileExt = AppUtility.removeDotFromFileExtension(fileExt);        
        return GnomeAssociationUtil.getMimeTypeByFileExt(fileExt);
    }
  
    /**
     * Returns the icon file name associated with the given mime type.
     *
     * @param mimeType Given mime type.
     * @return String
     */
    public String getIconFileNameByMimeType(String mimeType) {
        return GnomeAssociationUtil.getIconFileNameByMimeType(mimeType);
    }
  
    /**
     * Returns the icon file name associated with the given file extension.
     *
     * @param fileExt Given file extension.
     * @return String
     */
    public String getIconFileNameByFileExt(String fileExt) {
        // Remove the leading '.' character from the file extension if exists.
        fileExt = AppUtility.removeDotFromFileExtension(fileExt);        
        if (getMimeTypeByFileExt(fileExt) == null) {
            return null;
        } else {       
            return getIconFileNameByMimeType(getMimeTypeByFileExt(fileExt));
        }
    }
 
    /**
     * Returns the action list associated with the given file extension.
     *
     * @param fileExt Given file extension
     * @return List The action list
     */
    public List getActionListByMimeType(String mimeType) {
        return GnomeAssociationUtil.getActionListByMimeType(mimeType);
    }

    /**
     * Returns the action list associated with the given mime type.
     *
     * @param mimeType Given mime type
     * @return List The action list
     */
    public List getActionListByFileExt(String fileExt) {
        // Remove the leading '.' character from the file extension if exists.
        fileExt = AppUtility.removeDotFromFileExtension(fileExt);        
        if (getMimeTypeByFileExt(fileExt) == null) {
            return null;
        } else {
            return getActionListByMimeType(getMimeTypeByFileExt(fileExt));        
        }
    }

	/**
	 * Returns true if the mime type exists in the MIME database.
	 *
	 * @param mimeType given mimeType
	 * @return true if the mime type exists in the MIME database
	 */
	public boolean isMimeTypeExist(String mimeType) {
		return GnomeAssociationUtil.isMimeTypeExist(mimeType); 
	}
    
	/**
	 * Returns true if the file extension exists in the MIME database.
	 * 
	 * @param fileExt given file extension 
	 * @return true if the file extension exists in the MIME database
	 */
	public boolean isFileExtExist(String fileExt) {
		// Remove the leading '.' character from the file extension if exists.
		fileExt = AppUtility.removeDotFromFileExtension(fileExt);        
           
        return GnomeAssociationUtil.isFileExtExist(fileExt);
	}
}
