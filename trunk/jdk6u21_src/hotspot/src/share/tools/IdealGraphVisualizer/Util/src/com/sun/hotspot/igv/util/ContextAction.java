/*
 * Copyright (c) 1998, 2007, Oracle and/or its affiliates. All rights reserved.
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
package com.sun.hotspot.igv.util;

import java.awt.EventQueue;
import org.openide.util.ContextAwareAction;
import org.openide.util.Lookup;
import org.openide.util.LookupEvent;
import org.openide.util.LookupListener;
import org.openide.util.Utilities;
import org.openide.util.actions.CallableSystemAction;

/**
 *
 * @author Thomas Wuerthinger
 */
public abstract class ContextAction<T> extends CallableSystemAction implements LookupListener, ContextAwareAction {

    private Lookup context = null;
    private Lookup.Result<T> result = null;

    public ContextAction() {
        this(Utilities.actionsGlobalContext());
    }

    public ContextAction(Lookup context) {
        init(context);
    }

    private void init(Lookup context) {
        this.context = context;
        result = context.lookupResult(contextClass());
        result.addLookupListener(this);
        resultChanged(null);
    }

    public void resultChanged(LookupEvent e) {
        if (result.allItems().size() != 0) {
            update(result.allInstances().iterator().next());
        } else {
            update(null);
        }
    }

    @Override
    public void performAction() {
        final T t = result.allInstances().iterator().next();

        // Ensure it's AWT event thread
        EventQueue.invokeLater(new Runnable() {

            public void run() {
                performAction(t);
            }
        });
    }

    public void update(T t) {
        if (t == null) {
            setEnabled(false);
        } else {
            setEnabled(isEnabled(t));
        }
    }

    public boolean isEnabled(T context) {
        return true;
    }

    public abstract Class<T> contextClass();

    public abstract void performAction(T context);
}
