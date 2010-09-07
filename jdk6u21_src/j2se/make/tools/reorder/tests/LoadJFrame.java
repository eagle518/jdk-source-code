/*
 * @(#)LoadJFrame.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


import javax.swing.JFrame;

public class LoadJFrame {

    public static void main(String[] args) {
	new JFrame().show();
	// This starts a thread which never exits - so we suicide.
	try {
	    Thread.sleep(10000);
	} catch (Exception e) {
	}
	System.exit(0);
    }
}
