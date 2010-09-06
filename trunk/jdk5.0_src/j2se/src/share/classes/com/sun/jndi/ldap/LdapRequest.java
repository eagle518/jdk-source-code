/*
 * @(#)LdapRequest.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.ldap;

import java.io.IOException;
import java.util.Vector;
import javax.naming.CommunicationException;

final class LdapRequest {

    LdapRequest next;   // Set/read in synchronized Connection methods
    int msgId;    	// read-only

    private int gotten = 0;
    private Vector replies = new Vector(3);
    private boolean cancelled = false;
    private boolean pauseAfterReceipt = false;
    private boolean completed = false;

    LdapRequest(int msgId, boolean pause) {
	this.msgId = msgId;
	this.pauseAfterReceipt = pause;
    }

    synchronized void cancel() {
	cancelled = true;

	// Unblock reader of pending request
	// Should only ever have atmost one waiter
	notify();   
    }

    synchronized boolean addReplyBer(BerDecoder ber) {
	if (cancelled) {
	    return false;
	}
	replies.addElement(ber);

	// peek at the BER buffer to check if it is a SearchResultDone PDU
	try {
	    ber.parseSeq(null);
	    ber.parseInt();
	    completed = (ber.peekByte() == LdapClient.LDAP_REP_RESULT);
	} catch (IOException e) {
	    // ignore
	}
	ber.reset();

	notify(); // notify anyone waiting for reply
	return pauseAfterReceipt;
    }

    synchronized BerDecoder getReplyBer() throws CommunicationException {
	if (cancelled) {
	    throw new CommunicationException("Request: " + msgId +
		" cancelled");
	}

	if (gotten < replies.size()) {
	    BerDecoder answer = (BerDecoder)replies.elementAt(gotten);
	    replies.setElementAt(null, gotten); // remove reference
	    ++gotten; // skip to next
	    return answer;
	} else {
	    return null;
	}
    }

    synchronized boolean hasSearchCompleted() {
	return completed;
    }
}
