/*
 * @(#)Accessible.java	1.16 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

/**
 * Provides information on the accessibility of a type or type component.
 * Mirrors for program elements which allow an
 * an access specifier (private, protected, public) provide information
 * on that part of the declaration through this interface.
 *
 * @author Robert Field
 * @author Gordon Hirsch
 * @author James McIlree
 * @since  1.3
 */
public interface Accessible {                              

    /**
     * Returns the Java<sup><font size=-2>TM</font></sup>
     * programming language modifiers, encoded in an integer. 
     * <p>
     * The modifier encodings are defined in the
     * <a href="http://java.sun.com/docs/books/vmspec/">Java Virtual Machine
     * Specification</a>, in the <code>access_flag</code> tables for
     * <a href="http://java.sun.com/docs/books/vmspec/2nd-edition/html/ClassFile.doc.html#75734">classes</a>,
     * <a href="http://java.sun.com/docs/books/vmspec/2nd-edition/html/ClassFile.doc.html#88358">fields</a>, and 
     * <a href="http://java.sun.com/docs/books/vmspec/2nd-edition/html/ClassFile.doc.html#75568">methods</a>.
     */
    public int modifiers();

    /**
     * Determines if this object mirrors a private item. 
     * For {@link ArrayType}, the return value depends on the 
     * array component type. For primitive arrays the return value 
     * is always false. For object arrays, the return value is the
     * same as would be returned for the component type. 
     * For primitive classes, such as {@link java.lang.Integer#TYPE},
     * the return value is always false.
     *
     * @return <code>true</code> for items with private access; 
     * <code>false</code> otherwise.
     */
    boolean isPrivate(); 

    /**
     * Determines if this object mirrors a package private item. 
     * A package private item is declared with no access specifier.
     * For {@link ArrayType}, the return value depends on the 
     * array component type. For primitive arrays the return value 
     * is always false. For object arrays, the return value is the
     * same as would be returned for the component type. 
     * For primitive classes, such as {@link java.lang.Integer#TYPE},
     * the return value is always false.
     *
     * @return <code>true</code> for items with package private access; 
     * <code>false</code> otherwise.
     */
    boolean isPackagePrivate();

    /**
     * Determines if this object mirrors a protected item. 
     * For {@link ArrayType}, the return value depends on the 
     * array component type. For primitive arrays the return value 
     * is always false. For object arrays, the return value is the
     * same as would be returned for the component type. 
     * For primitive classes, such as {@link java.lang.Integer#TYPE},
     * the return value is always false.
     *
     * @return <code>true</code> for items with private access; 
     * <code>false</code> otherwise.
     */
    boolean isProtected(); 

    /**
     * Determines if this object mirrors a public item. 
     * For {@link ArrayType}, the return value depends on the 
     * array component type. For primitive arrays the return value 
     * is always true. For object arrays, the return value is the
     * same as would be returned for the component type. 
     * For primitive classes, such as {@link java.lang.Integer#TYPE},
     * the return value is always true.
     *
     * @return <code>true</code> for items with public access; 
     * <code>false</code> otherwise.
     */
    boolean isPublic(); 
}
