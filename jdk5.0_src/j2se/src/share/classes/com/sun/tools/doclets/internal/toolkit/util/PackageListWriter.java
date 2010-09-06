/*
 * @(#)PackageListWriter.java	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.internal.toolkit.util;

import com.sun.tools.doclets.internal.toolkit.*;
import com.sun.javadoc.*;
import java.io.*;
import java.util.*;

/**
 * Write out the package index.
 *
 * This code is not part of an API.
 * It is implementation that is subject to change.
 * Do not use it as an API
 * 
 * @see com.sun.javadoc.PackageDoc
 * @author Atul M Dambalkar
 */
public class PackageListWriter extends PrintWriter {

    private Configuration configuration;
    
    /**
     * Constructor.
     *
     * @param configuration the current configuration of the doclet.
     */
    public PackageListWriter(Configuration configuration) throws IOException {
        super(Util.genWriter(configuration, configuration.destDirName, 
            DocletConstants.PACKAGE_LIST_FILE_NAME, configuration.docencoding));
        this.configuration = configuration;
    }

    /**
     * Generate the package index.
     *
     * @param configuration the current configuration of the doclet.
     * @throws DocletAbortException
     */
    public static void generate(Configuration configuration) {
        PackageListWriter packgen;
        try {
            packgen = new PackageListWriter(configuration);
            packgen.generatePackageListFile(configuration.root);
            packgen.close();
        } catch (IOException exc) {
            configuration.message.error("doclet.exception_encountered",
                exc.toString(), DocletConstants.PACKAGE_LIST_FILE_NAME);
            throw new DocletAbortException();
        }
    }

    protected void generatePackageListFile(RootDoc root) {
        PackageDoc[] packages = configuration.packages;
        String[] names = new String[packages.length];
        for (int i = 0; i < packages.length; i++) {
            names[i] = packages[i].name();
        }
        Arrays.sort(names);
        for (int i = 0; i < packages.length; i++) {
            println(names[i]);
        }
    }
}
