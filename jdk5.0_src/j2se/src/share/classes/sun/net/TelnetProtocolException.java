/*
 * @(#)TelnetProtocolException.java	1.18 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net;

import java.io.*;

/**
 * An unexpected result was received by the client when talking to the
 * telnet server.
 * 
 * @version     1.18,12/19/03
 * @author      Jonathan Payne 
 */

public class TelnetProtocolException extends IOException {
    public TelnetProtocolException(String s) {
	super(s);
    }
}
