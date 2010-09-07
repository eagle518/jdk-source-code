 /* @(#)NodeBorder.java	1.4 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;

import javax.swing.border.LineBorder;
import java.awt.Color;

/**
 *
 * Special border to be used for the nodes in the tree.
 */
public class NodeBorder extends LineBorder {
    
    public NodeBorder(Color c){
        super(c);
    }
    
    public NodeBorder(Color c, int thickness){
        super(c, thickness);
    }
    
    public NodeBorder(Color c, int thickness, boolean roundedCorners){
        super(c, thickness, roundedCorners);
    }

    public void setBorderColor(Color newColor){
        super.lineColor = newColor;
    }

}
