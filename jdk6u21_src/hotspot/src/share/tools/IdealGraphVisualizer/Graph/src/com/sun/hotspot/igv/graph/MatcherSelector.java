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
package com.sun.hotspot.igv.graph;

import com.sun.hotspot.igv.data.Properties;
import com.sun.hotspot.igv.data.Properties.PropertyMatcher;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public class MatcherSelector implements Selector {

    private PropertyMatcher matcher;

    public MatcherSelector(PropertyMatcher matcher) {
        this.matcher = matcher;
    }

    public List<Figure> selected(Diagram d) {
        Properties.PropertySelector<Figure> selector = new Properties.PropertySelector<Figure>(d.getFigures());
        List<Figure> list = selector.selectMultiple(matcher);
        return list;
    }
}
