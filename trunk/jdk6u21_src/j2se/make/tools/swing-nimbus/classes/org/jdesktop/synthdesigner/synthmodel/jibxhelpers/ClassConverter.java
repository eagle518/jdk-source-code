/*
 * @(#)ClassConverter.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel.jibxhelpers;

/**
 * ClassConverter
 *
 * @author Created by Jasper Potts (Jul 16, 2007)
 * @version 1.0
 */
public class ClassConverter {

    public static String classToString(Class c) {
        return c == null ? "" : c.getName();
    }

    public static Class stringToClass(String className) {
        if (className == null || className.length() == 0) {
            return null;
        }
        try {
            ClassLoader classLoader = Thread.currentThread().getContextClassLoader();
            if (classLoader == null) classLoader = ClassConverter.class.getClassLoader();
            return classLoader.loadClass(className);
        } catch (ClassNotFoundException e) {
            System.err.println("Failed to find class with name [" + className + "] in ClassConverter");
            e.printStackTrace();
            return null;
        }
    }

}
