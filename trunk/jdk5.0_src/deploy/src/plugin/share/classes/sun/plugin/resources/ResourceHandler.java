/*
 * @(#)ResourceHandler.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.resources;

/*
 * A singleton object which deals with I18N resources handling.
 *
 * @version 	1.3
 * @author	Stanley Man-Kit Ho
 */

import java.util.MissingResourceException;
import java.util.ResourceBundle;
import java.text.MessageFormat;


public class ResourceHandler
{
    // Localization strings.
    private static ResourceBundle rb;

    static
    {
	// Get our internationalization resources.  (Make sure this is
        // done before calling showConsoleWindow.)
        rb = ResourceBundle.getBundle("sun.plugin.resources.Activator");
    }


    /**
     * Method to get an internationalized string from the Activator resource.
     */
    public static String getMessage(String key) {
        try {
	    return rb.getString(key);
        } catch (java.util.MissingResourceException ex) {
	    return key;
	}
    }


    /**
     * Method to get an internationalized string from the Activator resource,
     * with formatted message.
     */
    public static String getFormattedMessage(String key, Object[] args) {
        try 
	{
	    MessageFormat mf = new MessageFormat(rb.getString(key));
	    return mf.format(args);
        } catch (java.util.MissingResourceException ex) {
	    return key;
	}
    }

    /**
     * Method to get an internationalized string from the Activator resource.
     */
    public static String[] getMessageArray(String key) {
        try {
	    return rb.getStringArray(key);
        } catch (java.util.MissingResourceException ex) {
	    return new String[] { key };
	}
    }
    /**
    * Method to get an internationalized accelerator key
    */
    public static int getAcceleratorKey(String key) {
        Integer value = (Integer)rb.getObject(key + ".acceleratorKey");
        return value.intValue();
    }
}



