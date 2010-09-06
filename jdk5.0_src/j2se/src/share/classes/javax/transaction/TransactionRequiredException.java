/*
 * @(#)TransactionRequiredException.java	1.13 03/12/19
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
 * This exception indicates that a request carried a null transaction context,
 * but the target object requires an activate transaction.
 */
public class TransactionRequiredException extends java.rmi.RemoteException 
{
    public TransactionRequiredException()
    {
	super();
    }

    public TransactionRequiredException(String msg)
    {
	super(msg);
    }
}

