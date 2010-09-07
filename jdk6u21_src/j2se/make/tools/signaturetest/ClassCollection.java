/*
 * @(#)ClassCollection.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

/** This class represents table which can store Vector of entries
 *  for each String  key **/
public class ClassCollection {
    private Hashtable definitions;

    /** creates empty table **/
    public ClassCollection() {
        this.definitions = new Hashtable();
    }

    /** Adds new value to the Vector which mapped by key 
     *  @param entry this MemberEntry includes key and entry 
     *  which will be included **/
    public void addElement(MemberEntry entry) {
        addElement(entry.getKey(), entry.getEntry());
    }
    
    /** Adds new value to the Vector which mapped by key 
     *  @param key the key 
     *  @param def entry which will be included **/
    public void addElement(String key, Object def) {
        Vector h = (Vector)definitions.get(key);
        if (h == null) {
            h = new Vector();
            this.definitions.put(key, h);
        }
        h.addElement(def);
    }

    /** Adds unique new value to the Vector which mapped by key 
     *  If the entry is contained, than new empty will not be added  
     *  @param key the key 
     *  @param def entry which will be included **/
    public void addUniqueElement(String key, Object def) {
        Vector h = (Vector)definitions.get(key);
        if (h == null){
            h = new Vector();
            this.definitions.put(key, h);
        }
        if (!h.contains(def))
            h.addElement(def);
    }

    /** Returns enumeration of the keys **/
    public Enumeration keys() {
        return definitions.keys();
    }

    /** Returns Vectors of the entries which mapped for given key 
     *  @param key the key **/
    public Vector get(String key) {
        return (Vector)definitions.get(key);
    }

    /** put entry for given key. All entries which mapped by this key
     *  are removed 
     *  @param key the key 
     *  @param def the new entry which will be included instead all 
     *  previous values **/
    public void put(String key, Object def) {
        Vector h = new Vector();
        h.addElement(def);
        this.definitions.put(key, h);
    }

    /** put the Vector of the entries for given key. All entries which 
     *  mapped by this key are removed 
     *  @param key the key 
     *  @param def the new entry which will be included instead all 
     *  previous values **/
    public void putVector(String key, Vector member) {
        definitions.put(key, member);
    }

    /** removes entries for given key 
     *  @param key the key **/
    public void remove(String key) {
        definitions.remove(key);
    }

    /** removes entries for all keys **/
    public void clear() {
        definitions.clear();
    }
}
