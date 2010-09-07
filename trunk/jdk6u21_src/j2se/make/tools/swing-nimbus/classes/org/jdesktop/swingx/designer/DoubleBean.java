/*
 * @(#)DoubleBean.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer;

import org.jdesktop.beans.AbstractBean;

/**
 * DoubleBean - Simple bean for a observable double value
 *
 * @author Created by Jasper Potts (May 25, 2007)
 * @version 1.0
 */
public class DoubleBean extends AbstractBean {
    private double value = 0;

    public DoubleBean() {}

    public DoubleBean(double value) {
        this.value = value;
    }

    public double getValue() {
        return value;
    }

    public void setValue(double value) {
        double old = this.value;
        this.value = value;
        firePropertyChange("value", old, this.value);
    }


    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        DoubleBean that = (DoubleBean) o;

        if (Double.compare(that.value, value) != 0) return false;

        return true;
    }

    public int hashCode() {
        long temp = value != +0.0d ? Double.doubleToLongBits(value) : 0L;
        return (int) (temp ^ (temp >>> 32));
    }
}
