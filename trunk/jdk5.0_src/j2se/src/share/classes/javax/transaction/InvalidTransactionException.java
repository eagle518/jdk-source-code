/*
 * @(#)InvalidTransactionException.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Licensed Materials - Property of IBM
 * RMI-IIOP v1.0
 * Copyright IBM Corp. 1998 1999  All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

package javax.transaction;

/**
 * This exception indicates that the request carried an invalid transaction
 * context. For example, this exception could be raised if an error
 * occured when trying to register a resource.
 */
public class InvalidTransactionException extends java.rmi.RemoteException 
{
    public InvalidTransactionException()
    {
	super();
    }

    public InvalidTransactionException(String msg)
    {
	super(msg);
    }
}

