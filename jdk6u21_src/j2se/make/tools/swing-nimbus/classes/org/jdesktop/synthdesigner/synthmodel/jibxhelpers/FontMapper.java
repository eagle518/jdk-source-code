/*
 * @(#)FontMapper.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.synthdesigner.synthmodel.jibxhelpers;

import org.jibx.runtime.IMarshaller;
import org.jibx.runtime.IMarshallingContext;
import org.jibx.runtime.IUnmarshaller;
import org.jibx.runtime.IUnmarshallingContext;
import org.jibx.runtime.JiBXException;
import org.jibx.runtime.impl.MarshallingContext;
import org.jibx.runtime.impl.UnmarshallingContext;

import java.awt.Font;

/**
 * FontMapper
 *
 * @author Created by Jasper Potts (Jun 8, 2007)
 * @version 1.0
 */
public class FontMapper implements IMarshaller, IUnmarshaller {
    private static final String ELEMENT_NAME = "font";
    private static final String FAMILY_NAME = "family";
    private static final String STYLE_NAME = "style";
    private static final String SIZE_NAME = "size";

    public boolean isExtension(int i) {
        return false;
    }

    public boolean isPresent(IUnmarshallingContext iUnmarshallingContext) throws JiBXException {
        return iUnmarshallingContext.isAt(null, ELEMENT_NAME);
    }

    public void marshal(Object object, IMarshallingContext iMarshallingContext) throws JiBXException {
        if (!(object instanceof Font)) {
            throw new JiBXException("Invalid object type for marshaller");
        } else if (!(iMarshallingContext instanceof MarshallingContext)) {
            throw new JiBXException("Invalid object type for marshaller");
        } else {
            MarshallingContext ctx = (MarshallingContext) iMarshallingContext;
            Font font = (Font) object;
            ctx.startTagAttributes(0, ELEMENT_NAME).
                    attribute(0, FAMILY_NAME, font.getFamily()).
                    attribute(0, STYLE_NAME, font.getStyle()).
                    attribute(0, SIZE_NAME, font.getSize()).
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
        String name = ctx.attributeText(null, FAMILY_NAME, null);
        int style = ctx.attributeInt(null, STYLE_NAME, 0);
        int size = ctx.attributeInt(null, SIZE_NAME, 0);
        ctx.parsePastEndTag(null, ELEMENT_NAME);
        // create
        return new Font(name, style, size);
    }
}
