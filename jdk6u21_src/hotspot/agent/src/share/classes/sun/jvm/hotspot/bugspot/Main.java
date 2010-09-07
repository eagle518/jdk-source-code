/*
 * Copyright (c) 2001, 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package sun.jvm.hotspot.bugspot;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import sun.jvm.hotspot.ui.*;

/** The main class for the BugSpot debugger. */

public class Main {
  public static void main(String[] args) {
    JFrame frame = new JFrame("BugSpot");
    frame.setSize(800, 600);
    BugSpot db = new BugSpot();
    db.setMDIMode(true);
    db.build();
    frame.setJMenuBar(db.getMenuBar());
    frame.getContentPane().add(db);
    frame.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);

    GraphicsUtilities.reshapeToAspectRatio(frame,
                                           4.0f/3.0f, 0.85f, Toolkit.getDefaultToolkit().getScreenSize());
    GraphicsUtilities.centerInContainer(frame,
                                        Toolkit.getDefaultToolkit().getScreenSize());
    frame.setVisible(true);
  }
}
