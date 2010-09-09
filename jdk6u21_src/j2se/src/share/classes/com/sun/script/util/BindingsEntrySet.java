/*
 * @(#)BindingsEntrySet.java	1.4 10/03/23 18:06:03
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTAIL. Use is subject to license terms.
 */

package com.sun.script.util;
import java.util.*;

/**
 * Entry set implementation for Bindings implementations
 *
 * @version 1.0
 * @author Mike Grogan
 * @since 1.6
 */
public class BindingsEntrySet extends AbstractSet<Map.Entry<String, Object>> {
    
    private BindingsBase base;
    private String[] keys;
    
    public BindingsEntrySet(BindingsBase base) {
        this.base = base;
        keys = base.getNames();
    }
    
    public int size() {
        return keys.length;
    }
    
    public Iterator<Map.Entry<String, Object>> iterator() {
        return new BindingsIterator();
    }
    
    public class BindingsEntry implements Map.Entry<String, Object> {
        private String key;
        public BindingsEntry(String key) {
            this.key = key;
        }
        
        public Object setValue(Object value) {
            throw new UnsupportedOperationException();
        }
        
        public String getKey() {
            return key;
        }
        
        public Object getValue() {
            return base.get(key);
        }
        
    }
    
    public class BindingsIterator implements Iterator<Map.Entry<String, Object>> {
        
        private int current = 0;
        private boolean stale = false;
        
        public boolean hasNext() {
            return (current < keys.length);
        }
       
        public BindingsEntry next() {
            stale = false;
            return new BindingsEntry(keys[current++]);
        }
        
        public void remove() {
            if (stale || current == 0) {
                throw new IllegalStateException();
            }
            
            stale = true;
            base.remove(keys[current - 1]);
        }
        
    }
    
}
