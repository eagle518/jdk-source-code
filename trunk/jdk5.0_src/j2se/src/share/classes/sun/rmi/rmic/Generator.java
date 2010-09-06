/*
 * @(#)Generator.java	1.17 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Licensed Materials - Property of IBM
 * RMI-IIOP v1.0
 * Copyright IBM Corp. 1998 1999  All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

package sun.rmi.rmic;

import java.io.File;
import sun.tools.java.ClassDefinition;

/**
 * Generator defines the protocol for back-end implementations to be added
 * to rmic.  See the rmic.properties file for a description of the format for
 * adding new Generators to rmic.
 * <p>
 * Classes implementing this interface must have a public default constructor
 * which should set any required arguments to their defaults.  When Main
 * encounters a command line argument which maps to a specific Generator
 * subclass, it will instantiate one and call parseArgs(...).  At some later
 * point, Main will invoke the generate(...) method once for _each_ class passed
 * on the command line.
 *
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 *
 * @version	1.0, 2/23/98
 * @author	Bryan Atsatt
 */
public interface Generator {

    /**
     * Examine and consume command line arguments.
     * @param argv The command line arguments. Ignore null
     * and unknown arguments. Set each consumed argument to null.
     * @param main Report any errors using the main.error() methods.
     * @return true if no errors, false otherwise.
     */
    public boolean parseArgs(String argv[], Main main);
    
    /**
     * Generate output. Any source files created which need compilation should
     * be added to the compiler environment using the addGeneratedFile(File)
     * method.
     *
     * @param env	The compiler environment
     * @param cdef	The definition for the implementation class or interface from
     *              which to generate output
     * @param destDir	The directory for the root of the package hierarchy
     *			        for generated files. May be null.
     */
    public void generate(BatchEnvironment env, ClassDefinition cdef, File destDir);
}
