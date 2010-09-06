/*
 * @(#)DeployUIManager.java	1.20 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

/**
  * Customized metal theme to be used in Java Plug-in.
  *
  * @author Stanley Man-Kit Ho
  */
import com.sun.deploy.config.Config;

import java.awt.Font;
import java.util.WeakHashMap;
import javax.swing.LookAndFeel;
import javax.swing.UIManager;
import javax.swing.plaf.FontUIResource;
import javax.swing.plaf.metal.DefaultMetalTheme;
import javax.swing.plaf.metal.MetalLookAndFeel;
import javax.swing.plaf.metal.MetalTheme;
import sun.awt.AppContext;

public final class DeployUIManager 
{
    private static final String _v = System.getProperty("java.version");
    private static final boolean _isOldJava = 
	(_v.startsWith("1.2") || _v.startsWith("1.3") || _v.startsWith("1.4"));

    static class DeployMetalTheme extends DefaultMetalTheme
    {
	private FontUIResource controlTextFont = null;
	private FontUIResource menuTextFont = null;
	private FontUIResource windowTitleFont = null;

	DeployMetalTheme()
	{
	    FontUIResource resource = super.getControlTextFont();

	    controlTextFont = new FontUIResource(resource.getName(),
						 resource.getStyle() & ~(Font.BOLD),
						 resource.getSize());

	    resource = super.getMenuTextFont();

	    menuTextFont = new FontUIResource(resource.getName(),
					      resource.getStyle() & ~(Font.BOLD),
					      resource.getSize());

	    resource = super.getWindowTitleFont();

	    windowTitleFont = new FontUIResource(resource.getName(),
						 resource.getStyle() & ~(Font.BOLD),
						 resource.getSize());
	}

	public FontUIResource getControlTextFont() 
	{
	    return controlTextFont;
	}
	public FontUIResource getMenuTextFont() 
	{
	    return menuTextFont;
	}
	public FontUIResource getWindowTitleFont() 
	{
	    return windowTitleFont;
	}
    }

    // Weak hash map to store theme information 
    // for metal look and feel.
    //
    private static WeakHashMap themeMap = new WeakHashMap();

    /**
     * Change to deployment customized theme
     */
    public static LookAndFeel setLookAndFeel()
    {
	LookAndFeel lookAndFeel = UIManager.getLookAndFeel();

	// We should store the current theme for the MetalLookAndFeel,
	// so we may restore it properly afterwards - developers may
	// have set the theme to something other than default, so it
	// is very important to restore to the previous state afterwards.
	//
	if (lookAndFeel instanceof MetalLookAndFeel)
	    themeMap.put(lookAndFeel, getCurrentMetalTheme());
	try
	{
	    // Change metal theme - make all fonts PLAIN instead of
	    // BOLD.
	    //
	    if (_isOldJava) {
	        MetalLookAndFeel.setCurrentTheme(new DeployMetalTheme());
	    }

	    if (Config.useSystemLookAndFeel()) { 
	        UIManager.setLookAndFeel(
		    UIManager.getSystemLookAndFeelClassName());

	    } else {
	        UIManager.setLookAndFeel(new MetalLookAndFeel());
	    }
	}
	catch(Exception e)
	{
	}

	return lookAndFeel;
    }


    /**
     * Restore look and feel to default one
     */
    public static void restoreLookAndFeel(LookAndFeel lookAndFeel)
    {
	try
	{
	    // We should restore the previous theme for the MetalLookAndFeel -
	    // developers may have set the theme to something other than default, 
	    // so it is very important to restore to the previous state.
	    //
	    if (lookAndFeel instanceof MetalLookAndFeel)
	    {
		MetalTheme theme = (MetalTheme) themeMap.get(lookAndFeel);

		if (theme != null)
		    MetalLookAndFeel.setCurrentTheme(theme);
		else
		    MetalLookAndFeel.setCurrentTheme(new DefaultMetalTheme());
	    }	

	    if (lookAndFeel == null) {
	        if (Config.useSystemLookAndFeel()) { 
		    UIManager.setLookAndFeel(UIManager.
			getSystemLookAndFeelClassName());	    
	        } else {
		    UIManager.setLookAndFeel(
			UIManager.getCrossPlatformLookAndFeelClassName());
	        }
            } else {
		UIManager.setLookAndFeel(lookAndFeel);
	    }
	}
	catch(Exception e)
	{
	}
    }

    // This is a method copied from MetalLookAndFeel to obtain 
    // the current metal theme - too bad it is not public in Swing.
    //
    private static MetalTheme getCurrentMetalTheme() 
    {
        AppContext context = AppContext.getAppContext();

	return (MetalTheme)context.get("currentMetalTheme");
    }
}
