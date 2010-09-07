/*
 * @(#)DimensionMapper.java	1.3 10/03/23
 *
 * Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package org.jdesktop.swingx.designer.jibxhelpers;

import org.jibx.runtime.IAliasable;
import org.jibx.runtime.IMarshaller;
import org.jibx.runtime.IMarshallingContext;
import org.jibx.runtime.IUnmarshaller;
import org.jibx.runtime.IUnmarshallingContext;
import org.jibx.runtime.JiBXException;
import org.jibx.runtime.impl.MarshallingContext;
import org.jibx.runtime.impl.UnmarshallingContext;

import java.awt.Dimension;

/**
 * DimensionMapper
 *
 * @author Created by Jasper Potts (Jun 12, 2007)
 * @version 1.0
 */
public class DimensionMapper implements IMarshaller, IUnmarshaller, IAliasable {
    private static final String ELEMENT_NAME = "dimension";
    private static final String WIDTH_NAME = "width";
    private static final String HEIGHT_NAME = "height";

    private String uri;
    private int index;
    private String name;

    public DimensionMapper() {
        uri = null;
        index = 0;
        name = ELEMENT_NAME;
    }

    public DimensionMapper(String uri, int index, String name) {
        this.uri = uri;
        this.index = index;
        this.name = name;
    }

    public boolean isExtension(int i) {
        return false;
    }

    public boolean isPresent(IUnmarshallingContext iUnmarshallingContext) throws JiBXException {
        return iUnmarshallingContext.isAt(uri, ELEMENT_NAME);
    }

    public void marshal(Object object, IMarshallingContext iMarshallingContext) throws JiBXException {
        if (!(object instanceof Dimension)) {
            throw new JiBXException("Invalid object type for marshaller");
        } else if (!(iMarshallingContext instanceof MarshallingContext)) {
            throw new JiBXException("Invalid object type for marshaller");
        } else {
            MarshallingContext ctx = (MarshallingContext) iMarshallingContext;
            Dimension dimension = (Dimension) object;
            ctx.startTagAttributes(index, name).
                    attribute(index, WIDTH_NAME, dimension.width).
                    attribute(index, HEIGHT_NAME, dimension.height).
                    closeStartEmpty();
        }
    }

    public Object unmarshal(Object object, IUnmarshallingContext iUnmarshallingContext) throws JiBXException {
        // make sure we're at the appropriate start tag
        UnmarshallingContext ctx = (UnmarshallingContext) iUnmarshallingContext;
        if (!ctx.isAt(uri, name)) {
            ctx.throwStartTagNameError(uri, name);
        }
        // get values
        int width = ctx.attributeInt(uri, WIDTH_NAME, index);
        int height = ctx.attributeInt(uri, HEIGHT_NAME, index);
        // state finished parsing
        ctx.parsePastEndTag(uri, name);
        // create
        return new Dimension(width, height);
    }
}
