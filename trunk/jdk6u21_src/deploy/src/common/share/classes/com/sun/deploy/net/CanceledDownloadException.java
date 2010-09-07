/*
 * @(#)CanceledDownloadException.java	1.4 03/12/19
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.deploy.net;
import java.io.IOException;

public class CanceledDownloadException extends Exception {
    CanceledDownloadException() {
	// This string does not need localization. This should never
	// be seen by a user
	super("cancled download");
    }
}
