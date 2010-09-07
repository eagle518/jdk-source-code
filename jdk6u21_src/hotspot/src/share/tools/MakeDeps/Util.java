/*
 * Copyright (c) 2005, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

import java.util.*;
import java.io.File;

public class Util {
    static String join(String padder, Vector v) {
        return join(padder, v, false);
    }

    static String join(String padder, Vector v, boolean quoted) {
        StringBuffer sb = new StringBuffer();

        for (Iterator iter = v.iterator(); iter.hasNext(); ) {
            if (quoted) {
                sb.append('"');
            }
            sb.append((String)iter.next());
            if (quoted) {
                sb.append('"');
            }
            if (iter.hasNext()) sb.append(padder);
        }

        return sb.toString();
    }

     static String join(String padder, String v[]) {
        StringBuffer sb = new StringBuffer();

        for (int i=0; i<v.length; i++) {
            sb.append(v[i]);
            if (i < (v.length  - 1)) sb.append(padder);
        }

        return sb.toString();
    }



    static String prefixed_join(String padder, Vector v, boolean quoted) {
        StringBuffer sb = new StringBuffer();

        for (Iterator iter = v.iterator(); iter.hasNext(); ) {
            sb.append(padder);

            if (quoted) {
                sb.append('"');
            }
            sb.append((String)iter.next());
            if (quoted) {
                sb.append('"');
            }
        }

        return sb.toString();
    }


    static String normalize(String file) {
        return file.replace('\\', '/');
    }

    static String sep = File.separator;
    static String os = "Win32"; //System.getProperty("os.name");
}
