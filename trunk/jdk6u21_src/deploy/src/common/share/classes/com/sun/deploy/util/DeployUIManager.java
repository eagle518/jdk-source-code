/*
 * @(#)DeployUIManager.java	1.26 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

/**
  * Customized metal theme to be used in Java Plug-in.
  *
  * @author Stanley Man-Kit Ho
  */
import com.sun.deploy.config.Config;

import java.awt.Font;
import java.util.HashMap;
import javax.swing.plaf.FontUIResource;
import javax.swing.plaf.metal.DefaultMetalTheme;
import javax.swing.plaf.metal.MetalLookAndFeel;
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

    /**
     * Change to deployment customized theme
     */
    public static void setLookAndFeel()
    {
	try
	{
	    // Change metal theme - make all fonts PLAIN instead of
	    // BOLD.
	    //
	    if (_isOldJava) {
	        MetalLookAndFeel.setCurrentTheme(new DeployMetalTheme());
	    }

            // Set and customize default LAF without accessing UIManager class.
            // The necessary values are passed through AppContext (6653395)
            HashMap lafData = new HashMap(2);
	    if (Config.useSystemLookAndFeel()) {
                lafData.put("defaultlaf", getSystemLookAndFeelClassName());
                lafData.put("Slider.paintValue", Boolean.FALSE);
	    } else {
	        lafData.put("defaultlaf",
                            "javax.swing.plaf.metal.MetalLookAndFeel");
	    }
            AppContext.getAppContext().put("swing.lafdata", lafData);
	}
	catch(Exception e)
	{
	}
    }
    
    public static String getSystemLookAndFeelClassName() {
        String systemLAF = System.getProperty("swing.systemlaf");
        if (systemLAF != null) {
            return systemLAF;
        }

        String osName = System.getProperty("os.name");
        if (osName != null) {
            if (osName.indexOf("Windows") != -1) {
                return "com.sun.java.swing.plaf.windows.WindowsLookAndFeel";
            } else {
                String desktop = System.getProperty("sun.desktop");
                if ("gnome".equals(desktop)) {
                    return "com.sun.java.swing.plaf.gtk.GTKLookAndFeel";
                }
                if (osName.indexOf("Solaris") != -1 || 
                        osName.indexOf("SunOS") != -1) {
                    return "com.sun.java.swing.plaf.motif.MotifLookAndFeel";
                }
            }
        }
        return "javax.swing.plaf.metal.MetalLookAndFeel";
    }
}
