/*
 * @(#)Zone.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.javazic;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.StringTokenizer;

/**
 * Zone holds information corresponding to a "Zone" part of a time
 * zone definition file.
 *
 * @since 1.4
 */
class Zone {
    // zone name (e.g., "America/Los_Angeles")
    private String name;

    // zone records
    private ArrayList list;

    // target zone names for this compilation
    private static HashSet targetZones;

    /**
     * Constructs a Zone with the specified zone name.
     * @param name the zone name
     */
    Zone(String name) {
	this.name = name;
	list = new ArrayList();
    }

    /**
     * Reads time zone names to be generated, called "target zone
     * name", from the specified text file and creats an internal hash
     * table to keep those names. It's assumed that one text line
     * contains a zone name or comments if it starts with
     * '#'. Comments can't follow a zone name in a single line.
     * @param fileName the text file name
     */
    static void readZoneNames(String fileName) {
	if (fileName == null) {
	    return;
	}
	BufferedReader in = null;
	try {
	    FileReader fr = new FileReader(fileName);
	    in = new BufferedReader(fr);
	} catch (FileNotFoundException e) {
	    Main.panic("can't open file: " + fileName);
	}
	targetZones = new HashSet();
	String line;

	try {
	    while ((line = in.readLine()) != null) {
		line = line.trim();
		if (line.length() == 0 || line.charAt(0) == '#') {
		    continue;
		}
		if (!targetZones.add(line)) {
		    Main.warning("duplicated target zone name: " + line);
		}
	    }
	    in.close();
	} catch (IOException e) {
	    Main.panic("IO error: "+e.getMessage());
	}
    }

    /**
     * Determines whether the specified zone is one of the target zones.
     * If no target zones are specified, this method always returns
     * true for any zone name.
     * @param zoneName the zone name
     * @return true if the specified name is a target zone.
     */
    static boolean isTargetZone(String zoneName) {
	if (targetZones == null) {
	    return true;
	}
	return targetZones.contains(zoneName);
    }

    /**
     * Forces to add "MET" to the target zone table. This is because
     * there is a conflict between Java zone name "WET" and Olson zone
     * name.
     */
    static void addMET() {
	if (targetZones != null) {
	    targetZones.add("MET");
	}
    }

    /**
     * @return the zone name
     */
    String getName() {
	return name;
    }

    /**
     * Adds the specified zone record to the zone record list.
     */
    void add(ZoneRec rec) {
	list.add(rec);
    }

    /**
     * @param index the index at which the zone record in the list is returned.
     * @return the zone record specified by the index.
     */
    ZoneRec get(int index) {
	return (ZoneRec) list.get(index);
    }

    /**
     * @return the size of the zone record list
     */
    int size() {
	return list.size();
    }

    /**
     * Resolves the reference to a rule in each zone record.
     * @param zi the Zoneinfo object with which the rule reference is
     * resolved.
     */
    void resolve(Zoneinfo zi) {
	for (int i = 0; i < list.size(); i++) {
	    ZoneRec rec = (ZoneRec) list.get(i);
	    rec.resolve(zi);
	}
    }
}
