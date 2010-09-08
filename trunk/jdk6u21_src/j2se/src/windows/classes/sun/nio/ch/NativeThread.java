/*
 * @(#)NativeThread.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;


// Signalling operations on native threads


class NativeThread {

    static long current() { return -1; }

    static void signal(long nt) { }

}
