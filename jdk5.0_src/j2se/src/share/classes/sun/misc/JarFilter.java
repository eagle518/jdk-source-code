/*
 * @(#)JarFilter.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;

import java.io.File;
import java.io.FilenameFilter;

/**
 * <p>
 * This class checks that only jar and zip files are included in the file list.
 * This class is used in extension installation support (ExtensionDependency).
 * <p>
 *
 * @author  Michael Colburn
 * @version 1.0, 10/15/01
 */ 
public class JarFilter implements FilenameFilter {

    public boolean accept(File dir, String name) {
	String lower = name.toLowerCase();
	return lower.endsWith(".jar") || lower.endsWith(".zip");
    }
}
