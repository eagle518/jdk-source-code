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
package com.sun.hotspot.igv.coordinator;

import com.sun.hotspot.igv.data.Group;
import com.sun.hotspot.igv.data.services.GroupOrganizer;
import com.sun.hotspot.igv.data.Pair;
import java.util.ArrayList;
import java.util.List;

/**
 *
 * @author Thomas Wuerthinger
 */
public class StandardGroupOrganizer implements GroupOrganizer {

    public String getName() {
        return "-- None --";
    }

    public List<Pair<String, List<Group>>> organize(List<String> subFolders, List<Group> groups) {

        List<Pair<String, List<Group>>> result = new ArrayList<Pair<String, List<Group>>>();

        if (groups.size() == 1 && subFolders.size() > 0) {
            result.add(new Pair<String, List<Group>>("", groups));
        } else {
            for (Group g : groups) {
                List<Group> children = new ArrayList<Group>();
                children.add(g);
                Pair<String, List<Group>> p = new Pair<String, List<Group>>();
                p.setLeft(g.getName());
                p.setRight(children);
                result.add(p);
            }
        }

        return result;
    }
}
