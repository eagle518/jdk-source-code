/*
 * @(#)OriginNotAllowedException.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.liveconnect;

/**
 * @version 	1.1 
 * @author	Stanley Man-Kit Ho
 */

public class OriginNotAllowedException extends Exception 
{
    OriginNotAllowedException()
    {
	super();
    }

    OriginNotAllowedException(String msg)
    {
	super(msg);
    }
}



