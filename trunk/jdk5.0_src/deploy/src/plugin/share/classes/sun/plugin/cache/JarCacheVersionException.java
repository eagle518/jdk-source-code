/*
 * @(#)JarCacheVersionException.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
