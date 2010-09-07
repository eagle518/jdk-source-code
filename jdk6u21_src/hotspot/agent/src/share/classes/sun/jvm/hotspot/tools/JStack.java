/*
 * Copyright (c) 2004, 2005, Oracle and/or its affiliates. All rights reserved.
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

package sun.jvm.hotspot.tools;

public class JStack extends Tool {
    public JStack(boolean mixedMode, boolean concurrentLocks) {
        this.mixedMode = mixedMode;
        this.concurrentLocks = concurrentLocks;
    }

    public JStack() {
        this(true, true);
    }

    protected boolean needsJavaPrefix() {
        return false;
    }

    public String getName() {
        return "jstack";
    }

    protected void printFlagsUsage() {
       System.out.println("    -l\tto print java.util.concurrent locks");
       System.out.println("    -m\tto print both java and native frames (mixed mode)");
       super.printFlagsUsage();
    }

    public void run() {
        Tool tool = null;
        if (mixedMode) {
            tool = new PStack(false, concurrentLocks);
        } else {
            tool = new StackTrace(false, concurrentLocks);
        }
        tool.setAgent(getAgent());
        tool.setDebugeeType(getDebugeeType());
        tool.run();
    }

    public static void main(String[] args) {
        boolean mixedMode = false;
        boolean concurrentLocks = false;
        int used = 0;
        for (int i = 0; i < args.length; i++) {
            if (args[i].equals("-m")) {
                mixedMode = true;
                used++;
            } else if (args[i].equals("-l")) {
                concurrentLocks = true;
                used++;
            }
        }

        if (used != 0) {
            String[] newArgs = new String[args.length - used];
            for (int i = 0; i < newArgs.length; i++) {
                newArgs[i] = args[i + used];
            }
            args = newArgs;
        }

        JStack jstack = new JStack(mixedMode, concurrentLocks);
        jstack.start(args);
        jstack.stop();
    }

    private boolean mixedMode;
    private boolean concurrentLocks;
}
