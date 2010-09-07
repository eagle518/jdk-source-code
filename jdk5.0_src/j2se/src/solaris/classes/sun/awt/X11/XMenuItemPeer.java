/*
 * @(#)XMenuItemPeer.java	1.16 04/05/26
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.ActionEvent;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.awt.geom.Rectangle2D;
import java.awt.geom.AffineTransform;

public class XMenuItemPeer extends XMenuComponentPeer implements MenuItemPeer {
    private XMenuPeer menuPeer;
    private String label;
    private boolean enabled;
    private static final int ARROW_INDENT = 15;
    private static Field f_shortcut;
    private static Method m_getActionCommand;

    static {
        f_shortcut = XToolkit.getField(MenuItem.class, "shortcut");
        m_getActionCommand = XToolkit.getMethod(MenuItem.class, "getActionCommandImpl",null);
    }

    XMenuItemPeer() {
    }

    XMenuItemPeer(MenuItem target) {
        super(target);
        setLabel(target.getLabel());
        this.enabled = target.isEnabled();
        visible = true;
    }

    void postInit(XCreateWindowParams params) {
        super.postInit(params);
    }

    public XMenuPeer getMenuPeer() {
        return menuPeer;
    }

    void setMenuPeer(XMenuPeer peer) {
        menuPeer = peer;
    }

    /**
     * @see java.awt.peer.MenuItemPeer
     */
    public void setLabel(String label) {
        this.label = label == null?"":label;
        if ((menuPeer != null) && (menuPeer.isVisible())) {
            menuPeer.repaintMenuItem(this);
        }
    }

    /**
     * @see java.awt.peer.MenuItemPeer
     */
    public void setEnabled(boolean b) {
        if (this.enabled != b) {
            this.enabled = b;
            if ((menuPeer != null) && (menuPeer.isVisible())) {
                menuPeer.repaintMenuItem(this);
            }
        }
    }

    boolean isEnabled() {
        return enabled;
    }

    /**
     * DEPRECATED:  Replaced by setEnabled(boolean).
     * @see java.awt.peer.MenuItemPeer
     */
    public void enable() {
        setEnabled( true );
    }

    /**
     * DEPRECATED:  Replaced by setEnabled(boolean).
     * @see java.awt.peer.MenuItemPeer
     */
    public void disable() {
        setEnabled( false );
    }

    // NOTE: This method may be called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    void action(final long when, final int modifiers) {
	postEvent(new ActionEvent(menuTarget, ActionEvent.ACTION_PERFORMED,
				  getActionCommand(), when,
				  modifiers));
    }

    String getLabel() {
        return label;
    }

    MenuShortcut getTargetShortcut() {
	try {
            MenuComponent l_menuTarget = getMenuTarget();
            if (l_menuTarget != null) {
	        return (MenuShortcut)f_shortcut.get(l_menuTarget);
            }
	} catch (IllegalAccessException iae) {
	    iae.printStackTrace();
	}
	return null;
    }

    int getWidth(Graphics g) {
	g.setFont(getFont());
	FontMetrics fm = g.getFontMetrics();
	String str = getLabel();
	return fm.stringWidth(str);
    }

    int getHeight(Graphics g) {
	g.setFont(getFont());
	FontMetrics fm = g.getFontMetrics();
	return fm.getHeight();
    }

    int getShortcutWidth(Graphics g) {
	g.setFont(getFont());
	FontMetrics fm = g.getFontMetrics();
	MenuShortcut sc = getTargetShortcut();
	String scStr = sc != null ? "   " + sc.toString() : "";
	return fm.stringWidth(scStr.trim());
    }

    void paint(Graphics g, int top, int bottom, int width, int shortcutOffset, boolean selected) {
        if (!getMenuPeer().isVisible() || isDisposed()) {
            return;
        }
	g.setFont(getFont());
	FontMetrics fm = g.getFontMetrics();
	int h = fm.getHeight();
	int y = top + (h + fm.getMaxAscent() - fm.getMaxDescent()) / 2;
	String str = getLabel();
	MenuShortcut sc = getTargetShortcut();
	String scStr = sc != null ? sc.toString() : "";
	if (str.equals("-")) {
	    g.setColor(getBackground());
	    g.fillRect(2, top, width-4, 1);
	    draw3DRect(g, 2, top, width-5, 1, false);
	} else {
	    if (selected) {
		g.setColor(getSelect());
		g.fillRect(2, top, width-4, h);  
		draw3DRect(g, 2, top, width-5, bottom-top, false);
	    } else {
		g.setColor(getBackground());
		g.fillRect(2, top, width-4, h);
	    }
	    g.setColor(isEnabled()
		       ? getForeground()
		       : getDisabled());
	    Shape clip = g.getClip();
	    if (isEnabled() && (menuTarget instanceof Menu)) {
		g.setClip(0, top, width-ARROW_INDENT, bottom-top+1);
	    } else {
		g.setClip(0, top, width-3, bottom-top+1);
	    }
	    g.drawString(str, menuPeer.getXIndent(), y);
	    if (sc != null) {
		g.drawString(scStr, menuPeer.getXIndent() + shortcutOffset, y);
	    }
	    g.setClip(clip);

	    if (isEnabled() && (menuTarget instanceof Menu)) {
		int ax = width - ARROW_INDENT;
		g.setColor(getDarkShadow());
		g.drawLine(ax, y - 2, ax + 8, y - 6);
		g.setColor(getLightShadow());
		g.drawLine(ax + 8, y - 6, ax, y - 10);
		g.drawLine(ax, y - 10, ax, y - 2);
	    } else if (menuTarget instanceof CheckboxMenuItem) {
		paintCheck(g, 1, top, width, h);
	    }
	}
    }

    void paintCheck(Graphics g, int x, int y, int width, int height) {
    }

    String getActionCommand() {
        String str = "";
        try {
            str = (String) m_getActionCommand.invoke(menuTarget,null);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }
        return str;
    }

    void setVisible(boolean b) {
        visible = b;
    }

    public String toString() {
	return getClass().getName() + "[label=" + getLabel() + "]";
    }
}
