/**
 * @(#)Main.java	1.2 04/07/27
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.apt;

import java.io.PrintWriter;

/**
 * The main program for the command-line compiler apt.
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
	    loader.setPackageAssertionStatus("com.sun.tools.apt", true);
    }

    /** Command line interface.
     * @param args   The command line parameters.
     */
    public static void main(String[] args) {
	System.exit(compile(args));
    }

    /** Programmatic interface.
     * @param args   The command line parameters.
     */
    public static int compile(String[] args) {
	com.sun.tools.apt.main.Main compiler =
	    new com.sun.tools.apt.main.Main("apt");
	return compiler.compile(args);
    }

    /** Programmatic interface.
     * @param args   The command line parameters.
     * @param out    Where the compiler's output is directed.
     */
    public static int compile(String[] args, PrintWriter out) {
	com.sun.tools.apt.main.Main compiler =
	    new com.sun.tools.apt.main.Main("apt", out);
	return compiler.compile(args);
    }
}
