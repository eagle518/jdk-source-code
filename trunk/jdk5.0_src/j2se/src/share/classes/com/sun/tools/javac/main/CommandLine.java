/**
 * @(#)CommandLine.java	1.13 04/01/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.main;

import java.io.IOException;
import java.io.Reader;
import java.io.FileReader;
import java.io.BufferedReader;
import java.io.StreamTokenizer;
import com.sun.tools.javac.util.ListBuffer;

/**
 * Various utility methods for processing Java tool command line arguments.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class CommandLine {
    /**
     * Process Win32-style command files for the specified command line
     * arguments and return the resulting arguments. A command file argument
     * is of the form '@file' where 'file' is the name of the file whose
     * contents are to be parsed for additional arguments. The contents of
     * the command file are parsed using StreamTokenizer and the original
     * '@file' argument replaced with the resulting tokens. Recursive command
     * files are not supported. The '@' character itself can be quoted with
     * the sequence '@@'.
     */
    public static String[] parse(String[] args)
	throws IOException
    {
	ListBuffer<String> newArgs = new ListBuffer<String>();
	for (int i = 0; i < args.length; i++) {
	    String arg = args[i];
	    if (arg.length() > 1 && arg.charAt(0) == '@') {
		arg = arg.substring(1);
		if (arg.charAt(0) == '@') {
		    newArgs.append(arg);
		} else {
		    loadCmdFile(arg, newArgs);
		}
	    } else {
		newArgs.append(arg);
	    }
	}
	return newArgs.toList().toArray(new String[newArgs.length()]);
    }

    private static void loadCmdFile(String name, ListBuffer<String> args)
	throws IOException
    {
	Reader r = new BufferedReader(new FileReader(name));
	StreamTokenizer st = new StreamTokenizer(r);
	st.resetSyntax();
	st.wordChars(' ', 255);
	st.whitespaceChars(0, ' ');
	st.commentChar('#');
	st.quoteChar('"');
	st.quoteChar('\'');
	while (st.nextToken() != st.TT_EOF) {
	    args.append(st.sval);
	}
	r.close();
    }
}
