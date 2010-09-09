/*
 * @(#)MailcapParseException.java	1.6 10/04/02
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * 
 * This software is the proprietary information of Oracle.  
 * Use is subject to license terms.
 * 
 */

package	com.sun.activation.registries;

/**
 *	A class to encapsulate Mailcap parsing related exceptions
 */
public class MailcapParseException extends Exception {

    public MailcapParseException() {
	super();
    }

    public MailcapParseException(String inInfo) {
	super(inInfo);
    }
}
