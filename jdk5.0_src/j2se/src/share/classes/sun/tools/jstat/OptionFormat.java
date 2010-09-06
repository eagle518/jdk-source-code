/*
 * @(#)OptionFormat.java	1.1 04/02/23
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jstat;

import java.util.*;
import sun.jvmstat.monitor.MonitorException;

/**
 * A class for describing the output format specified by a command
 * line option that was parsed from an option description file.
 *
 * @author Brian Doherty
 * @version 1.1, 02/23/04
 * @since 1.5
 */
public class OptionFormat {
    protected String name;
    protected List children;

    public OptionFormat(String name) {
        this.name = name;
        this.children = new ArrayList();
    }

    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof OptionFormat)) {
            return false;
        }
        OptionFormat of = (OptionFormat)o;
        return (this.name.compareTo(of.name) == 0);
    }

    public int hashCode() {
      return name.hashCode();
    }

    public void addSubFormat(OptionFormat f) {
        children.add(f);
    }

    public OptionFormat getSubFormat(int index) {
        return (OptionFormat)children.get(index);
    }

    public void insertSubFormat(int index, OptionFormat f) {
        children.add(index, f);
    }

    public String getName() {
        return name;
    }

    public void apply(Closure c) throws MonitorException {

      for (Iterator i = children.iterator(); i.hasNext(); /* empty */) {
          OptionFormat o = (OptionFormat)i.next();
          c.visit(o, i.hasNext());
      }

      for (Iterator i = children.iterator(); i.hasNext(); /* empty */) {
          OptionFormat o = (OptionFormat)i.next();
          o.apply(c);
      }
    }

    public void printFormat() {
        printFormat(0);
    }

    public void printFormat(int indentLevel) {
        String indentAmount = "  ";
        StringBuilder indent = new StringBuilder("");

        for (int j = 0; j < indentLevel; j++) {
            indent.append(indentAmount);
        }
        System.out.println(indent + name + " {");

        // iterate over all children and call their printFormat() methods
        for (Iterator i = children.iterator(); i.hasNext(); /* empty */) {
            OptionFormat of = (OptionFormat)i.next();
            of.printFormat(indentLevel+1);
        }
        System.out.println(indent + "}");
    }
}
