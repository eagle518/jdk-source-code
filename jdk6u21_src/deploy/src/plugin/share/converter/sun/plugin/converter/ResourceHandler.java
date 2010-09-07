/*
 * @(#)ResourceHandler.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.converter;

/*
 * @version 	1.1 
 * @author	Stanley Man-Kit Ho
 */

import java.util.ResourceBundle;

public class ResourceHandler {

    // Localization strings.
    private static ResourceBundle rb = ResourceBundle.getBundle("sun.plugin.converter.resources.Converter");

    /**
     * Method to get an internationalized string from the Converter resource.
     */
    public static String getMessage(String key) {
        try {
	    return rb.getString(key);
        } catch (java.util.MissingResourceException ex) {
	    return key;
	}
    }

    /**
     * Method to get an internationalized string from the Converter resource.
     */
    public static String[] getMessageArray(String key) {
        try {
	    return rb.getStringArray(key);
        } catch (java.util.MissingResourceException ex) {
	    return new String[] { key };
	}
    }

   /*
    * Method to get an international object from the Converter resource.
    *
    * @return The object requested.  If not found null is returned.
    */
   public static Object getResource(String key) {
      try {
         return rb.getObject(key);
      }
      catch (java.util.MissingResourceException ex) {
         ex.printStackTrace();
         return null;
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
