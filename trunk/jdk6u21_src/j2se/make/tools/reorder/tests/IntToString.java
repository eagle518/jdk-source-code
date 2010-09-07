/*
 * @(#)IntToString.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */



public class IntToString {

    public static void main(String[] args) {
	String n = "10000";
	System.err.println("Hello");

	if (args.length == 0) {
	    try {
		Thread.currentThread().sleep(Integer.parseInt(n));
	    } catch (Exception e) {
		e.printStackTrace();
	    }
	    System.exit(0);
	}
    }
}
