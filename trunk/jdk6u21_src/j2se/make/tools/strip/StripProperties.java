/*
 * @(#)StripProperties.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

// Compile with "javac StripProperties.java" using JDK 1.1 or higher.

import java.io.BufferedWriter;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Properties;

/**
 * Reads a properties file from standard input and writes an equivalent
 * properties file without comments to standard output.
 */
public class StripProperties {

    public static void main(String[] args) throws IOException {
        InputStream in = System.in;
        OutputStream out = System.out;
        Properties prop = new Properties();
        prop.load(in);
        storeProperties(prop, out);
    }
    
    // --- code below here is adapted from java.util.Properties ---
    
    private static final String specialSaveChars = "=: \t\r\n\f#!";

    /*
     * Converts unicodes to encoded &#92;uxxxx
     * and writes out any of the characters in specialSaveChars
     * with a preceding slash
     */
    private static String saveConvert(String theString, boolean escapeSpace) {
        int len = theString.length();
        StringBuffer outBuffer = new StringBuffer(len*2);

        for(int x=0; x<len; x++) {
            char aChar = theString.charAt(x);
            switch(aChar) {
		case ' ':
		    if (x == 0 || escapeSpace) 
			outBuffer.append('\\');

		    outBuffer.append(' ');
		    break;
                case '\\':outBuffer.append('\\'); outBuffer.append('\\');
                          break;
                case '\t':outBuffer.append('\\'); outBuffer.append('t');
                          break;
                case '\n':outBuffer.append('\\'); outBuffer.append('n');
                          break;
                case '\r':outBuffer.append('\\'); outBuffer.append('r');
                          break;
                case '\f':outBuffer.append('\\'); outBuffer.append('f');
                          break;
                default:
                    if ((aChar < 0x0020) || (aChar == 0x007e) || (aChar > 0x00ff)) {
                        outBuffer.append('\\');
                        outBuffer.append('u');
                        outBuffer.append(toHex((aChar >> 12) & 0xF));
                        outBuffer.append(toHex((aChar >>  8) & 0xF));
                        outBuffer.append(toHex((aChar >>  4) & 0xF));
                        outBuffer.append(toHex( aChar        & 0xF));
                    } else {
                        if (specialSaveChars.indexOf(aChar) != -1)
                            outBuffer.append('\\');
                        outBuffer.append(aChar);
                    }
            }
        }
        return outBuffer.toString();
    }

    /**
     * Writes the content of <code>properties</code> to <code>out</code>.
     * The format is that of Properties.store with the following modifications:
     * <ul>
     * <li>No header or date is written
     * <li>Latin-1 characters are written as single bytes, not escape sequences
     * <li>Line breaks are indicated by a single \n independent of platform
     * <ul>
     */
    private static void storeProperties(Properties properties, OutputStream out)
            throws IOException {
        BufferedWriter awriter;
        awriter = new BufferedWriter(new OutputStreamWriter(out, "8859_1"));
        for (Enumeration e = properties.keys(); e.hasMoreElements();) {
            String key = (String)e.nextElement();
            String val = (String)properties.get(key);
            key = saveConvert(key, true);

	    /* No need to escape embedded and trailing spaces for value, hence
	     * pass false to flag.
	     */
            val = saveConvert(val, false);
            writeln(awriter, key + "=" + val);
        }
        awriter.flush();
    }

    private static void writeln(BufferedWriter bw, String s) throws IOException {
        bw.write(s);
        bw.write("\n");
    }

    /**
     * Convert a nibble to a hex character
     * @param	nibble	the nibble to convert.
     */
    private static char toHex(int nibble) {
	return hexDigit[(nibble & 0xF)];
    }

    /** A table of hex digits */
    private static final char[] hexDigit = {
	'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'
    };
}

