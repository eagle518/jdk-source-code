/*
 * @(#)FtpLoginException.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.ftp;

import java.io.*;

/**
 * This exception is thrown when an error is encountered during an
 * FTP login operation. 
 * 
 * @version 	1.18, 12/19/03
 * @author	Jonathan Payne
 */
public class FtpLoginException extends FtpProtocolException {
    FtpLoginException(String s) {
	super(s);
    }
}

