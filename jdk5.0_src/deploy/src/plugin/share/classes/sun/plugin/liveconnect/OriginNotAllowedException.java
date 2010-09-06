/*
 * @(#)OriginNotAllowedException.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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



