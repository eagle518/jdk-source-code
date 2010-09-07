/*
 * @(#)Property.java	1.3 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.util.Properties;

import com.sun.deploy.config.Config;

// A property spec, unique by it's key
// Handles all quoting at toString() and unquoting at construction
public class Property implements Cloneable {
    private static final boolean DEBUG = false;
    String key;
    String value;
    boolean isSecure;

    public Property(String spec) {
        spec=StringQuoteUtil.unquoteIfEnclosedInQuotes(spec);
        int startKey = spec.indexOf("-D");
        if(startKey<0 || startKey==spec.length()-2 ) {
            throw new IllegalArgumentException("Property invalid");
        }
        startKey+=2; // skip "-D"
        int endKey = spec.indexOf("=");
        if(endKey<0) {
            // it's legal to have no assignment
            this.key=spec.substring(startKey);
            this.value=new String("");
        } else {
            this.key=spec.substring(startKey, endKey);
            this.value=StringQuoteUtil.unquoteIfEnclosedInQuotes(spec.substring(endKey+1));
        }
        this.isSecure=Config.isSecureSystemPropertyKey(this.key);
        if(DEBUG) {
            System.out.println("new Property: isSecure: "+isSecure+", spec: <"+spec+"> -> <"+this.key+"> <"+this.value+">");
        }
    }

    public static Property createProperty(String spec) {
        Property prop = null;
        try {
            prop = new Property(spec);
        } catch (IllegalArgumentException iae) {}
        return prop;
    }

    public Property(String key, String value) {
        this.key=key;
        if(value!=null) {
            this.value=StringQuoteUtil.unquoteIfEnclosedInQuotes(value);
        } else {
            this.value=new String("");
        }
        this.isSecure=Config.isSecureSystemPropertyKey(this.key);
        if(DEBUG) {
            System.out.println("new Property: isSecure: "+isSecure+", <"+key+"> <"+value+"> -> <"+this.key+"> <"+this.value+">");
        }
    }

    public String getKey() { return key; }
    public String getValue() { return value; }
    public boolean isSecure() { return isSecure; }

    // @return String representation, unquoted, unified presentation
    public String toString() {
        return toString(false);
    }

    // @arg osConform OS conform presentation, if true
    // @return String representation, unquoted
    // @see getQuotesWholePropertySpec
    public String toString(boolean osConform) {
        if(value.length()==0) {
            return "-D"+key;
        }
        if(osConform && _quoteWholePropertySpec) {
            return StringQuoteUtil.quoteIfNeeded("-D"+key+"="+value);
        }
        return "-D"+key+"="+StringQuoteUtil.quoteIfNeeded(value);
    }

    public void addTo(Properties props) {
        props.setProperty(key, value);
    }

    // CLONEABLE

    public Object clone()
    {
        return new Property(key, value);
    }

    // Hash Object 

    public boolean equals(Object o) {
        if ( !(o instanceof Property) ) {
            return false;
        }
        Property op = (Property)o;
        int hashTheirs = op.hashCode();
        int hashThis   = hashCode();
        return hashTheirs == hashThis;
    }

    public int hashCode() {
        return key.hashCode();
    }

    /** Depending on the OS (currently Windows only),
      * the quoting at toString() will be done over the whole 
      * spec (string representation).
      * Normally it is done over the r-value only (Unix).
      */
    public static final boolean getQuotesWholePropertySpec() {
        return _quoteWholePropertySpec;
    }

    private static final boolean _quoteWholePropertySpec;

    static {
        if ( Config.getOSName().startsWith("Win") ) {
            _quoteWholePropertySpec=true;
            if(DEBUG) {
                System.out.println("Property: Quoting will happen on the whole spec (win)");
            }
        } else {
            _quoteWholePropertySpec=false;
            if(DEBUG) {
                System.out.println("Property: Quoting will happen on the r-value only (Unix)");
            }
        }
    }
}

