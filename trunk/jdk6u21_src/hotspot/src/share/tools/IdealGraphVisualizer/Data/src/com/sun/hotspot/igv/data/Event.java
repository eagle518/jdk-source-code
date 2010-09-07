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

import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public abstract class Event<L> {

    private List<L> listener;

    public Event() {
        listener = new ArrayList<L>();
    }

    public void addListener(L l) {
        listener.add(l);
    }

    public void removeListener(L l) {
        listener.remove(l);
    }

    public void fire() {
        List<L> tmpList = new ArrayList<L>(listener);
        for (L l : tmpList) {
            fire(l);
        }
    }

    protected abstract void fire(L l);
}
