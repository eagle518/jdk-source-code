/*
 * @(#)UrlUtil.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.toolkit.url;

import java.net.MalformedURLException;
import java.io.UnsupportedEncodingException;

/**
 * Utilities for dealing with URLs.
 * @author Vincent Ryan
 * @version 1.9 03/12/19
 */

final public class UrlUtil {

    // To prevent creation of this static class
    private UrlUtil() {
    }

    /**
     * Decode a URI string (according to RFC 2396).
     */
    public static final String decode(String s) throws MalformedURLException {
	try {
	    return decode(s, "8859_1");
	} catch (UnsupportedEncodingException e) {
	    // ISO-Latin-1 should always be available?
	    throw new MalformedURLException("ISO-Latin-1 decoder unavailable");
	}
    }

    /**
     * Decode a URI string (according to RFC 2396).
     *
     * Three-character sequences '%xy', where 'xy' is the two-digit
     * hexadecimal representation of the lower 8-bits of a character,
     * are decoded into the character itself.
     *
     * The string is subsequently converted using the specified encoding
     */
    public static final String decode(String s, String enc) 
	throws MalformedURLException, UnsupportedEncodingException {

        int length = s.length();
	byte[] bytes = new byte[length];
	int j = 0;

        for (int i = 0; i < length; i++) {
            if (s.charAt(i) == '%') {
		i++;  // skip %
		try {
		    bytes[j++] = (byte)
			Integer.parseInt(s.substring(i, i + 2),	16);

		} catch (Exception e) {
		    throw new MalformedURLException("Invalid URI encoding: " + s);
		}
		i++;  // skip first hex char; for loop will skip second one
            } else {
		bytes[j++] = (byte) s.charAt(i);
            }
        }

	return new String(bytes, 0, j, enc);
    }

    /**
     * Encode a string for inclusion in a URI (according to RFC 2396).
     *
     * Unsafe characters are escaped by encoding them in three-character
     * sequences '%xy', where 'xy' is the two-digit hexadecimal representation
     * of the lower 8-bits of the character.
     *
     * The question mark '?' character is also escaped, as required by RFC 2255.
     *
     * The string is first converted to the specified encoding.
     * For LDAP (2255), the encoding must be UTF-8.
     */
    public static final String encode(String s, String enc) 
	throws UnsupportedEncodingException {

	byte[] bytes = s.getBytes(enc);
	int count = bytes.length;

	/*
	 * From RFC 2396:
         *
	 *     mark = "-" | "_" | "." | "!" | "~" | "*" | "'" | "(" | ")"
	 * reserved = ";" | "/" | ":" | "?" | "@" | "&" | "=" | "+" | "$" | ","
	 */
	final String allowed = "=,+;.'-@&/$_()!~*:"; // '?' is omitted
	char[] buf = new char[3 * count];
	int j = 0;

	for (int i = 0; i < count; i++) {
	    if ((bytes[i] >= 0x61 && bytes[i] <= 0x7A) || // a..z
		(bytes[i] >= 0x41 && bytes[i] <= 0x5A) || // A..Z
		(bytes[i] >= 0x30 && bytes[i] <= 0x39) || // 0..9
		(allowed.indexOf(bytes[i]) >= 0)) {
		buf[j++] = (char) bytes[i];
	    } else {
		buf[j++] = '%';
		buf[j++] = Character.forDigit(0xF & (bytes[i] >>> 4), 16);
		buf[j++] = Character.forDigit(0xF & bytes[i], 16);
	    }
	}
	return new String(buf, 0, j);
    }
}
