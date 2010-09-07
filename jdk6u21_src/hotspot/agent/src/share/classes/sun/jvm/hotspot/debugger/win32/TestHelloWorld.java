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

/** Tests to see whether we can find the "Hello, World" string in a
    target process */

public class TestHelloWorld {
  private static void usage() {
    System.out.println("usage: java TestHelloWorld [pid]");
    System.out.println("pid must be the process ID of the HelloWorldDLL programs");
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
      System.err.println("Trying to attach...");
      debugger.attach(pid);
      System.err.println("Attach succeeded.");
      Address addr = debugger.lookup("helloworld.dll", "helloWorldString");
      System.err.println("helloWorldString address = " + addr);
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
