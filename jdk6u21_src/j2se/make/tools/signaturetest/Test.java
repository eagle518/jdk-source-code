/*
 * @(#)Test.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.javatest;

import java.io.PrintWriter;

/**
 * This interface is implemented by tests to be run under the test harness
 * of javasoft.sqe.javatest. Information about the test is normally contained
 * in the parameters of a table in an HTML file that is read by the
 * test harness.
 *
 * A test should also define `main' as follows:
 * <pre>
 * <code>
 * 	public static void main(String[] args) {
 * 	    Test t = new <em>test-class-name</em>();
 * 	    Status s = t.run(args, new PrintWriter(System.err), new PrintWriter(System.out));
 * 	    s.exit();
 * 	}
 * </code>
 * </pre>
 * Defining `main' like this means that the test can also be run standalone, 
 * independent of the harness.
 *
 * @author Jonathan J Gibbons
 */
public interface Test
{
    /**
     * Runs the test embodied by the implementation.
     * @param args 	These are supplied from the `executeArgs'
     *		   	values in the corresponding test description
     *             	and permit an implementation to be used for a variety of tests.
     * @param log  	A stream to which to report messages and errors.
     * @param ref  	A stream to which to write reference output.
     *			The file may subsequently be used to determine if the test 
     *			succeeded by comparing the contents against a golden file.
     */
    public Status run(String[] args, PrintWriter log, PrintWriter ref);

}
