/*
 * @(#)ConvertSet.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

import sun.plugin.converter.util.*;
import java.util.*;


public interface ConvertSet {


    /*
      A directory
      */
    public void setSourcePath(java.io.File f, java.io.FilenameFilter filter) throws NotDirectoryException;
    public java.io.File getSourcePath();

    /*
      A directory
      */
    public void setDestinationPath(java.io.File f);
    public java.io.File getDestinationPath();
	
    public void setBackupPath(java.io.File f);
    public java.io.File getBackupPath();

    /*If file is mentioned*/
    public void setFile(java.io.File f);

    /* get the files */
    public Enumeration getFilesEnumeration();
	
    public String toString();
    public java.io.FilenameFilter getFilenameFilter();
	
}
