/*
 * @(#)XMenuComponentPeer.java	1.18 03/12/19
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import java.awt.event.MouseEvent;
import java.awt.event.FocusEvent;
import java.awt.event.KeyEvent;
import java.awt.event.ActionEvent;
import sun.java2d.SunGraphics2D;
import java.awt.image.ColorModel;
import sun.awt.X11GraphicsConfig;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;


public abstract class XMenuComponentPeer extends XMenuWindow implements MenuComponentPeer {

    private XFramePeer framePeer;
    private Font font;
    Color background;
    private Color foreground;
    private Color lightShadow;
    private Color darkShadow;
    private Color select;
    private Color disabled;
    private static Field f_mParent;
    private static Field f_cParent;
    private static Field f_mPeer;
    private static Field f_cPeer;
    private static Method m_menuGetFontMethod;
    private Component parentComponent;

    static {
        f_mParent = XToolkit.getField(MenuComponent.class, "parent");
        f_cParent = XToolkit.getField(Component.class, "parent");
        f_mPeer = XToolkit.getField(MenuComponent.class, "peer");
        f_cPeer = XToolkit.getField(Component.class, "peer");
        m_menuGetFontMethod = XToolkit.getMethod(MenuComponent.class, "getFont_NoClientCode", new Class[0]);
    }

    XMenuComponentPeer() {
    }

    XMenuComponentPeer(MenuComponent target) {
        super(target);
        frame = getFrame();
    }

    void preInit(XCreateWindowParams params) {
	super.preInit(params);
    }

    void postInit(XCreateWindowParams params) {
	super.postInit(params);
        setBackground(getFrame().getBackground());
        setVisible(true);
    }

    MenuContainer getParent() {
        return getParent(menuTarget);
    }

    MenuContainer getParent(MenuComponent target) {
	try { 
	    return (MenuContainer)f_mParent.get(target);
	} catch (IllegalAccessException iae) {
	    iae.printStackTrace();
	    return null;
	} 
    }

    Component getParent(Component target) {
	try { 
	    return (Component)f_cParent.get(target);
	} catch (IllegalAccessException iae) {
	    iae.printStackTrace();
	    return null;
	} 
    }

    MenuComponentPeer getPeer(MenuComponent target) {
	try { 
            if (target != null) {
	        return (MenuComponentPeer)f_mPeer.get(target);
            }
	} catch (IllegalAccessException iae) {
	    iae.printStackTrace();
        }
	return null;
    }

    ComponentPeer getPeer(Component target) {
	try { 
            if (target == null) {
                return null;
            } else {
	        return (ComponentPeer)f_cPeer.get(target);
            }
	} catch (IllegalAccessException iae) {
	    iae.printStackTrace();
	    return null;
        }
    }

    Font getFont() {
        Font l_font = font;
	if (l_font == null) {
	    Object obj = menuTarget;
	    Font targetFont = targetGetFont();
	    if (targetFont == null) {
		return defaultFont;
	    } else {
		return targetFont;
	    }
	}
	else {
	    return l_font;
	}
    }

    public void setFont(Font f) {
	font = f;
        repaint();
    }

    public MenuComponent getTarget() {
        return menuTarget;
    }

    public Graphics getGraphics() {
	if (getSurfaceData() == null) return null;
	return getGraphics(getSurfaceData(),
			   getForeground(),
			   getBackground(),
			   getFont());
    }

    Component getParentComponent() {
        if (parentComponent != null) {
            return parentComponent;
        }
        else {
            Object obj = menuTarget;
            while (!(obj instanceof Component) && (obj != null)) {
                if (obj instanceof MenuComponent) {
                    obj = getParent((MenuComponent)obj);
                } else if (obj instanceof Component) {
                    obj = getParent((Component)obj);
                }
            }
            parentComponent = (Component)obj;
            return parentComponent;
        }
    }

    Color getBackground() {
        Color componentBackground = null;
        Component parentComp = getParentComponent();
        if (parentComp != null) {
            componentBackground = parentComp.getBackground();
        }
        if (componentBackground == null) {
            componentBackground = SystemColor.window;
        }
        if (background != componentBackground) {
            setBackground(componentBackground);
        }
        return background;
    }

    public void setBackground(Color c) {
        if (surfaceData != null) {
            super.setBackground(c);
        }
        setColors(c);
    }

    Color getForeground() {
        return foreground;
    }

    Color getLightShadow() {
        return lightShadow;
    }

    Color getDarkShadow() {
        return darkShadow;
    }

    Color getSelect() {
        return select;
    }

    Color getDisabled() {
        return disabled;
    }

    void setColors(Color bg) {
	int red = bg.getRed();
	int green = bg.getGreen();
	int blue = bg.getBlue();

	background = bg;
	foreground = new Color(MotifColorUtilities.calculateForegroundFromBackground(red,green,blue));
	lightShadow = new Color(MotifColorUtilities.calculateTopShadowFromBackground(red,green,blue));
	darkShadow = new Color(MotifColorUtilities.calculateBottomShadowFromBackground(red,green,blue));
	select = new Color(MotifColorUtilities.calculateSelectFromBackground(red,green,blue)); 
	disabled = bg.darker();
    }

    void draw3DRect(Graphics g, int x, int y, int width, int height, boolean raised) {
	Color c = g.getColor();
	g.setColor(raised ? getLightShadow() : getDarkShadow());
	g.drawLine(x, y, x, y + height);
	g.drawLine(x + 1, y, x + width, y);
	g.setColor(raised ? getDarkShadow() : getLightShadow());
	g.drawLine(x + 1, y + height, x + width, y + height);
	g.drawLine(x + width, y + 1, x + width, y + height - 1);
	g.setColor(c);
    }

    final Font targetGetFont() {
        try {
            MenuComponent l_menuTarget = getMenuTarget();
            if (l_menuTarget != null) {
                return (Font)m_menuGetFontMethod.invoke(l_menuTarget, new Object[0]);
            }
        }
        catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }
        return null; 
    }

    final Object getMenuTreeLock() {
        return getFrame().getTreeLock();
    }

    Frame getFrame() {
        if (frame != null) {
            return frame;
        }
        else {
            Object obj = menuTarget;
	    while (!(obj instanceof Frame) && (obj != null)) {
		if (obj instanceof MenuComponent) {
		    obj = getParent((MenuComponent)obj);
		} else if (obj instanceof Component) {
		    obj = getParent((Component)obj);
		}
	    }
            setFrame((Frame)obj);
            return frame;
        }
    }

    void setFrame( Frame frame ) {
        this.frame = frame;
        this.framePeer = (XFramePeer)getPeer(frame);
    }
}
