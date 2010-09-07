/*
 * @(#)JHello.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


import java.awt.*;
import java.io.PrintStream;
import javax.swing.*;

public class JHello extends JFrame {

    JHello() {
        JLabel jlabel = new JLabel("Hello");
        jlabel.setFont(new Font("Monospaced", 0, 144));
        getContentPane().add(jlabel);
        pack();
    }

    public static void main(String args[]) {
        new JHello().show();
	try {
	    Thread.sleep(10000);
	} catch (Exception e) {
	}
	System.exit(0);
    }
}
