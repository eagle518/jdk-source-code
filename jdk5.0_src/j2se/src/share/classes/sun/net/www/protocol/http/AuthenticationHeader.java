/*
 * @(#)AuthenticationHeader.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.net.www.protocol.http;

import sun.net.www.*;
import java.util.Iterator;
import java.util.HashMap;

/**
 * This class is used to parse the information in WWW-Authenticate: and Proxy-Authenticate:
 * headers. It searches among multiple header lines and within each header line
 * for the best currently supported scheme. It can also return a HeaderParser
 * containing the challenge data for that particular scheme.
 *
 * Some examples:
 *
 * WWW-Authenticate: Basic realm="foo" Digest realm="bar" NTLM
 *  Note the realm parameter must be associated with the particular scheme.
 *
 * or
 * 
 * WWW-Authenticate: Basic realm="foo"
 * WWW-Authenticate: Digest realm="foo",qop="auth",nonce="thisisanunlikelynonce"
 * WWW-Authenticate: NTLM
 * 
 * or 
 *
 * WWW-Authenticate: Basic realm="foo"
 * WWW-Authenticate: NTLM ASKAJK9893289889QWQIOIONMNMN
 *
 * The last example shows how NTLM breaks the rules of rfc2617 for the structure of 
 * the authentication header. This is the reason why the raw header field is used for ntlm.
 *
 * At present, the class chooses schemes in following order :
 * 	1. Digest 2. NTLM (if supported) 3. Basic
 * 
 * This choice can be modified by setting a system property:
 *
 * 	-Dhttp.auth.preference="scheme"
 *
 * which in this case, specifies that "scheme" should be used as the auth scheme when offered
 * disregarding the default prioritisation. If scheme is not offered then the default priority
 * is used.
 */

public class AuthenticationHeader {
    
    MessageHeader rsp; // the response to be parsed
    HeaderParser preferred; 
    String preferred_r;	// raw Strings

    static String authPref=null;

    static {
	authPref = (String) java.security.AccessController.doPrivileged(
	    new java.security.PrivilegedAction() {
	         public Object run() {
		     return System.getProperty("http.auth.preference");
	         }
	    });
	if (authPref != null) {
	    authPref = authPref.toLowerCase();
	}
    }

    String hdrname; // Name of the header to look for

    /**
     * parse a set of authentication headers and choose the preferred scheme
     * that we support
     */
    public AuthenticationHeader (String hdrname, MessageHeader response) {
	rsp = response;
	this.hdrname = hdrname;
	schemes = new HashMap();
	parse();
    }
 
    /* we build up a map of scheme names mapped to SchemeMapValue objects */
    static class SchemeMapValue {
	SchemeMapValue (HeaderParser h, String r) {raw=r; parser=h;}
	String raw;	
	HeaderParser parser;
    }

    HashMap schemes; 

    /* Iterate through each header line, and then within each line.
     * If multiple entries exist for a particular scheme (unlikely)
     * then the last one will be used. The
     * preferred scheme that we support will be used.
     */
    private void parse () {
	Iterator iter = rsp.multiValueIterator (hdrname);
	while (iter.hasNext()) {
	    String raw = (String)iter.next();
	    HeaderParser hp = new HeaderParser (raw);
	    Iterator keys = hp.keys();
	    int i, lastSchemeIndex;
	    for (i=0, lastSchemeIndex = -1; keys.hasNext(); i++) {
		keys.next();
		if (hp.findValue(i) == null) { /* found a scheme name */
		    if (lastSchemeIndex != -1) {
			HeaderParser hpn = hp.subsequence (lastSchemeIndex, i);
			String scheme = hpn.findKey(0);
			schemes.put (scheme, new SchemeMapValue (hpn, raw));
		    }
		    lastSchemeIndex = i;
		}
	    }
	    if (i > lastSchemeIndex) {
		HeaderParser hpn = hp.subsequence (lastSchemeIndex, i);
		String scheme = hpn.findKey(0);
		schemes.put (scheme, new SchemeMapValue (hpn, raw));
	    }
	}

	/* choose the best of them */
	SchemeMapValue v;
	if (authPref == null || (v=(SchemeMapValue)schemes.get (authPref)) == null) {
	    if ((v=(SchemeMapValue)schemes.get ("digest")) == null) {
	        if (!NTLMAuthentication.isSupported() || 
			    ((v=(SchemeMapValue)schemes.get("ntlm"))==null)) {
		    v = (SchemeMapValue)schemes.get ("basic");
	        }
	    }
	}
	if (v != null) {
	    preferred = v.parser;
	    preferred_r = v.raw;;
	} 
    }

    /**
     * return a header parser containing the preferred authentication scheme (only).
     * The preferred scheme is the strongest of the schemes proposed by the server.
     * The returned HeaderParser will contain the relevant parameters for that scheme
     */
    public HeaderParser headerParser() {
	return preferred;
    }

    /**
     * return the name of the preferred scheme
     */
    public String scheme() {
	if (preferred != null) {
	    return preferred.findKey(0);
	} else {
	    return null;
	}
    }

    /* return the raw header field for the preferred/chosen scheme */

    public String raw () {
	return preferred_r;
    }

    /**
     * returns true is the header exists and contains a recognised scheme
     */
    public boolean isPresent () {
	return preferred != null;
    }
}
