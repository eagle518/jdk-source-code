/**
 * @(#)Convert.java	1.16 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.util;

/** Utility class for static conversion methods between numbers
 *  and strings in various formats.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Convert {

    /** Convert string to integer.
     */
    public static int string2int(String s, int radix)
	throws NumberFormatException {
	if (radix == 10) {
	    return Integer.parseInt(s, radix);
	} else {
	    char[] cs = s.toCharArray();
	    int limit = Integer.MAX_VALUE / (radix/2);
	    int n = 0;
	    for (int i = 0; i < cs.length; i++) {
		int d = Character.digit(cs[i], radix);
		if (n < 0 ||
		    n > limit ||
		    n * radix > Integer.MAX_VALUE - d)
		    throw new NumberFormatException();
		n = n * radix + d;
	    }
	    return n;
	}
    }

    /** Convert string to long integer.
     */
    public static long string2long(String s, int radix)
	throws NumberFormatException {
	if (radix == 10) {
	    return Long.parseLong(s, radix);
	} else {
	    char[] cs = s.toCharArray();
	    long limit = Long.MAX_VALUE / (radix/2);
	    long n = 0;
	    for (int i = 0; i < cs.length; i++) {
		int d = Character.digit(cs[i], radix);
		if (n < 0 ||
		    n > limit ||
		    n * radix > Long.MAX_VALUE - d)
		    throw new NumberFormatException();
		n = n * radix + d;
	    }
	    return n;
	}
    }

