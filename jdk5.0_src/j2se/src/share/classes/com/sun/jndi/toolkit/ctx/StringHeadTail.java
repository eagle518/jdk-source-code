/*
 * @(#)StringHeadTail.java	1.6 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.toolkit.ctx;

/**
  * A class for returning the result of c_parseComponent().
  *
  * @author Rosanna Lee
  */
public class StringHeadTail {
    private int status;
    private String head;
    private String tail;

    public StringHeadTail(String head, String tail) {
	this(head, tail, 0);
    }

    public StringHeadTail(String head, String tail, int status) {
	this.status = status;
	this.head = head;
	this.tail = tail;
    }

    public void setStatus(int status) {
	this.status = status;
    }

    public String getHead() {
	return this.head;
    }

    public String getTail() {
	return this.tail;
    }

    public int getStatus() {
	return this.status;
    }
}
