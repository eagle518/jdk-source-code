/*
 * @(#)ProgressListener.java	1.2 04/02/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.net;

import java.util.EventListener;

/**
 * ProgressListener is an interface to be implemented by parties
 * interested to be notified of progress in network input stream. 
 *
 * @author Stanley Man-Kit Ho 
 */
public interface ProgressListener extends EventListener
{
    /**
     * Start progress. 
     */
    public void progressStart(ProgressEvent evt);
    
    /**
     * Update progress. 
     */
    public void progressUpdate(ProgressEvent evt);
    
    /**
     * Finish progress. 
     */    
    public void progressFinish(ProgressEvent evt);
}



