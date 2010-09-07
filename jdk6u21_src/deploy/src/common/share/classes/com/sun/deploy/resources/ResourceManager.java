/*
 * @(#)ResourceManager.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.resources;

/*
 * A singleton object which deals with I18N resources handling.
 *
 * @version 	1.3
 * @author	Stanley Man-Kit Ho
 */

import java.awt.Color;
import java.awt.Font;
import java.lang.reflect.Field;
import java.util.MissingResourceException;
import java.util.ResourceBundle;
import java.text.MessageFormat;
import java.text.NumberFormat;
import javax.swing.ImageIcon;
import javax.swing.JLabel;
import com.sun.deploy.util.Trace;

public class ResourceManager
{
    // Localization strings.
    private static ResourceBundle rb;
    private static NumberFormat   _numberFormat = null;

    static
    {
	// Get our internationalization resources.  (Make sure this is
        // done before calling showConsoleWindow.)
        rb = ResourceBundle.getBundle("com.sun.deploy.resources.Deployment");
	// Get number formatter object for current local
	_numberFormat = NumberFormat.getInstance();
    }


    /**
     * Method to get an internationalized string from the deployment resource.
     */
    public static String getMessage(String key) {
        try {
	    return rb.getString(key);
        } catch (java.util.MissingResourceException ex) {
	    return key;
	}
    }


    /**
     * Method to get an internationalized string from the deployment resource,
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
     * Method to get an internationalized string from the deployment resource.
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

      /**
    * Returns a string from the resources
    */
    static public String getString(String key) {
        try {
            return rb.getString(key);
        } catch (MissingResourceException mre) {
            Trace.ignoredException(mre);
            return null;
        }
    }
    
    /**
    * Returns an Integer from the resources
    */
    static public int getInteger(String key) {
        try {
            return Integer.parseInt(getString(key),16);
        } catch (MissingResourceException mre) {
            Trace.ignoredException(mre);
            return -1;
        }
    }
    
    /**
    * Returns a string from a resource, substituting argument 1
    */
    static public String getString(String key, String arg) {
        Object[] messageArguments = { arg };
        return applyPattern(key, messageArguments);
    }
    
    /**
    * Returns a string from a resource, substituting argument 1 and 2
    */
    static public String getString(String key, String arg1, String arg2) {
        Object[] messageArguments = { arg1, arg2};
        return applyPattern(key, messageArguments);
    }

    /**
    * Returns a string from a resource, substituting argument 1 and 2
    */
    static public String getString(String key, Long arg1, Long arg2) {
        Object[] messageArguments = { arg1, arg2};
        return applyPattern(key, messageArguments);
    }
    
    /**
    * Returns a string from a resource, substituting argument 1,2, and 3
    */
    static public String getString(String key, String arg1, String arg2, String arg3) {
        Object[] messageArguments = { arg1, arg2, arg3 };
        return applyPattern(key, messageArguments);
    }
    
    /**
    * Returns a string from a resource, substituting argument 1,2,3 and 4
    */
    static public String getString(String key, String arg1, String arg2, 
            String arg3, String arg4) {
        Object[] messageArguments = { arg1, arg2, arg3, arg4};
        return applyPattern(key, messageArguments);
    }
     
    /**
    *  Returns a string from a resource, substituting the integer
    */
    static public String getString(String key, int arg1) {
        Object[] messageArguments = { new Integer(arg1) };
        return applyPattern(key, messageArguments);
    }
    
    /**
    *  Returns a string from a resource, substituting the three integers
    */
    static public String getString(String key, int arg1, int arg2, int arg3) {
        Object[] messageArguments = { new Integer(arg1), new Integer(arg2), new Integer(arg3) };
        return applyPattern(key, messageArguments);
    }
    
    static public String getString(String key, String arg1, int arg2, String arg3) {
        Object[] messageArguments = { arg1, new Integer(arg2), arg3 };
        return applyPattern(key, messageArguments);
    }
    
    static public String getString(String key, String arg1, int arg2) {
        Object[] messageArguments = { arg1, new Integer(arg2) };
        return applyPattern(key, messageArguments);
    }
        
    static public synchronized String formatDouble(double d, int digits) {
        _numberFormat.setGroupingUsed(true);
        _numberFormat.setMaximumFractionDigits(digits);
        _numberFormat.setMinimumFractionDigits(digits);
        return _numberFormat.format(d);
    }
        
    /**
    * Returns the Icon given a resource name
    */
    static public ImageIcon getIcon(String key) {
        String resourceName = getString(key);
        return new ImageIcon(ResourceManager.class.getResource(resourceName));
    }

    /**
     * returns up to 4 icons for a JButton (normal, pressed, disabled, rollover
     */
    static public ImageIcon [] getIcons(String key) {
	ImageIcon icons[] = new ImageIcon[4];

	String name = getString(key);
	icons[0] = new ImageIcon(ResourceManager.class.getResource(name));
	int i = name.lastIndexOf(".");
	String root = name;
	String extension = "";
	if (i > 0) {
	    root = name.substring(0, i);
	    extension = name.substring(i);
	}
	icons[1] =  new ImageIcon(ResourceManager.class.getResource(
	    root + "-p" + extension));
	icons[2] =  new ImageIcon(ResourceManager.class.getResource(
	    root + "-d" + extension));
	icons[3] =  new ImageIcon(ResourceManager.class.getResource(
	    root + "-o" + extension));
	return icons;
    }

    
    /** Helper function that applies the messageArguments to a message from the resource object */
    static private String applyPattern(String key, Object[] messageArguments) {
        String message = getString(key);
        MessageFormat formatter = new MessageFormat(message);
        String output = formatter.format(message, messageArguments);
        return output;
    }

    /**
    * Returns a Color Object from the resources
    */
    public static Color getColor(String key) {
	int rgb = getInteger(key);
	return new Color(rgb);
    }

    /**
     * Return font used in UI
     */
    public static Font getUIFont() {
	return (new JLabel().getFont());
    }
    
    /*
     * Return min font specified in the resource - if any.
     */
    public static int getMinFontSize(){
        int fontSize = 0;
        try{
            fontSize = ((Integer)rb.getObject("ui.min.font.size")).intValue();
        }catch(java.util.MissingResourceException mre){}
        
        return fontSize;
    }

    /**
    * Returns a Virtual Key Code from the resources
    */
    static Class _keyEventClazz = null;
    static public int getVKCode(String key) {
	String resource = getString(key);
	if (resource != null && resource.startsWith("VK_")) try {
            if (_keyEventClazz == null) {
                _keyEventClazz= Class.forName("java.awt.event.KeyEvent");
	    }
            Field field = _keyEventClazz.getDeclaredField(resource);
            int value = field.getInt(null);
            return value;
	} catch (ClassNotFoundException cnfe) { 
            Trace.ignoredException(cnfe);    
        } catch (NoSuchFieldException nsfe) {
            Trace.ignoredException(nsfe);
        } catch (SecurityException se) {
            Trace.ignoredException(se);
        } catch (Exception e) {
            Trace.ignoredException(e);
        }
	return 0;
    }
}



