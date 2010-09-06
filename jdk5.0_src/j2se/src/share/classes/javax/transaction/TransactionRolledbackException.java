/*
 * @(#)TransactionRolledbackException.java	1.13 03/12/19
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
 * This exception indicates that the transaction associated with processing
 * of the request has been rolled back, or marked to roll back. Thus the
 * requested operation either could not be performed or was not performed
 * because further computation on behalf of the transaction would be
 * fruitless
 */
public class TransactionRolledbackException extends java.rmi.RemoteException 
{
    public TransactionRolledbackException()
    {
	super();
    }

    public TransactionRolledbackException(String msg)
    {
	super(msg);
    }
}

