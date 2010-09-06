/*
 * @(#)InterfaceType.java	1.17 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jdi;

import java.util.List;

/**
 * A mirror of an interface in the target VM. An InterfaceType is 
 * a refinement of {@link ReferenceType} that applies to true interfaces
 * in the JLS  sense of the definition (not a class, not an array type). 
 * An interface type will never be returned by 
 * {@link ObjectReference#referenceType}, but it may be in the list
 * of implemented interfaces for a {@link ClassType} that is returned 
 * by that method.
 *
 * @see ObjectReference 
 *
 * @author Robert Field
 * @author Gordon Hirsch
 * @author James McIlree
 * @since  1.3
 */
public interface InterfaceType extends ReferenceType {
    /**
     * Gets the interfaces directly extended by this interface.
     * The returned list contains only those interfaces this
     * interface has declared to be extended.
     *
     * @return a List of {@link InterfaceType} objects each mirroring
     * an interface extended by this interface.
     * If none exist, returns a zero length List.
     * @throws ClassNotPreparedException if this class not yet been 
     * prepared.
     */
    List<InterfaceType> superinterfaces();

    /**
     * Gets the currently prepared interfaces which directly extend this
     * interface. The returned list contains only those interfaces that
     * declared this interface in their "extends" clause.
     *
     * @return a List of {@link InterfaceType} objects each mirroring
     * an interface extending this interface.
     * If none exist, returns a zero length List.
     */
    List<InterfaceType> subinterfaces();

    /**
     * Gets the currently prepared classes which directly implement this
     * interface. The returned list contains only those classes that
     * declared this interface in their "implements" clause.
     *
     * @return a List of {@link ClassType} objects each mirroring
     * a class implementing this interface.
     * If none exist, returns a zero length List.
     */
    List<ClassType> implementors();
}

