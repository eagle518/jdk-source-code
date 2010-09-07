/*
 * @(#)ExtendedService.java	1.9 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javax.jnlp;

import java.io.IOException;
import java.awt.print.Printable;
import java.awt.print.PageFormat;

/**
 * <code>ExtendedService</code> provides additional support to the current 
 * JNLP API, which allow applications to open a specific file in the client's file system. 
 *
 *
 * @since 1.5
 */


public interface ExtendedService {
 /**  
   *  Asks the user the permission to open the specified file if the file
   *  has not been opened before.
   * 
   *  The contents of the file is returned as a FileContents object.
   *  The returned FileContents object contains the contents
   *  the file.
   *
   *  @param  file  the file object
   *
   *  @return A FileContent object with information about the
   *          opened file
   *
   *  @exception IOException - if there is any I/O error
   */

    public FileContents openFile(java.io.File file) throws IOException; 

 /** 
   *  Asks the user the permission to open the specified list of files if
   *  any of the files has not been opened before.
   *
   *  The contents of each file is returned as a FileContents object in the FileContents array.
   *  The returned FileContents object contains the contents
   *  the file.
   *
   *  @param  files  the array of files 
   *
   *  @return A FileContent[] object with information about each
   *          opened file
   *
   *  @exception IOException - if there is any I/O error
   */
    public FileContents[] openFiles(java.io.File[] files) throws IOException; 

}
