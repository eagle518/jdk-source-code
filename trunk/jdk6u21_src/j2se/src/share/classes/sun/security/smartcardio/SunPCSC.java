/*
 * @(#)SunPCSC.java	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.smartcardio;

import java.security.*;

import javax.smartcardio.*;

/**
 * Provider object for PC/SC.
 *
 * @since   1.6
 * @version 1.4, 03/23/10
 * @author  Andreas Sterbenz
 */
public final class SunPCSC extends Provider {

    private static final long serialVersionUID = 6168388284028876579L;
    
    public SunPCSC() {
	super("SunPCSC", 1.6d, "Sun PC/SC provider");
	AccessController.doPrivileged(new PrivilegedAction<Void>() {
	    public Void run() {
		put("TerminalFactory.PC/SC", "sun.security.smartcardio.SunPCSC$Factory");
		return null;
	    }
	});
    }

    public static final class Factory extends TerminalFactorySpi {
	public Factory(Object obj) throws PCSCException {
	    if (obj != null) {
		throw new IllegalArgumentException
		    ("SunPCSC factory does not use parameters");
	    }
	    // make sure PCSC is available and that we can obtain a context
	    PCSC.checkAvailable();
	    PCSCTerminals.initContext();
	}
	/**
	 * Returns the available readers.
	 * This must be a new object for each call.
	 */
	protected CardTerminals engineTerminals() {
	    return new PCSCTerminals();
	}
    }
    
}
