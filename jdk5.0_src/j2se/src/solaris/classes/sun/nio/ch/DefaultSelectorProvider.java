/*
 * @(#)DefaultSelectorProvider.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.nio.ch;

import java.nio.channels.spi.SelectorProvider;
import java.security.AccessController;
import java.security.PrivilegedAction;
import sun.security.action.GetPropertyAction;

/**
 * Creates this platform's default SelectorProvider
 */

public class DefaultSelectorProvider {

    /**
     * Prevent instantiation.
     */
    private DefaultSelectorProvider() { }

    /**
     * Returns the default SelectorProvider.
     */
    public static SelectorProvider create() {
	PrivilegedAction pa
	    = new GetPropertyAction("os.version");
	String result = (String) AccessController.doPrivileged(pa);
        String[] numbers = result.split("\\.", 0);
        if (numbers[1].compareTo("7") < 0)
            return new sun.nio.ch.PollSelectorProvider();
        else
            return new sun.nio.ch.DevPollSelectorProvider();
    }

}
