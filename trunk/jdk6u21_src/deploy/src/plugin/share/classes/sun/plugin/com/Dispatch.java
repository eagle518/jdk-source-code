/*
 * @(#)Dispatch.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

/**
 *  Dispatch is the Java side companion to the IDispatch COM interface.
 */
public interface Dispatch
{
    /**
     * Invoke a method according to the method index.
     *
     * @param flag invoke context
     * @param methodIndex Method index
     * @param args Arguments.
     * @return Java object.
     */
    public Object invoke(int flag, int methodIndex, Object [] args)
        throws  Exception;

    /**
     * Return the Java object wrapped by this proxy/
     */
    public int getIdForName(String methodName) throws Exception;


    /**
     * Return the return type for the method
     */
    public int getReturnType(int flag, int methodIndex, Object [] args);

    /**
     * Return the wrapped Java object 
     */
    public Object getWrappedObject();

    //constants
    public final static int METHOD = 0x1;
    public final static int PROPERTYGET = 0x2;
    public final static int PROPERTYPUT = 0x4;
    public final static int PROPERTYPUTREF = 0x8;

    public final static int propertyBase = 0x1000;
    public final static int eventBase = 0x4000;
    public final static int methodBase = 0x8000;

    public final static int JT_BOOL = 1;
    public final static int JT_BYTE = 2;
    public final static int JT_CHAR = 3;
    public final static int JT_SHORT = 4;
    public final static int JT_INT = 5;
    public final static int JT_LONG = 6;
    public final static int JT_FLOAT = 7;
    public final static int JT_DOUBLE = 8;
    public final static int JT_STRING = 9;
    public final static int JT_ARRAY = 10;
    public final static int JT_OBJECT = 11;
}

