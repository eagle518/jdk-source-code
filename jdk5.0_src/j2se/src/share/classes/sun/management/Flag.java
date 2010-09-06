/*
 * @(#)Flag.java	1.4 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.management;

public abstract class Flag implements java.io.Serializable {
    private String name;
    private Object value;
    private boolean writeable;
    private FlagSource source;

    // Couldn't use enum until javac can support non-ascii character
    // or JMX source removes the French copyright chararcter
    // static enum FlagSource { vmDefault, other}

    static class FlagSource implements java.io.Serializable {
        String name;
        int ordinal;

        FlagSource(String name, int ordinal) {
            this.name = name;
            this.ordinal = ordinal;
        }
        public final static FlagSource vmDefault = 
            new FlagSource("vmDefault", 1);
        public final static FlagSource other = 
            new FlagSource("other", 1);
        public String toString() {
            return name;
        }
    }
        
    Flag(String name, Object value, boolean writeable, FlagSource source) {
        this.name = name;
        this.value = value;
        this.writeable = writeable;
        this.source = source;
    }

    /**
     * Returns the name of this flag.
     *
     * @return the name of this flag.
     */
    public String getName() {
        return name;
    }

    /**
     * Returns the current value of this flag.
     *
     * @return an <tt>Object</tt> representing the current value of this flag.
     */
    public Object getValue() {
        return value;
    }

    /**
     * Tests if this flag is writeable.  If this flag is writeable,
     * it can be set by {@link HotspotRuntimeMBean#setFlag} method.
     *
     * @return <tt>true</tt> if this flag is writeable; <tt>false</tt>
     * otherwise.
     */
    public boolean isWriteable() {
        return writeable;
    }

    /**
     * Returns the source by which this flag was last set.
     *
     * @return the source by which this flag was last set.
     */
    public String getSource() {
        return source.toString();
    }

    private static final long serialVersionUID = 6992337162326171013L;
}
