/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association.utility;


import java.net.URL;

/**
 * Utility class containing shared methods.
 *
 * @version 1.0
 */
public class AppUtility {

    /**
     * Suppress default constructor for noninstantiability.
     */
    private AppUtility() {}
    
    /**
     * Returns the file extension from the file part of the URL.
     * The returned file extension include the leading '.' character.
     * <P>
     * For example: if the URL is http://www.sun.com/index.html, the 
     * returned file extension is ".html".
     *
     * @param url the specified URL
     * @return the file extension of the file part of the URL.
     */
    public static String getFileExtensionByURL(URL url) {
        String trimFile = url.getFile().trim();

        if (trimFile == null || trimFile.equals("") || trimFile.equals("/") ) {
            return null;
        }
         
        int strIndex = trimFile.lastIndexOf("/");
        String filePart = trimFile.substring(strIndex + 1, trimFile.length());

        strIndex = filePart.lastIndexOf(".");
        if (strIndex == -1 || strIndex == filePart.length() - 1) {
            return null;
        } else {
            String fileExt = filePart.substring(strIndex, filePart.length());

            return fileExt;
        }
    }

    /**
     * Removes the leading '.' character from the specified file extension.
     *
     * @param fileExt the specified file extension.
     * @return file extension without a leading '.' character.
     * @see com.sun.deploy.association.Association#addFileExtension and 
     * com.sun.deploy.association.AssociationService#getFileExtensionAssociation. 
     */
    public static String removeDotFromFileExtension(String fileExt) {
        String temFileExt = fileExt;
        if (fileExt.charAt(0) == '.') {
            temFileExt = fileExt.substring(1, fileExt.length());
        }
        
        return temFileExt;
    }

    /**
     * Adds one leading '.' character for the specified file extension.
     * If the leading '.' character already exists, it just returns. 
     *
     * @param fileExt the specified file extension.
     * @return file extension with a leading '.' character.
     * @see com.sun.deploy.association.Association#addFileExtension and 
     * com.sun.deploy.association.AssociationService#getFileExtensionAssociation. 
     */
    public static String addDotToFileExtension(String fileExt) {
        String temFileExt = fileExt;
        if (fileExt.charAt(0) != '.') {
            String dotStr = ".";
            temFileExt = dotStr.concat(fileExt);
        }
       
        return temFileExt;
    }
}

    
