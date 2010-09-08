/*
 * @(#)ComponentAccessor.java	1.19 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.Component;
import java.awt.Container;
import java.awt.AWTEvent;
import java.awt.Font;
import java.awt.Color;

import java.awt.peer.ComponentPeer;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;

import java.util.Hashtable;

import java.security.AccessController;
import java.security.PrivilegedAction;

/**
 * A collection of methods for modifying package private fields in AWT components.
 * This class is meant to be used by Peer code only. Previously peer code
 * got around this problem by modifying fields from native code. However
 * as we move away from native code to Pure-java peers we need this class. 
 *
 * @author Bino George
 * @version  1.19
 */


class ComponentAccessor {


    private static Class componentClass;
    private static Field  fieldX;
    private static Field  fieldY;
    private static Field  fieldWidth;
    private static Field  fieldHeight;
    private static Method methodGetParentNoClientCode;
    private static Method methodGetFontNoClientCode;
    private static Method methodProcessEvent;
    private static Method methodEnableEvents;
    private static Field  fieldParent;
    private static Field  fieldBackground;
    private static Field  fieldForeground;
    private static Field  fieldFont;
    private static Field  fieldPacked;
    private static Field  fieldIgnoreRepaint;
    private static Field fieldPeer;
    private static Method methodResetGC;

    static {
        AccessController.doPrivileged( new PrivilegedAction() {
                public Object run() {
                    try {
                        componentClass = Class.forName("java.awt.Component");   
                        fieldX  = componentClass.getDeclaredField("x");
                        fieldX.setAccessible(true);
                        fieldY  = componentClass.getDeclaredField("y");
                        fieldY.setAccessible(true);
                        fieldWidth  = componentClass.getDeclaredField("width");
                        fieldWidth.setAccessible(true);
                        fieldHeight  = componentClass.getDeclaredField("height");
                        fieldHeight.setAccessible(true);
                        fieldForeground  = componentClass.getDeclaredField("foreground");
                        fieldForeground.setAccessible(true);
                        fieldBackground  = componentClass.getDeclaredField("background");
                        fieldBackground.setAccessible(true);
                        fieldFont = componentClass.getDeclaredField("font");
                        fieldFont.setAccessible(true);
                        methodGetParentNoClientCode = componentClass.getDeclaredMethod("getParent_NoClientCode", (Class[]) null);
                        methodGetParentNoClientCode.setAccessible(true);
                        methodGetFontNoClientCode = componentClass.getDeclaredMethod("getFont_NoClientCode", (Class[]) null);
                        methodGetFontNoClientCode.setAccessible(true);
                        Class[] argTypes = { AWTEvent.class }; 
                        methodProcessEvent = componentClass.getDeclaredMethod("processEvent",argTypes);
                        methodProcessEvent.setAccessible(true);
                        Class[] argTypesForMethodEnableEvents = { Long.TYPE }; 
                        methodEnableEvents = componentClass.getDeclaredMethod("enableEvents",argTypesForMethodEnableEvents);
                        methodEnableEvents.setAccessible(true);

                        fieldParent  = componentClass.getDeclaredField("parent");
                        fieldParent.setAccessible(true);
                        fieldPacked = componentClass.getDeclaredField("isPacked");
                        fieldPacked.setAccessible(true);
                        fieldIgnoreRepaint = componentClass.getDeclaredField("ignoreRepaint");
                        fieldIgnoreRepaint.setAccessible(true);

                        fieldPeer = componentClass.getDeclaredField("peer");
                        fieldPeer.setAccessible(true);

                        methodResetGC = componentClass.getDeclaredMethod("resetGC", (Class[]) null);
                        methodResetGC.setAccessible(true);
                        
                    }
                    catch (NoSuchFieldException e) {
                        System.out.println("Unable to create ComponentAccessor : ");
                        e.printStackTrace(); 
                    }
                    catch (ClassNotFoundException e) {
                        System.out.println("Unable to create ComponentAccessor : ");
                        e.printStackTrace(); 
                    }
                    catch (NoSuchMethodException e) {
                        System.out.println("Unable to create ComponentAccessor : ");
                        e.printStackTrace(); 
                    }
                    // to please javac
                    return null;
                }
            });
    }

    static void setX(Component c, int x)
    {
        try {
            fieldX.setInt(c,x);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
    }
    
    static void setY(Component c, int y)
    {
        try {
            fieldY.setInt(c,y);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
    }

    static void setWidth(Component c, int width)
    {
        try {
            fieldWidth.setInt(c,width);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
    }
    
    static void setHeight(Component c, int height)
    {
        try {
            fieldHeight.setInt(c,height);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
    }

    static void setBounds(Component c, int x, int y, int width, int height)
    {
        try {
            fieldX.setInt(c,x);
            fieldY.setInt(c,y);
            fieldWidth.setInt(c,width);
            fieldHeight.setInt(c,height);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
    }

    static int getX(Component c) {
        try {
            return fieldX.getInt(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return 0;
    }

    static int getY(Component c) {
        try {
            return fieldY.getInt(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return 0;
    }

    static int getWidth(Component c) {
        try {
            return fieldWidth.getInt(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return 0;
    }

    static int getHeight(Component c) {
        try {
            return fieldHeight.getInt(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return 0;
    }

    static boolean getIsPacked(Component c) {
        try {
            return fieldPacked.getBoolean(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return false;
    }

    static Container getParent_NoClientCode(Component c) {
        Container parent=null;

        try {
            parent = (Container) methodGetParentNoClientCode.invoke(c, (Object[]) null);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }

        return parent;
    }
    
    static Font getFont_NoClientCode(Component c) {
        Font font=null;

        try {
            font = (Font) methodGetFontNoClientCode.invoke(c, (Object[]) null);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }

        return font;
    }

    static void processEvent(Component c, AWTEvent event) {
        Font font=null;

        try {
            Object[] args = new Object[1];
            args[0] = event;
            methodProcessEvent.invoke(c,args);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }

    static void enableEvents(Component c, long event_mask) {
        try {
            Object[] args = new Object[1];
            args[0] = Long.valueOf(event_mask);
            methodEnableEvents.invoke(c,args);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }

    static void setParent(Component c, Container parent)
    {
        try {
            fieldParent.set(c,parent);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
    }

    static Color getForeground(Component c)
    {
        Color color = null;
        try {
            color = (Color) fieldForeground.get(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return color;
    }

    static Color getBackground(Component c)
    {
        Color color = null;
        try {
            color = (Color) fieldBackground.get(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return color;
    }

    static void setBackground(Component c, Color color) {
        try {
            fieldBackground.set(c, color);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
    }

    static Font getFont(Component c)
    {
        Font f = null;
        try {
            f = (Font) fieldFont.get(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return f;
    }
    static ComponentPeer getPeer(Component c) {
        ComponentPeer peer = null;
        try {
            peer = (ComponentPeer)fieldPeer.get(c);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }
        return peer;
    }

    static void setPeer(Component c, ComponentPeer peer) {
        try {
            fieldPeer.set(c, peer);
        } catch (IllegalAccessException e)
        {
            e.printStackTrace();
        }            
    }

    static boolean getIgnoreRepaint(Component comp) {
        try {
            return fieldIgnoreRepaint.getBoolean(comp);
        }
        catch (IllegalAccessException e) {
            e.printStackTrace();
        }

        return false;
    }
    static void resetGC(Component c) {
        try {
            methodResetGC.invoke(c, (Object[]) null);
        }
        catch (IllegalAccessException e) {
            e.printStackTrace();
        }
        catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }
}
