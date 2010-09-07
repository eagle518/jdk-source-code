/*
 * @(#)CompileFontConfig.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import sun.awt.FontConfiguration;

public class CompileFontConfig {
    public static void main(String[] argv) {
        boolean verbose = false;
        if (argv.length != 0 && "-verbose".equals(argv[0])) {
            verbose = true;
	}
        if (verbose) {
            if (argv.length != 3) 
		System.out.println("Usage: java CompileFontConfig [-verbose] propertiesFileName binaryFileName");
            else 
                new CompileFontConfig(argv[1], argv[2], true);
        } else {
            if (argv.length != 2) 
		System.out.println("Usage: java CompileFontConfig [-verbose] propertiesFileName binaryFileNme");
	    else 
                new CompileFontConfig(argv[0], argv[1], false);
	}
    }

    CompileFontConfig(String inFile, String outFile, boolean verbose) {
        try {
            FileInputStream in = new FileInputStream(inFile);
            FileOutputStream out = new FileOutputStream(outFile);
            FontConfiguration.verbose = verbose;
            FontConfiguration.loadProperties(in);
            FontConfiguration.saveBinary(out);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
