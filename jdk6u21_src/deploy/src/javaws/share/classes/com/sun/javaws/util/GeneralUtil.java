/*
 * @(#)GeneralUtil.java	1.13 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package com.sun.javaws.util;

import java.util.Locale;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.awt.Frame;
import java.awt.*;

/** Handy class to add some utility methods for dealing
 *  with property matching etc.
 */
public class GeneralUtil {
  
    
    public static boolean prefixMatchStringList(String[] prefixList, String target) {
        // No prefixes matches everything
        if (prefixList == null) return true;
	// No target, but a prefix list does not match anything
	if (target == null) return false;
        for(int i = 0; i < prefixList.length; i++) {
            if (target.startsWith(prefixList[i])) return true;
        }
        return false;
    }
    
    /** Converts a space delimitered string to a list of strings */
    static public String[] getStringList(String str) {
        if (str == null) return null;
        ArrayList list = new ArrayList();
        int i = 0;
        int length = str.length();
        StringBuffer sb = null;
        while(i < length) {
            char ch = str.charAt(i);
            if (ch == ' ') {
                // A space was hit. Add string to list
                if (sb != null) {
                    list.add(sb.toString());
                    sb = null;
                }
            } else if (ch == '\\') {
                // It is a delimiter. Add next character
                if (i + 1 < length) {
                    ch = str.charAt(++i);
                    if (sb == null) sb = new StringBuffer();
                    sb.append(ch);
                }
            } else {
                if (sb == null) sb = new StringBuffer();
                sb.append(ch);
            }
            i++; // Next character
        }
        // Make sure to add the last part to the list too
        if (sb != null) {
            list.add(sb.toString());
        }	
	if (list.size() == 0) return null;        
        String[] results = new String[list.size()];
        return (String[])list.toArray(results);
    }
                    
    /** Checks if string list matches default locale */
    static public boolean matchLocale(String[] localeList, Locale locale) {
        // No locale specified matches everything
        if (localeList == null) return true;
        for(int i = 0; i < localeList.length; i++) {
            if (matchLocale(localeList[i], locale)) return true;
        }
        return false;
    }
    
    /** Checks if string matches default locale */
    static public boolean matchLocale(String localeStr, Locale locale) {
        if (localeStr == null || localeStr.length() == 0) return true;        	
	
        // Compare against default locale
        String language = "";
        String country  = "";
        String variant = "";
		
        // The locale string is of the form language_country_variant
        StringTokenizer st = new StringTokenizer(localeStr, "_", false);
        if (st.hasMoreElements() && locale.getLanguage().length() > 0) {
            language = st.nextToken();
            if (!language.equalsIgnoreCase(locale.getLanguage())) return false;
        }
        if (st.hasMoreElements() && locale.getCountry().length() > 0) {
            country = st.nextToken();
            if (!country.equalsIgnoreCase(locale.getCountry())) return false;
        }
        if (st.hasMoreElements() && locale.getVariant().length() > 0) {
            variant = st.nextToken();
            if (!variant.equalsIgnoreCase(locale.getVariant())) return false;
        }
        
        return true;
    }
    
    static public long heapValToLong(String heapValue) {
	if (heapValue == null) return -1;
	long multiplier = 1;
	if (heapValue.toLowerCase().lastIndexOf('m') != -1) {
	    // units are megabytes, 1 megabyte = 1024 * 1024 bytes
	    multiplier = 1024 * 1024;
	    heapValue = heapValue.substring(0, heapValue.length() - 1);
	} else if (heapValue.toLowerCase().lastIndexOf('k') != -1) {
	    // units are kilobytes, 1 kilobyte = 1024 bytes
	    multiplier = 1024;
	    heapValue = heapValue.substring(0, heapValue.length() - 1);
	}
	long theValue = -1;
	try {
	    theValue = Long.parseLong(heapValue);
	    theValue = theValue * multiplier;
	} catch (NumberFormatException e) {
	    theValue = -1;
	}
	return theValue;
    }
    
    public static Frame getActiveTopLevelFrame() {
	Frame [] frames = Frame.getFrames();
	int index = -1;
	if (frames == null) return null;
	for (int i = 0; i < frames.length; i++) {
	    if (frames[i].getFocusOwner() != null) {
		index = i;
	    }
	}
	return (index >= 0 ? frames[index] : null);
    }
}

