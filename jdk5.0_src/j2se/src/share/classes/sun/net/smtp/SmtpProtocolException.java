/*
 * @(#)SmtpProtocolException.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.smtp;

import java.io.IOException;

/**
 * This exeception is thrown when unexpected results are returned during
 * an SMTP session.
 */
public class SmtpProtocolException extends IOException {
    SmtpProtocolException(String s) {
	super(s);
    }
}

