/*
 * @(#)SwingLazyValue.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.swing;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import javax.swing.UIDefaults;

/**
 * SwingLazyValue is a copy of ProxyLazyValue that does not snapshot the
 * AccessControlContext or use a doPrivileged to resolve the class name.
 * It's intented for use in places in Swing where we need ProxyLazyValue, this
 * should never be used in a place where the developer could supply the
 * arguments.
 *
 * @version 1.4 12/19/03
 */
public class SwingLazyValue implements UIDefaults.LazyValue {
    private String className;
    private String methodName;
    private Object[] args;

    public SwingLazyValue(String c) {
        this(c, (String)null);
    }
    public SwingLazyValue(String c, String m) {
        this(c, m, null);
    }
    public SwingLazyValue(String c, Object[] o) {
        this(c, null, o);
    }
    public SwingLazyValue(String c, String m, Object[] o) {
        className = c;
        methodName = m;
        if (o != null) {
            args = (Object[])o.clone();
        }
    }

    public Object createValue(final UIDefaults table) {
        try {
            Class c;
            Object cl;
            c = Class.forName(className, true, null);
            if (methodName != null) {
                Class[] types = getClassArray(args);
                Method m = c.getMethod(methodName, types);
                return m.invoke(c, args);
            } else {
                Class[] types = getClassArray(args);
                Constructor constructor = c.getConstructor(types);
                return constructor.newInstance(args);
            }
        } catch(Exception e) {
            // Ideally we would throw an exception, unfortunately
            // often times there are errors as an initial look and
            // feel is loaded before one can be switched. Perhaps a
            // flag should be added for debugging, so that if true
            // the exception would be thrown.
        }
        return null;
    }

    private Class[] getClassArray(Object[] args) {
        Class[] types = null;
        if (args!=null) {
            types = new Class[args.length];
            for (int i = 0; i< args.length; i++) {
                /* PENDING(ges): At present only the primitive types 
                   used are handled correctly; this should eventually
                   handle all primitive types */
                if (args[i] instanceof java.lang.Integer) {
                    types[i]=Integer.TYPE;
                } else if (args[i] instanceof java.lang.Boolean) {
                    types[i]=Boolean.TYPE;			
                } else if (args[i] instanceof javax.swing.plaf.ColorUIResource) {
                    /* PENDING(ges) Currently the Reflection APIs do not 
                       search superclasses of parameters supplied for
                       constructor/method lookup.  Since we only have
                       one case where this is needed, we substitute
                       directly instead of adding a massive amount
                       of mechanism for this.  Eventually this will
                       probably need to handle the general case as well.
                    */
                    types[i]=java.awt.Color.class;
                } else {
                    types[i]=args[i].getClass();
                }
            }
        }
        return types;
    }
}
