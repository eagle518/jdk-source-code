/*
 * @(#)CanceledDownloadException.java	1.4 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.net;
import java.io.IOException;

public class CanceledDownloadException extends Exception {
    CanceledDownloadException() {
	// This string does not need localization. This should never
	// be seen by a user
	super("cancled download");
    }
}
