/*
 * @(#)UIAction.java	1.3 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.swing;

import java.beans.PropertyChangeListener;

import javax.swing.Action;

/**
 * UIAction is the basis of all of basic's action classes that are used in
 * an ActionMap. Subclasses need to override <code>actionPerformed</code>.
 * <p>
 * A typical subclass will look like:
 * <pre>
 *    private static class Actions extends UIAction {
 *        Actions(String name) {
 *            super(name);
 *        }
 *
 *        public void actionPerformed(ActionEvent ae) {
 *            if (getName() == "selectAll") {
 *                selectAll();
 *            }
 *            else if (getName() == "cancelEditing") {
 *                cancelEditing();
 *            }
 *        }
 *    }
 * </pre>
 * <p>
 * Subclasses that wish to conditionalize the enabled state should override
 * <code>isEnabled(Component)</code>, and be aware that the passed in
 * <code>Component</code> may be null.
 *
 * @see com.sun.java.swing.ExtendedAction
 * @see javax.swing.Action
 * @version 1.3 12/19/03
 * @author Scott Violet
 */
public abstract class UIAction implements Action {
    private String name;

    public UIAction(String name) {
        this.name = name;
    }

    public final String getName() {
        return name;
    }

    public Object getValue(String key) {
        if (key == NAME) {
            return name;
        }
        return null;
    }

    // UIAction is not mutable, this does nothing.
    public void putValue(String key, Object value) {
    }

    // UIAction is not mutable, this does nothing.
    public void setEnabled(boolean b) {
    }

    /**
     * Cover method for <code>isEnabled(null)</code>.
     */
    public final boolean isEnabled() {
        return isEnabled(null);
    }

    /**
     * Subclasses that need to conditionalize the enabled state should
     * override this. Be aware that <code>sender</code> may be null.
     *
     * @param sender Widget enabled state is being asked for, may be null.
     */
    public boolean isEnabled(Object sender) {
        return true;
    }

    // UIAction is not mutable, this does nothing.
    public void addPropertyChangeListener(PropertyChangeListener listener) {
    }

    // UIAction is not mutable, this does nothing.
    public void removePropertyChangeListener(PropertyChangeListener listener) {
    }
}
