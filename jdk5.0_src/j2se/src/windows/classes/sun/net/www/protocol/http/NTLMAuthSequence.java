/*
 * @(#)NTLMAuthSequence.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.http;

import java.io.IOException;
import sun.misc.BASE64Encoder;
import sun.misc.BASE64Decoder;

/*
 * Hooks into Windows implementation of NTLM.
 * This class will be replaced if a cross-platform version of NTLM
 * is implemented in the future.
 */

public class NTLMAuthSequence {

    private String username;
    private String password;
    private String ntdomain;
    private int state; 
    private long crdHandle;
    private long ctxHandle;

    static {
	initFirst();
    }

    NTLMAuthSequence (String username, String password, String ntdomain) 
    throws IOException
    {
	this.username = username;
	this.password = password;
	this.ntdomain = ntdomain;
	state = 0;
	crdHandle = getCredentialsHandle (username, ntdomain, password);
	if (crdHandle == 0) {
	    throw new IOException ("could not get credentials handle");
	}
    }

    public String getAuthHeader (String token) throws IOException {
	byte[] input = null;
	if (token != null)
	    input = (new BASE64Decoder()).decodeBuffer(token);
	byte[] b = getNextToken (crdHandle, input);
	if (b == null)
	    throw new IOException ("Internal authentication error");
	return (new B64Encoder()).encode (b);
    }

    private native static void initFirst ();
 
    private native long getCredentialsHandle (String user, String domain, String password);

    private native byte[] getNextToken (long crdHandle, byte[] lastToken);
}

class B64Encoder extends BASE64Encoder {
    protected int bytesPerLine () {
	return 1024;
    }
}
