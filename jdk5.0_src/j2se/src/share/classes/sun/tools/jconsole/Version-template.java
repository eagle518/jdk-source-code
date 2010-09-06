/*
 * @(#)Version-template.java	1.2 04/05/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.io.PrintStream;

public class Version {
    private static final String jconsole_version =
        "@@jconsole_version@@";

    public static void print(PrintStream ps) {
        printFullVersion(ps);

        ps.println(Resources.getText("Name and Build",
                                     System.getProperty("java.runtime.name"), 
                                     System.getProperty("java.runtime.version")));

        ps.println(Resources.getText("Name Build and Mode",
                                     System.getProperty("java.vm.name"),
                                     System.getProperty("java.vm.version"),
                                     System.getProperty("java.vm.info"))); 

    }

    public static void printFullVersion(PrintStream ps) {
        ps.println(Resources.getText("JConsole version", jconsole_version));
    }
}
