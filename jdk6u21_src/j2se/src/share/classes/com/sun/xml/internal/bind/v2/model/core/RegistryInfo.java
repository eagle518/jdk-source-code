/*
 * Copyright (c) 1997, 2008, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package com.sun.xml.internal.bind.v2.model.core;

import java.util.Set;

import javax.xml.bind.annotation.XmlRegistry;

import com.sun.xml.internal.bind.v2.model.impl.ModelBuilder;

/**
 * Represents the information in a class with {@link XmlRegistry} annotaion.
 *
 * <p>
 * This interface is only meant to be used as a return type from {@link ModelBuilder}.
 *
 * @author Kohsuke Kawaguchi
 */
public interface RegistryInfo<T,C> {
    /**
     * Returns all the references to other types in this registry.
     */
    Set<TypeInfo<T,C>> getReferences();

    /**
     * Returns the class with {@link XmlRegistry}.
     */
    C getClazz();
}
