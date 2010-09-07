/*
 * @(#)JVMEventListener.java	1.2 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.jvm;

import java.util.EventListener;

/** Provides an event listener mechanism for running subordinate JVM
    instances. This is principally intended to provide a mechanism for
    listening for a JVM's unexpected exit. */

public interface JVMEventListener extends EventListener {
    public void jvmExited(JVMLauncher launcher);
}
