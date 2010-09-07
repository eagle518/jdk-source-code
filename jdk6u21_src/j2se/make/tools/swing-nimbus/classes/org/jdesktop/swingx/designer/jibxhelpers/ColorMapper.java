/*
 * @(#)ColorMapper.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.jibxhelpers;

import org.jibx.runtime.IMarshaller;
import org.jibx.runtime.IMarshallingContext;
import org.jibx.runtime.IUnmarshaller;
import org.jibx.runtime.IUnmarshallingContext;
import org.jibx.runtime.JiBXException;
import org.jibx.runtime.impl.MarshallingContext;
import org.jibx.runtime.impl.UnmarshallingContext;

import java.awt.Color;

/**
 * ColorMapper
 *
 * @author Created by Jasper Potts (Jun 8, 2007)
 * @version 1.0
 */
public class ColorMapper implements IMarshaller, IUnmarshaller {
    private static final String ELEMENT_NAME = "color";
    private static final String RED_NAME = "red";
    private static final String GREEN_NAME = "green";
    private static final String BLUE_NAME = "blue";
    private static final String ALPHA_NAME = "alpha";

    public boolean isExtension(int i) {
        return false;
    }

    public boolean isPresent(IUnmarshallingContext iUnmarshallingContext) throws JiBXException {
        return iUnmarshallingContext.isAt(null, ELEMENT_NAME);
    }

    public void marshal(Object object, IMarshallingContext iMarshallingContext) throws JiBXException {
        if (!(object instanceof Color)) {
            throw new JiBXException("Invalid object type for marshaller");
        } else if (!(iMarshallingContext instanceof MarshallingContext)) {
            throw new JiBXException("Invalid object type for marshaller");
        } else {
            MarshallingContext ctx = (MarshallingContext) iMarshallingContext;
            Color color = (Color) object;
            ctx.startTagAttributes(0, ELEMENT_NAME).
                    attribute(0, RED_NAME, color.getRed()).
                    attribute(0, GREEN_NAME, color.getGreen()).
                    attribute(0, BLUE_NAME, color.getBlue()).
                    attribute(0, ALPHA_NAME, color.getAlpha()).
                    closeStartEmpty();
        }
    }

    public Object unmarshal(Object object, IUnmarshallingContext iUnmarshallingContext) throws JiBXException {
        // make sure we're at the appropriate start tag
        UnmarshallingContext ctx = (UnmarshallingContext) iUnmarshallingContext;
        if (!ctx.isAt(null, ELEMENT_NAME)) {
            ctx.throwStartTagNameError(null, ELEMENT_NAME);
        }
        // get values
        int red = ctx.attributeInt(null, RED_NAME, 0);
        int green = ctx.attributeInt(null, GREEN_NAME, 0);
        int blue = ctx.attributeInt(null, BLUE_NAME, 0);
        int alpha = ctx.attributeInt(null, ALPHA_NAME, 0);
        ctx.parsePastEndTag(null, ELEMENT_NAME);
        // create
        return new Color(red, green, blue, alpha);
    }
}
