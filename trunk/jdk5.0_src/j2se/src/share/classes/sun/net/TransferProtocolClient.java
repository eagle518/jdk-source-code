/*
 * @(#)TransferProtocolClient.java	1.33 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net;

import java.lang.StringIndexOutOfBoundsException;
import java.io.*;
import java.util.Vector;
import sun.net.NetworkClient;

/**
 * This class implements that basic intefaces of transfer protocols.
 * It is used by subclasses implementing specific protocols.
 *
 * @version 	1.33, 12/19/03
 * @author 	Jonathan Payne
 * @see 	sun.net.ftp.FtpClient
 * @see		sun.net.nntp.NntpClient
 */

public class TransferProtocolClient extends NetworkClient {
    static final boolean debug = false;

    /** Array of strings (usually 1 entry) for the last reply
	from the server. */
    protected Vector	serverResponse = new Vector(1);

    /** code for last reply */
    protected int	lastReplyCode;


    /**
     * Pulls the response from the server and returns the code as a
     * number. Returns -1 on failure.
     */
    public int readServerResponse() throws IOException {
	StringBuffer	replyBuf = new StringBuffer(32);
	int		c;
	int		continuingCode = -1;
	int		code;
	String		response;

	serverResponse.setSize(0);
	while (true) {
	    while ((c = serverInput.read()) != -1) {
		if (c == '\r') {
		    if ((c = serverInput.read()) != '\n')
			replyBuf.append('\r');
		}
		replyBuf.append((char)c);
		if (c == '\n')
		    break;
	    }
	    response = replyBuf.toString();
	    replyBuf.setLength(0);
	    if (debug) {
		System.out.print(response);
	    }

	    if (response.length() == 0) {
		code = -1;
	    } else {
	        try {
		    code = Integer.parseInt(response.substring(0, 3));
	        } catch (NumberFormatException e) {
		    code = -1;
	        } catch (StringIndexOutOfBoundsException e) {
		    /* this line doesn't contain a response code, so
		       we just completely ignore it */
		    continue;
		}
	    }
	    serverResponse.addElement(response);
	    if (continuingCode != -1) {
		/* we've seen a XXX- sequence */
		if (code != continuingCode ||
		    (response.length() >= 4 && response.charAt(3) == '-')) {
		    continue;
		} else {
		    /* seen the end of code sequence */
		    continuingCode = -1;
		    break;
		}
	    } else if (response.length() >= 4 && response.charAt(3) == '-') {
		continuingCode = code;
		continue;
	    } else {
		break;
	    }
	}

	return lastReplyCode = code;
    }

    /** Sends command <i>cmd</i> to the server. */
    public void sendServer(String cmd) {
	serverOutput.print(cmd);
	if (debug) {
	    System.out.print("Sending: " + cmd);
	}
    }

    /** converts the server response into a string. */
    public String getResponseString() {
	return (String) serverResponse.elementAt(0);
    }

    /** Returns all server response strings. */
    public Vector getResponseStrings() {
	return serverResponse;
    }

    /** standard constructor to host <i>host</i>, port <i>port</i>. */
    public TransferProtocolClient(String host, int port) throws IOException {
	super(host, port);
    }

    /** creates an uninitialized instance of this class. */
    public TransferProtocolClient() {}
}
