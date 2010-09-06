/*
 * @(#)Version-template.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.misc;
import java.io.PrintStream;

public class Version {

    private static final String java_version =
	"@@java_version@@";
	
    private static final String java_runtime_name =
	"Java(TM) 2 Runtime Environment, Standard Edition";

    private static final String java_runtime_version =
	"@@java_runtime_version@@";

    static {
	init();
    }

    public static void init() {
	System.setProperty("java.version", java_version);
	System.setProperty("java.runtime.version", java_runtime_version);
	System.setProperty("java.runtime.name", java_runtime_name);
    }

    /**
     * In case you were wondering this method is called by java -version.
     * Sad that it prints to stderr; would be nicer if default printed on
     * stdout.
     */
    public static void print() {
	print(System.err);
    }

    /**
     * Give a stream, it will print version info on it.
     */
    public static void print(PrintStream ps) {
	/* First line: platform version. */
	ps.println("java version \"" + java_version + "\"");

	/* Second line: runtime version (ie, libraries). */
	ps.println(java_runtime_name + " (build " +
			   java_runtime_version + ")");

	/* Third line: JVM information. */
	String java_vm_name    = (String)System.getProperty("java.vm.name");
	String java_vm_version = (String)System.getProperty("java.vm.version");
	String java_vm_info    = (String)System.getProperty("java.vm.info");
	ps.println(java_vm_name + " (build " + java_vm_version + ", " +
		   java_vm_info + ")");
    }

}

// Help Emacs a little because this file doesn't end in .java.
//
// Local Variables: ***
// mode: java ***
// End: ***
