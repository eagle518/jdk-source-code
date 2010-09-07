/*
 * @(#)AppInfo.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.ui;

import com.sun.deploy.config.Config;
import com.sun.deploy.cache.AssociationDesc;
import java.net.URL;
import sun.awt.AppContext;


public class AppInfo {

    public final static int TYPE_UNKNOWN          = 0;
    public final static int TYPE_APPLICATION      = 1;
    public final static int TYPE_APPLET           = 2;
    public final static int TYPE_LIBRARY          = 3;
    public final static int TYPE_INSTALLER        = 4;

    // DialogTemplate uses 48x48 icons
    public final static int ICON_SIZE       	  = 48;

    private int type = TYPE_UNKNOWN;
    private String title = null;
    private String vendor = null;
    private URL from = null;
    private URL    iconRef = null;
    private String iconVersion = null;
    private boolean desktopHint = false;
    private boolean menuHint = false;
    private String submenu = null;
    private AssociationDesc [] associations = new AssociationDesc[0];

    public AppInfo() {
        type = TYPE_APPLET;

	AppContext context = AppContext.getAppContext();
	Object obj = context.get(Config.APPCONTEXT_APP_NAME_KEY);
	if (obj != null) {
            title = obj.toString();
	}
    }

    public AppInfo(int type, String title, String vendor, 
                   URL from, URL iconRef, String iconVersion,
                   boolean desktop, boolean menu, String submenu, 
                   AssociationDesc[] associations) {
        this.type = type;
        this.title = title;
        this.vendor = vendor;
        this.from = from;
        this.iconRef = iconRef;
        this.iconVersion = iconVersion;
        this.desktopHint = desktop;
        this.menuHint = menu;
        this.submenu = submenu;
        this.associations = associations;
    }


    public int getType()                        { return type; }
    public String getTitle()                    { return title; }
    public String getVendor()                   { return vendor; }
    public URL    getFrom()                     { return from; }
    public URL    getIconRef()                  { return iconRef; }
    public String getIconVersion()              { return iconVersion; }
    public boolean getDesktopHint()             { return desktopHint; }
    public boolean getMenuHint()                { return menuHint; }
    public String getSubmenu()                  { return submenu; }
    public AssociationDesc [] getAssociations() { return associations; }


    public void setType(int i)                       { type = i; }
    public void setTitle(String s)                   { title = s; }
    public void setVendor(String s)                  { vendor = s; }
    public void setFrom(URL u)                       { from = u; }
    public void setIconRef(URL url)                  { iconRef = url; }
    public void setIconVersion(String s)             { iconVersion = s; }
    public void setDesktopHint(boolean b)            { desktopHint = b; }
    public void setMenuHint(boolean b)               { menuHint = b; }
    public void setSubmenu(String s)                 { submenu = s; }
    public void setAssociations(AssociationDesc[] a) { associations = a; }


}

