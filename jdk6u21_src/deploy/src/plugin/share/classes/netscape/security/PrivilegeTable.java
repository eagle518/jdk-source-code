/*
 * @(#)PrivilegeTable.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;

import java.util.Enumeration;
import java.util.Hashtable;

/**
 * This class is used by PrivilegeManager and other security classes to
 * manage simple user-specified privileges. The purpose of a 
 * PrivilegeTable is to map Targets to Privileges. The interface emulates 
 * that of a Hashtable, with the exception that if you look up a Target 
 * that has not been stored in the table, the blank, forever Privilege is 
 * returned, rather than null. 
 * 
 * Also, while the normal use of a PrivilegeTable is to map Target to 
 * Privilege, you can really map anything to a Privilege. Thus, if you 
 * chose to map Principal to Privilege, you would have a traditional access 
 * control list (ACL). 
 *
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public class PrivilegeTable
{
    // private member
    Hashtable table = new Hashtable();
  
    /**
     * Create an empty PrivilegeTable. 
     */
    public PrivilegeTable()
    {
    }
    
    /** 
     * @return Number of entries in the PrivilegeTable 
     */
    public int size()
    {
	return table.size();
    }

    /**
     * @return true iff the PrivilegeTable has any entries. 
     */
    public boolean isEmpty()
    {
	return table.isEmpty();
    }


    /**
     * @return an Enumeration of the Targets in the PrivilegeTable 
     */
    public Enumeration keys()
    {
	return table.keys();
    }

    /**
     * @return an Enumeration of the Privileges in the PrivilegeTable 
     */
    public Enumeration elements()
    {
	return table.elements();
    }

    /** 
     * Retrieve a Privilege from the PrivilegeTable 
     *
     * @return Privilege associated with the obj key, or blank, forever 
     *	       if the key is not stored in the table 
     */
    public Privilege get(Object obj)
    {
	return (Privilege) table.get(obj);
    }

    /** 
     * Retrieve a Privilege from the PrivilegeTable 
     *
     * @return Privilege associated with the target argument, or blank, 
     *	       forever if the target is not stored in the table 
     */
    public Privilege get(Target t)
    {
	return get((Object)t);
    }

    /** 
     * Store a key / Privilege pair in the PrivilegeTable 
     *
     * @return the same privilege as the priv argument 
     */
    public Privilege put(Object key, Privilege priv)
    {
	return (Privilege) table.put(key, priv);
    }

    /** 
     * Store a Target / Privilege pair in the PrivilegeTable 
     *
     * @return the same privilege as the priv argument 
     */
    public Privilege put(Target key, Privilege priv)
    {
	return (Privilege) table.put(key, priv);
    }

    /** 
     * Remove the key / Privilege pair from the PrivilegeTable 
     *
     * @return the Privilege formerly stored with the argument 
     *	       key, or null if it wasn't there 
     */
    public Privilege remove(Object key)
    {
	return (Privilege) table.remove(key);
    }

    /** 
     * Remove the Target / Privilege pair from the PrivilegeTable 
     *
     * @return the Privilege formerly stored with the argument 
     *	       key, or null if it wasn't there 
     */
    public Privilege remove(Target key)
    {
	return (Privilege) table.remove(key);
    }

    public void clear()
    {
	table.clear();
    }
}
