/*
 * @(#)ClassFinder.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

import java.io.IOException;
import java.lang.reflect.Method;
import java.lang.reflect.InvocationTargetException;
import java.util.Hashtable;
import java.util.Properties;

/** This class find an load SignatureClass. The method Class.forName
 *  is used for founding of the Class object. If the Class.forName(String,
 *  boolean, ClassLoader) is available, than this method will be
 *  used. Otherwise, Class.forName(String) will be used. **/
public class ClassFinder {
    /** formats member definitions. **/
    protected DefinitionFormat filter;
    protected Properties details;
 
   /** The the Class.forName(String, boolean, ClassLoader) method.
     *  If this method is not available, then rhis field is null. **/
    private Method forName = null;
    /** arguments of the Class.forName method. **/
    private Object[] args = null;

    /** creates ClassFinder. **/
    public ClassFinder(DefinitionFormat filter, Properties details) {
        this.details = details;
        args = new Object[] {
            "",
            Boolean.FALSE,
            this.getClass().getClassLoader()
        };
        this.filter = filter;
        Class c = Class.class;
        Class[] param = {
            String.class,
            Boolean.TYPE,
            ClassLoader.class
        };
        try {
            forName = c.getDeclaredMethod("forName", param);
        } catch (NoSuchMethodException e) {
        } catch (SecurityException e1) {
        }
    } 

    /** loads class with the given name. **/
    public SignatureClass loadClass(String name)
        throws ClassNotFoundException {
        if (forName == null) {
            return new SignatureClass(Class.forName(name), filter, this,
                                      details);
        } else {
            args[0] = name;
            try {
                return new SignatureClass((Class)forName.invoke(null, args),
                                          filter, this, details);
            } catch (InvocationTargetException e) {
                Throwable t = e.getTargetException();
                if (t instanceof LinkageError)
                    throw (LinkageError)t;
                else if (t instanceof ClassNotFoundException)
                    throw (ClassNotFoundException)t;
                else
                    // if the other than Exception of the 
                    return new SignatureClass(Class.forName(name), filter, this,
                                              details);
            } catch (Throwable t) {
                return new SignatureClass(Class.forName(name), filter, this,
                                          details);
            }
        }
    }
}
