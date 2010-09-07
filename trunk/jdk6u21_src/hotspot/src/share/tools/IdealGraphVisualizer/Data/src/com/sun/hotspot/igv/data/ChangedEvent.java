/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
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
 */
package com.sun.hotspot.igv.data;

/**
 *
 * @author Thomas Wuerthinger
 */
public class ChangedEvent<T> extends Event<ChangedListener<T>> {

    private T object;

    public ChangedEvent() {
    }

    public ChangedEvent(T object) {
        this.object = object;
    }

    protected void fire(ChangedListener<T> l) {
        l.changed(object);
    }
}
