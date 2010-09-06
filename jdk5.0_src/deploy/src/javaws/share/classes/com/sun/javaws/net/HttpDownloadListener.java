/*
 * @(#)HttpDownloadListener.java	1.4 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.net;

/** Called to track progress, as well as to check if operation should be canceled */
public interface HttpDownloadListener {
    /** Called periodically with information about downloa progress. Total will be zero, if
     *  download size is unknown. A call with (0, total) is guaranteed after successful connection,
     *  and a (total, total) at the end (given total != 0). If false is returned, the download
     *  will be cancled.
     */
    public boolean downloadProgress(int downloaded, int total);
}

