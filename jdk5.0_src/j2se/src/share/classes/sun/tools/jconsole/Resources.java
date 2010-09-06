/*
 * @(#)Resources.java	1.3 04/05/20
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import java.text.MessageFormat;
import java.util.MissingResourceException;
import java.util.ResourceBundle;

/**
 * Provides resource support for jconsole.
 *
 * @version 1.3, 04/05/20
 */
public final class Resources {

    private static final Object lock = new Object();
    private static ResourceBundle resources = null;
    static {
	try {
	    resources =
		ResourceBundle.getBundle("sun.tools.jconsole.resources.JConsoleResources");
	} catch (MissingResourceException e) {
	    // gracefully handle this later
	}
    }

    private Resources() { throw new AssertionError(); }

    /**
     * Returns the text of the jconsole resource for the specified key
     * formatted with the specified arguments.
     *
     */
    public static String getText(String key, Object... args) {
	String format = getString(key);
	if (format == null) {
	    format = "missing resource key: key = \"" + key + "\", " +
		"arguments = \"{0}\", \"{1}\", \"{2}\"";
	}
        String ss = null;
        synchronized (lock) {
            /*
             * External synchronization required for safe use of
             * java.text.MessageFormat:
             */
            ss = MessageFormat.format(format, args);
        }
        return ss;
    }

    /**
     * Returns the jconsole resource string for the specified key.
     *
     */
    private static String getString(String key) {
	if (resources != null) {
	    try {
		return resources.getString(key);
	    } catch (MissingResourceException e) {
		return null;
	    }
	}
	return "missing resource bundle: key = \"" + key + "\", " +
	    "arguments = \"{0}\", \"{1}\", \"{2}\"";
    }
}
