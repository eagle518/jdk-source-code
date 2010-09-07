/*
 * @(#)AmbientProperty.java	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.com;

public interface AmbientProperty {
    public void setBackground(int r, int g, int b);
    public void setForeground(int r, int g, int b);
    public void setFont(String name, int style, int size);

    public int getBackground();
    public int getForeground();
    public java.awt.Font getFont();
}