/* Conversion routines between names, strings, and byte arrays in Utf8 format
 */

    /** Convert `len' bytes from utf8 to characters.
     *  Parameters are as in System.arraycopy
     *  Return first index in `dst' past the last copied char.
     *  @param src        The array holding the bytes to convert.
     *  @param sindex     The start index from which bytes are converted.
     *  @param dst        The array holding the converted characters..
     *  @param sindex     The start index from which converted characters
     *                    are written.
     *  @param len        The maximum number of bytes to convert.
     */
    public static int utf2chars(byte[] src, int sindex,
				char[] dst, int dindex,
				int len) {
        int i = sindex;
        int j = dindex;
	int limit = sindex + len;
        while (i < limit) {
            int b = src[i++] & 0xFF;
            if (b >= 0xE0) {
                b = (b & 0x0F) << 12;
                b = b | (src[i++] & 0x3F) << 6;
                b = b | (src[i++] & 0x3F);
            } else if (b >= 0xC0) {
                b = (b & 0x1F) << 6;
                b = b | (src[i++] & 0x3F);
            }
            dst[j++] = (char)b;
        }
	return j;
    }

    /** Return bytes in Utf8 representation as an array of characters.
     *  @param src        The array holding the bytes.
     *  @param sindex     The start index from which bytes are converted.
     *  @param len        The maximum number of bytes to convert.
     */
    public static char[] utf2chars(byte[] src, int sindex, int len) {
	char[] dst = new char[len];
	int len1 = utf2chars(src, sindex, dst, 0, len);
	char[] result = new char[len1];
	System.arraycopy(dst, 0, result, 0, len1);
	return result;
    }

    /** Return all bytes of a given array in Utf8 representation
     *  as an array of characters.
     *  @param src        The array holding the bytes.
     */
    public static char[] utf2chars(byte[] src) {
	return utf2chars(src, 0, src.length);
    }

    /** Return bytes in Utf8 representation as a string.
     *  @param src        The array holding the bytes.
     *  @param sindex     The start index from which bytes are converted.
     *  @param len        The maximum number of bytes to convert.
     */
    public static String utf2string(byte[] src, int sindex, int len) {
	char dst[] = new char[len];
	int len1 = utf2chars(src, sindex, dst, 0, len);
	return new String(dst, 0, len1);
    }

    /** Return all bytes of a given array in Utf8 representation
     *  as a string.
     *  @param src        The array holding the bytes.
     */
    public static String utf2string(byte[] src) {
	return utf2string(src, 0, src.length);
    }

    /** Copy characters in source array to bytes in target array,
     *  converting them to Utf8 representation.
     *  The target array must be large enough to hold the result.
     *  returns first index in `dst' past the last copied byte.
     *  @param src        The array holding the characters to convert.
     *  @param sindex     The start index from which characters are converted.
     *  @param dst        The array holding the converted characters..
     *  @param sindex     The start index from which converted bytes
     *                    are written.
     *  @param len        The maximum number of characters to convert.
     */
    public static int chars2utf(char[] src, int sindex,
				byte[] dst, int dindex,
				int len) {
        int j = dindex;
	int limit = sindex + len;
        for (int i = sindex; i < limit; i++) {
            char ch = src[i];
	    if (1 <= ch && ch <= 0x7F) {
		dst[j++] = (byte)ch;
	    } else if (ch <= 0x7FF) {
		dst[j++] = (byte)(0xC0 | (ch >> 6));
		dst[j++] = (byte)(0x80 | (ch & 0x3F));
	    } else {
		dst[j++] = (byte)(0xE0 | (ch >> 12));
		dst[j++] = (byte)(0x80 | ((ch >> 6) & 0x3F));
		dst[j++] = (byte)(0x80 | (ch & 0x3F));
	    }
	}
	return j;
    }

    /** Return characters as an array of bytes in Utf8 representation.
     *  @param src        The array holding the characters.
     *  @param sindex     The start index from which characters are converted.
     *  @param len        The maximum number of characters to convert.
     */
    public static byte[] chars2utf(char[] src, int sindex, int len) {
	byte[] dst = new byte[len * 3];
	int len1 = chars2utf(src, sindex, dst, 0, len);
	byte[] result = new byte[len1];
	System.arraycopy(dst, 0, result, 0, len1);
	return result;
    }

    /** Return all characters in given array as an array of bytes
     *  in Utf8 representation.
     *  @param src        The array holding the characters.
     */
    public static byte[] chars2utf(char[] src) {
	return chars2utf(src, 0, src.length);
    }

    /** Return string as an array of bytes in in Utf8 representation.
     */
    public static byte[] string2utf(String s) {
	return chars2utf(s.toCharArray());
    }

    /** Quote all non-printing characters in string, but leave unicode
     *  characters alone.
     */
    public static String quote(String s) {
	StringBuffer buf = new StringBuffer();
	for (int i = 0; i < s.length(); i++) {
            char ch = s.charAt(i);
            switch (ch) {
            case '\n':
		buf.append("\\n");
                break;
            case '\t':
		buf.append("\\t");
                break;
            case '\b':
		buf.append("\\b");
                break;
            case '\f':
		buf.append("\\f");
                break;
            case '\r':
		buf.append("\\r");
                break;
            case '\"':
		buf.append("\\\"");
                break;
            case '\'':
		buf.append("\\\'");
                break;
            case '\\':
		buf.append("\\\\");
                break;
            default:
		if (ch < 32 || 128 <= ch && ch < 255) {
		    buf.append("\\");
		    buf.append((char)('0' + (ch >> 6) % 8));
		    buf.append((char)('0' + (ch >> 3) % 8));
		    buf.append((char)('0' + (ch     ) % 8));
		} else {
		    buf.append(ch);
		}
	    }
	}
	return buf.toString();
    }

    /** Escape all unicode characters in string.
     */
    public static String escapeUnicode(String s) {
	int len = s.length();
	int i = 0;
	while (i < len) {
	    char ch = s.charAt(i);
	    if (ch > 255) {
		StringBuffer buf = new StringBuffer();
		buf.append(s.substring(0, i));
		while (i < len) {
		    ch = s.charAt(i);
		    if (ch > 255) {
			buf.append("\\u");
			buf.append(Character.forDigit((ch >> 12) % 16, 16));
			buf.append(Character.forDigit((ch >>  8) % 16, 16));
			buf.append(Character.forDigit((ch >>  4) % 16, 16));
			buf.append(Character.forDigit((ch      ) % 16, 16));
		    } else {
			buf.append(ch);
		    }
		    i++;
		}
		s = buf.toString();
	    } else {
		i++;
	    }
	}
	return s;
    }

/* Conversion routines for qualified name splitting
 */
    /** Return the last part of a class name.
     */
    public static Name shortName(Name classname) {
        return classname.subName(
	    classname.lastIndexOf((byte)'.') + 1, classname.len);
    }

    /** Return the package name of a class name, excluding the trailing '.',
     *  "" if not existent.
     */
    public static Name packagePart(Name classname) {
        return classname.subName(0, classname.lastIndexOf((byte)'.'));
    }
}



