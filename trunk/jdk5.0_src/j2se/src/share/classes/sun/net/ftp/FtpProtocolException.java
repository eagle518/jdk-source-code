/*
 * @(#)FtpProtocolException.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.ftp;

import java.io.*;

/**
 * This exeception is thrown when unexpected results are returned during
 * an FTP session.
 *
 * @version	1.18, 12/19/03
 * @author	Jonathan Payne
 */
public class FtpProtocolException extends IOException {
    FtpProtocolException(String s) {
	super(s);
    }
}

