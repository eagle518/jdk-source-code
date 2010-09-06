/*
 * @(#)SourceMapper.java	1.18 03/12/19
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

package com.sun.tools.example.debug.tty;

import com.sun.jdi.Location;
import com.sun.jdi.AbsentInformationException;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.StringTokenizer;
import java.io.*;

class SourceMapper {

    private final String[] dirs;

    SourceMapper(List sourcepath) {
        /*
         * sourcepath can arrive from the debugee as a List.
         * (via PathSearchingVirtualMachine.classPath())
         */
        List dirList = new ArrayList();
        Iterator iter = sourcepath.iterator();
        while (iter.hasNext()) {
            String element = (String)iter.next();
            //XXX remove .jar and .zip files; we want only directories on
            //the source path. (Bug ID 4186582)
            if ( ! (element.endsWith(".jar") ||
                    element.endsWith(".zip"))) {
                dirList.add(element);
            }
        }
        dirs = (String[])dirList.toArray(new String[0]);
    }

    SourceMapper(String sourcepath) {
        /*
         * sourcepath can also arrive from the command line
         * as a String.  (via "-sourcepath")
         * 
         * Using File.pathSeparator as delimiter below is OK
         * because we are on the same machine as the command
         * line originiated.
         */
        StringTokenizer st = new StringTokenizer(sourcepath,
                                                 File.pathSeparator);
        List dirList = new ArrayList();
        while (st.hasMoreTokens()) {
            String s = st.nextToken();
            //XXX remove .jar and .zip files; we want only directories on
            //the source path. (Bug ID 4186582)
            if ( ! (s.endsWith(".jar") ||
                    s.endsWith(".zip"))) {
                dirList.add(s);
            }
        }
        dirs = (String[])dirList.toArray(new String[0]);
    }
    
    /*
     * Return the current sourcePath as a String.
     */
    String getSourcePath() {
        int i = 0;
        StringBuffer sp;
        if (dirs.length < 1) {
            return "";          //The source path is empty.
        } else {
            sp = new StringBuffer(dirs[i++]);
        }
        for (; i < dirs.length; i++) {
            sp.append(File.pathSeparator);
            sp.append(dirs[i]);
        }
        return sp.toString();
    }

    /**
     * Return a File cooresponding to the source of this location.
     * Return null if not available.
     */
    File sourceFile(Location loc) {
        try {
            String filename = loc.sourceName();
            String refName = loc.declaringType().name();
            int iDot = refName.lastIndexOf('.');
            String pkgName = (iDot >= 0)? refName.substring(0, iDot+1) : "";
            String full = pkgName.replace('.', File.separatorChar) + filename;
            for (int i= 0; i < dirs.length; ++i) {
                File path = new File(dirs[i], full);
                if (path.exists()) {
                    return path;
                }
            }
            return null;
        } catch (AbsentInformationException e) {
            return null;
        }
    }

    /**
     * Return a BufferedReader cooresponding to the source 
     * of this location.
     * Return null if not available.
     * Note: returned reader must be closed.
     */
    BufferedReader sourceReader(Location loc) {
        File sourceFile = sourceFile(loc);
        if (sourceFile == null) {
            return null;
        }
        try {
            return new BufferedReader(new FileReader(sourceFile));
        } catch(IOException exc) {
        }
        return null;
    }
}
        
