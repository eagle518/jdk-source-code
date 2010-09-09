/*
 * @(#)Cover.java	1.18 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.asm;

/**
 * WARNING: The contents of this source file are not part of any
 * supported API.  Code that depends on them does so at its own risk:
 * they are subject to change or removal without notice.
 */
public class Cover {
public int Type;
public long Addr;
public int NumCommand;

     /**
      * Constructor
      */
     public Cover(int type, long addr, int command) {
        Type=type;
        Addr=addr;
        NumCommand=command;
     }
}
