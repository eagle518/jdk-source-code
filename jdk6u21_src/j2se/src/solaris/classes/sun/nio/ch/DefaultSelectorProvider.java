/*
 * @(#)DefaultSelectorProvider.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
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
	PrivilegedAction pa = new GetPropertyAction("os.name");
	String osname = (String) AccessController.doPrivileged(pa);
        if ("SunOS".equals(osname)) {
            return new sun.nio.ch.DevPollSelectorProvider();
        }

        // use EPollSelectorProvider for Linux kernels >= 2.6
        if ("Linux".equals(osname)) {
            pa = new GetPropertyAction("os.version");
            String osversion = (String) AccessController.doPrivileged(pa);
            String[] vers = osversion.split("\\.", 0);
            if (vers.length >= 2) {
                try {
                    int major = Integer.parseInt(vers[0]);
                    int minor = Integer.parseInt(vers[1]);
                    if (major > 2 || (major == 2 && minor >= 6)) {
                        return new sun.nio.ch.EPollSelectorProvider();
                    }
                } catch (NumberFormatException x) {
                    // format not recognized
                }
            }
        }

        return new sun.nio.ch.PollSelectorProvider();
    }

}
