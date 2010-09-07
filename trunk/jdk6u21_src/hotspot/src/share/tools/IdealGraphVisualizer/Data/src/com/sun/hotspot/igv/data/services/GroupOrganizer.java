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
package com.sun.hotspot.igv.data.services;

import com.sun.hotspot.igv.data.Group;
import com.sun.hotspot.igv.data.Pair;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public interface GroupOrganizer {

    public String getName();

    public List<Pair<String, List<Group>>> organize(List<String> subFolders, List<Group> groups);
}
