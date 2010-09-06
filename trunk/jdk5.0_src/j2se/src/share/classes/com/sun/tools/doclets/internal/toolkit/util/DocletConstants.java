/*
 * @(#)DocletConstants.java	1.7 04/03/14
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.util;

/**
 * Stores all constants for a Doclet.  Extend this class if you have doclet
 * specific constants to add.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @author Jamie Ho
 * @since 1.5
 */
public class DocletConstants {
    
    /**
     * The default package name.
     */
    public static final String DEFAULT_PACKAGE_NAME = "&lt;Unnamed&gt;";
    
    /**
     * The default package file name.
     */
    public static final String DEFAULT_PACKAGE_FILE_NAME = "default";
    
    /**
     * The anchor for the default package.
     */
    public static final String UNNAMED_PACKAGE_ANCHOR = "unnamed_package";
    
    /**
     * The name of the doc files directory.
     */
    public static final String DOC_FILES_DIR_NAME = "doc-files";
    
    /**
     * The default amount of space between tab stops.
     */
    public static final int DEFAULT_TAB_STOP_LENGTH = 8;
    
    /**
     * The name of the directory where we will copy resource files to.
     */
    public static final String RESOURE_DIR_NAME = "resources";
    
    /**
     * The source output directory name
     */
    public static final String SOURCE_OUTPUT_DIR_NAME = "src-html/";   
    
    /**
     * The name of the package list file.
     */
    public static final String PACKAGE_LIST_FILE_NAME = "package-list";
    
    /**
     * The line seperator for the current operating system.
     */
    public static final String NL = System.getProperty("line.separator");
}
