/*
 * @(#)Combine.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


// Append files to one another without duplicating any lines.

import java.io.BufferedReader;
import java.io.FileReader;
import java.util.HashMap;

public class Combine {

    private static HashMap map = new HashMap(10007);

    private static void appendFile(String fileName, boolean keep) {
	try {
	    BufferedReader br = new BufferedReader(new FileReader(fileName));

	    // Read a line at a time.  If the line does not appear in the
	    // hashmap, print it and add it to the hashmap, so that it will
	    // not be repeated.

	lineLoop:
	    while (true) {
		String line = br.readLine();
		if (line == null)
		    break;
		if (keep || !map.containsKey(line)) {
		    System.out.println(line);
		    map.put(line,line);
		}
	    }
	    br.close();
	} catch (Exception e) {
	    e.printStackTrace();
	    System.exit(1);
	}
    }


    public static void main(String[] args) {

	if (args.length < 2) {
	    System.err.println("Usage:  java Combine  file1  file2  ...");
	    System.exit(2);
	}

	for (int i = 0; i < args.length; ++i)
	    appendFile(args[i], i == 0);
    }
}
