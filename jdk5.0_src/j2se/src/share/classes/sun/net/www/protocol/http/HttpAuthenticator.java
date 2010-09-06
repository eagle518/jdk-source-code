/*
 * @(#)HttpAuthenticator.java	1.14 04/05/18
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.http;

import java.net.URL;

/**
 * An interface for all objects that implement HTTP authentication.
 * See the HTTP spec for details on how this works in general.
 * A single class or object can implement an arbitrary number of
 * authentication schemes.  
 *
 * @author David Brown
 *
 * @deprecated -- use java.net.Authenticator instead
 * @see java.net.Authenticator
 */
//
// REMIND:  Unless compatibility with sun.* API's from 1.2 to 2.0 is
// a goal, there's no reason to carry this forward into JDK 2.0.
@Deprecated
public interface HttpAuthenticator {
    

    /**
     * Indicate whether the specified authentication scheme is
     * supported.  In accordance with HTTP specifications, the
     * scheme name should be checked in a case-insensitive fashion.
     */

    boolean schemeSupported (String scheme);

    /**
     * Returns the String that should be included in the HTTP
     * <B>Authorization</B> field.  Return null if no info was
     * supplied or could be found.
     * <P>
     * Example:
     * --> GET http://www.authorization-required.com/ HTTP/1.0
     * <-- HTTP/1.0 403 Unauthorized
     * <-- WWW-Authenticate: Basic realm="WallyWorld"
     * call schemeSupported("Basic"); (return true)
     * call authString(u, "Basic", "WallyWorld", null);
     *   return "QWadhgWERghghWERfdfQ=="
     * --> GET http://www.authorization-required.com/ HTTP/1.0
     * --> Authorization: Basic QWadhgWERghghWERfdfQ==
     * <-- HTTP/1.0 200 OK
     * <B> YAY!!!</B>
     */

    public String authString (URL u, String scheme, String realm);

}




