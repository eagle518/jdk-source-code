/*
 * @(#)JarCacheVersionException.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.cache;

public class JarCacheVersionException extends Exception
{
    JarCacheVersionException()
    {
    }    

    JarCacheVersionException(String msg)
    {
	super(msg);
    }    
}
