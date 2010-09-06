/**
 * @(#)Main.java	1.21 04/04/15
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac;

import java.io.PrintWriter;
import java.lang.reflect.*;

/**
 * The main program for the command-line compiler javac.
 *
 * <p>Nothing described in this source file is part of any supported
 * API.  If you write code that depends on this, you do so at your own
 * risk.  This code and its internal interfaces are subject to change
 * or deletion without notice.
 */
public class Main {

    static {
	ClassLoader loader = Main.class.getClassLoader();
	if (loader != null)
	    loader.setPackageAssertionStatus("com.sun.tools.javac", true);
    }

    /** Command line interface.
     * @param args   The command line parameters.
     */
    public static void main(String[] args) throws Exception {
      if (args.length > 0 && args[0].equals("-Xjdb")) {
        String[] newargs = new String[args.length + 2];
        Object rcvr   = null;
        Class c = Class.forName("com.sun.tools.example.debug.tty.TTY");
        Method method = c.getDeclaredMethod ("main", new Class[] {args.getClass()});
        method.setAccessible(true);
        System.arraycopy(args, 1, newargs, 3, args.length - 1);
        newargs[0] = "-connect";
        newargs[1] = "com.sun.jdi.CommandLineLaunch:options=-esa -ea:com.sun.tools...";
        newargs[2] = "com.sun.tools.javac.Main";
        method.invoke(rcvr, new Object[] { newargs });
      } else {
        System.exit(compile(args));
      }
    }

    /** Programmatic interface.
     * @param args   The command line parameters.
     */
    public static int compile(String[] args) {
	com.sun.tools.javac.main.Main compiler =
	    new com.sun.tools.javac.main.Main("javac");
	return compiler.compile(args);
    }

    /** Programmatic interface.
     * @param args   The command line parameters.
     * @param out    Where the compiler's output is directed.
     */
    public static int compile(String[] args, PrintWriter out) {
	com.sun.tools.javac.main.Main compiler =
	    new com.sun.tools.javac.main.Main("javac", out);
	return compiler.compile(args);
    }
}
