/*
 * @(#)Utils.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

package com.sun.tools.example.debug.bdi;   //### does it belong here?

import com.sun.jdi.*;
import com.sun.tools.jdi.*;
import java.util.*;
import java.io.*;

public class Utils {

    /**
     * Return the thread status description.
     */
    public static String getStatus(ThreadReference thr) {
	int status = thr.status();
	String result;
	switch (status) {
	  case ThreadReference.THREAD_STATUS_UNKNOWN:
	    result = "unknown status";
	    break;
	  case ThreadReference.THREAD_STATUS_ZOMBIE:
	    result = "zombie";
	    break;
	  case ThreadReference.THREAD_STATUS_RUNNING:
	    result = "running";
	    break;
	  case ThreadReference.THREAD_STATUS_SLEEPING:
	    result = "sleeping";
	    break;
	  case ThreadReference.THREAD_STATUS_MONITOR:
	    result = "waiting to acquire a monitor lock";
	    break;
	  case ThreadReference.THREAD_STATUS_WAIT:
	    result = "waiting on a condition";
	    break;
	  default:
	    result = "<invalid thread status>";
	}
	if (thr.isSuspended()) {
	    result += " (suspended)";
	}
	return result;
    }

    /** 
     * Return a description of an object.
     */
    public static String description(ObjectReference ref) {
        ReferenceType clazz = ref.referenceType();
        long id = ref.uniqueID();  //### TODO use real id
        if (clazz == null) {
            return toHex(id);
        } else {
            return "(" + clazz.name() + ")" + toHex(id);
        }
    }

    /**
     * Convert a long to a hexadecimal string.
     */
    public static String toHex(long n) {
	char s1[] = new char[16];
	char s2[] = new char[18];

	// Store digits in reverse order.
	int i = 0;
	do {
	    long d = n & 0xf;
	    s1[i++] = (char)((d < 10) ? ('0' + d) : ('a' + d - 10));
	} while ((n >>>= 4) > 0);

	// Now reverse the array.
	s2[0] = '0';
	s2[1] = 'x';
	int j = 2;
	while (--i >= 0) {
	    s2[j++] = s1[i];
	}
	return new String(s2, 0, j);
    }

    /**
     * Convert hexadecimal strings to longs.
     */
    public static long fromHex(String hexStr) {
	String str = hexStr.startsWith("0x") ?
	    hexStr.substring(2).toLowerCase() : hexStr.toLowerCase();
	if (hexStr.length() == 0) {
	    throw new NumberFormatException();
	}
	
	long ret = 0;
	for (int i = 0; i < str.length(); i++) {
	    int c = str.charAt(i);
	    if (c >= '0' && c <= '9') {
		ret = (ret * 16) + (c - '0');
	    } else if (c >= 'a' && c <= 'f') {
		ret = (ret * 16) + (c - 'a' + 10);
	    } else {
		throw new NumberFormatException();
	    }
	}
	return ret;
    }


    /*
     * The next two methods are used by this class and by EventHandler
     * to print consistent locations and error messages.
     */
    public static String locationString(Location loc) {
	return  loc.declaringType().name() + 
	    "." + loc.method().name() + "(), line=" + 
	    loc.lineNumber();
    }

//### UNUSED.
/************************
    private String typedName(Method method) {
        // TO DO: Use method.signature() instead of method.arguments() so that
	// we get sensible results for classes without debugging info
        StringBuffer buf = new StringBuffer();
        buf.append(method.name());
        buf.append("(");
        Iterator it = method.arguments().iterator();
        while (it.hasNext()) {
            buf.append(((LocalVariable)it.next()).typeName());
            if (it.hasNext()) {
                buf.append(",");
            }
        }
        buf.append(")");
        return buf.toString();
    }
************************/

    public static boolean isValidMethodName(String s) {
        return isJavaIdentifier(s) || 
               s.equals("<init>") ||
               s.equals("<clinit>");
    }

    public static boolean isJavaIdentifier(String s) {
        if (s.length() == 0) {                              
            return false;
        }
        int cp = s.codePointAt(0);
        if (! Character.isJavaIdentifierStart(cp)) {
            return false;
        }
        for (int i = Character.charCount(cp); i < s.length(); i += Character.charCount(cp)) {
            cp = s.codePointAt(i);
            if (! Character.isJavaIdentifierPart(cp)) {
                return false;
            }
        }
        return true;
    }

}

