/*
 * @(#)OptionFinder.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import java.util.*;
import java.net.*;
import java.io.*;

/**
 * A class for finding a specific special option in the jstat_options file.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class OptionFinder {

    private static final boolean debug = false;

    URL[] optionsSources;

    public OptionFinder(URL[] optionsSources) {
        this.optionsSources = optionsSources;
    }

    public OptionFormat getOptionFormat(String option, boolean useTimestamp) {
        OptionFormat of = getOptionFormat(option, optionsSources);
        OptionFormat tof = null;
        if ((of != null) && (useTimestamp)) {
            // prepend the timestamp column as first column
            tof = getOptionFormat("timestamp", optionsSources);
            if (tof != null) {
                ColumnFormat cf = (ColumnFormat)tof.getSubFormat(0);
                of.insertSubFormat(0, cf);
            }
        }
        return of;
    }

    protected OptionFormat getOptionFormat(String option, URL[] sources) {
        OptionFormat of = null;
        for (int i = 0; (i < sources.length) && (of == null); i++) {
            try {
                URL u = sources[i];
                Reader r = new BufferedReader(
                        new InputStreamReader(u.openStream()));
                of = new Parser(r).parse(option);
            } catch (IOException e) {
                if (debug) {
                    System.err.println("Error processing " + sources[i]
                                       + " : " + e.getMessage());
                    e.printStackTrace();
                }
            } catch (ParserException e) {
                // Exception in parsing the options file.
                System.err.println(sources[i] + ": " + e.getMessage());
                System.err.println("Parsing of " + sources[i] + " aborted");
            }
        }
        return of;
    }
}
