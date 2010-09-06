/*
 * @(#)OptionLister.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import java.util.*;
import java.net.*;
import java.io.*;

/**
 * A class for listing the available options in the jstat_options file.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class OptionLister {
    private static final boolean debug = false;
    private URL[] sources;

    public OptionLister(URL[] sources) {
        this.sources = sources;
    }

    public void print(PrintStream ps) {
        Comparator c = new Comparator() {
               public int compare(Object o1, Object o2) {
                   OptionFormat of1 = (OptionFormat)o1;
                   OptionFormat of2 = (OptionFormat)o2;
                   return (of1.getName().compareTo(of2.getName()));
               }
        };

        Set options = new TreeSet(c);

        for (int i = 0; i < sources.length; i++) {
            try {
                URL u = sources[i];
                Reader r = new BufferedReader(
                        new InputStreamReader(u.openStream()));
                Set s = new Parser(r).parseOptions();
                options.addAll(s);
            } catch (IOException e) {
                if (debug) {
                    System.err.println(e.getMessage());
                    e.printStackTrace();
                }
            } catch (ParserException e) {
                // Exception in parsing the options file.
                System.err.println(sources[i] + ": " + e.getMessage());
                System.err.println("Parsing of " + sources[i] + " aborted");
            }
        }

        for (Iterator i = options.iterator(); i.hasNext(); /* empty */) {
            OptionFormat of = (OptionFormat)i.next();
            if (of.getName().compareTo("timestamp") == 0) {
              // ignore the special timestamp OptionFormat.
              continue;
            }
            ps.println("-" + of.getName());
        }
    }
}
