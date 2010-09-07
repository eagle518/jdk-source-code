/*
 * @(#)LoadToolkit.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


import java.awt.Toolkit;

public class LoadToolkit {

    public static void main(String[] args) {
	Toolkit kit = Toolkit.getDefaultToolkit();
	// This starts a thread which never exits - so we suicide.
	try {
	    Thread.sleep(5000);
	} catch (Exception e) {
	}
	System.exit(0);
    }
}
