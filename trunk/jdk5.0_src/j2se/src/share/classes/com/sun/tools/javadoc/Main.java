/**
 * @(#)Main.java	1.11 04/01/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import java.io.PrintWriter;

/**
 * Provides external entry points (tool and programmatic)
 * for the javadoc program.
 *
 * @since JDK1.4
 */
public class Main {
    
    /**
     * Constructor should never be called.
     */
    private Main() {
    }

    /**
     * Command line interface.
     * @param args   The command line parameters.
     */
    public static void main(String[] args) {
	System.exit(execute(args));
    }

    /**
     * Programmatic interface.
     * @param args   The command line parameters.
     * @return The return code.
     */
    public static int execute(String[] args) {
	Start jdoc = new Start();
	return jdoc.begin(args);
    }

    /**
     * Programmatic interface.
     * @param programName  Name of the program (for error messages).
     * @param args   The command line parameters.
     * @return The return code.
     */
    public static int execute(String programName, String[] args) {
	Start jdoc = new Start(programName);
	return jdoc.begin(args);
    }

    /**
     * Programmatic interface.
     * @param programName  Name of the program (for error messages).
     * @param defaultDocletClassName  Fully qualified class name.
     * @param args   The command line parameters.
     * @return The return code.
     */
    public static int execute(String programName, 
                              String defaultDocletClassName,
                              String[] args) {
	Start jdoc = new Start(programName, defaultDocletClassName);
	return jdoc.begin(args);
    }

    /**
     * Programmatic interface.
     * @param programName  Name of the program (for error messages).
     * @param errWriter    PrintWriter to receive error messages.
     * @param warnWriter    PrintWriter to receive error messages.
     * @param noticeWriter    PrintWriter to receive error messages.
     * @param defaultDocletClassName  Fully qualified class name.
     * @param args   The command line parameters.
     * @return The return code.
     */
    public static int execute(String programName, 
                              PrintWriter errWriter, 
                              PrintWriter warnWriter, 
                              PrintWriter noticeWriter,
                              String defaultDocletClassName,
                              String[] args) {
	Start jdoc = new Start(programName, 
                               errWriter, warnWriter, noticeWriter,
                               defaultDocletClassName);
	return jdoc.begin(args);
    }
}
