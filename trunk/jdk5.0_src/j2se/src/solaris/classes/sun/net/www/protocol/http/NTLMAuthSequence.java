/*
 * @(#)NTLMAuthSequence.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.http;

import java.io.IOException;

public class NTLMAuthSequence {

    NTLMAuthSequence (String username, String password, String ntdomain) {
    }

    public String getAuthHeader (String token) throws IOException {
	throw new UnsupportedOperationException ("NTLM not supported on this OS");
    }
}
