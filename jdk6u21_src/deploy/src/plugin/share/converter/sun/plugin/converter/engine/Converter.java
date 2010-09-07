/*
 * @(#)Converter.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter.engine;

import sun.plugin.converter.util.*;
import java.util.Vector;
import java.io.*;


public interface Converter {

	
    /*
      Do we show progress in Stdout
      */
    public void setShowProgressStdOut(boolean b);
    public boolean isShowProgressStdOut();
	
    /*
      Name of template file to be used
      */
    public void setTemplateFilePath(String fileName) throws FileNotFoundException;
    public String getTemplateFilePath();

    /*
      Do we process into subdirectories
      */
    public void setRecurse(boolean b);
    public boolean isRecurse();

    /*
      Stream to direct log information
      */
    public void setLogFile(java.io.File f);
    public java.io.File getLogFile();
	
    /*
      Do we create a log?
      */
    public void setCreateLog(boolean b);
    public boolean isCreateLog();


    /*
      Progress can be posted out via the ConverterProgressEvent object
      */
    public void addConverterProgressListener(ConverterProgressListener l);
    public void removedConverterProgressListener(ConverterProgressListener l);
	
}
