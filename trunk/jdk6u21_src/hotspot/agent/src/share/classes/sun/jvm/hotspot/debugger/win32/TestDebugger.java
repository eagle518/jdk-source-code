/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.debugger.win32;

import java.util.*;
import sun.jvm.hotspot.debugger.*;

public class TestDebugger {
  private static void usage() {
    System.out.println("usage: java TestDebugger [pid]");
    System.exit(1);
  }

  public static void main(String[] args) {
    try {
      if (args.length != 1) {
        usage();
      }

      int pid = 0;
      try {
        pid = Integer.parseInt(args[0]);
      }
      catch (NumberFormatException e) {
        usage();
      }

      JVMDebugger debugger = new Win32DebuggerLocal(new MachineDescriptionIntelX86(), true);
      System.err.println("Process list: ");
      List processes = debugger.getProcessList();
      for (Iterator iter = processes.iterator(); iter.hasNext(); ) {
        ProcessInfo info = (ProcessInfo) iter.next();
        System.err.println(info.getPid() + " " + info.getName());
      }
      System.err.println("Trying to attach...");
      debugger.attach(pid);
      System.err.println("Attach succeeded.");
      System.err.println("Trying to detach...");
      if (!debugger.detach()) {
        System.err.println("ERROR: detach failed.");
        System.exit(0);
      }
      System.err.println("Detach succeeded.");
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
}
