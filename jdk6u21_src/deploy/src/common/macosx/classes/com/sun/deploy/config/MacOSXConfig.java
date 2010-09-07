/*
 * @(#)MacOSXConfig.java	1.3 10/03/24
 *
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.config;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.util.Iterator;
import java.util.HashSet;
import java.util.Set;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/** This class principally overrides the mechanism for finding the
    available JREs on the system, knowing the Mac OS X-specific
    directory layout of the JavaVM.framework. */

public class MacOSXConfig extends UnixConfig {

    private static Pattern versionPattern;

    private boolean isValidVersion(String name) {
        if (versionPattern == null) {
            versionPattern = Pattern.compile("\\d\\.\\d\\.\\d.*");
        }
        Matcher matcher = versionPattern.matcher(name);
        return matcher.matches();
    }

    public String getLibrarySufix() {
        return ".jnilib";
    }

    public Vector getInstalledJREList() {
        try {
            File allVersions = new File("/System/Library/Frameworks/JavaVM.framework/Versions");
            if (allVersions.exists()) {
                // Enumerate the valid versions in this directory
                File[] allValidVersions = allVersions.listFiles(new FileFilter() {
                        public boolean accept(File pathname) {
                            return (pathname.isDirectory() &&
                                    isValidVersion(pathname.getName()));
                        }
                    });
                if (allValidVersions != null) {
                    // Canonicalize these to get rid of symlinks
                    // This is probably not strictly necessary, but if we
                    // re-use this logic in the Control Panel, we'll want it
                    Set validVersions = new HashSet();
                    for (int i = 0; i < allValidVersions.length; i++) {
                        validVersions.add(allValidVersions[i].getCanonicalFile());
                    }
                    // Now create the resulting Vector
                    Vector res = new Vector();
                    for (Iterator iter = validVersions.iterator(); iter.hasNext(); ) {
                        File cur = (File) iter.next();
                        res.add(cur.getName());                           // Version
                        res.add(new File(cur, "Home").getAbsolutePath()); // Path to java.home
                    }
                    return res;
                }
            }
        } catch (IOException e) {
        }
        return null;
    }
}
