/*
 * @(#)GSSNameSpi.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.jgss.spi;

import org.ietf.jgss.*;
import java.security.Provider;
 
/**
 * This interface is implemented by a mechanism specific name element. A
 * GSSName is conceptually a container class of several name elements from
 * different mechanisms.
 *
 * @author Mayank Upadhyay
 * @version 1.5, 12/19/03
 */

public interface GSSNameSpi {

    public Provider getProvider();

    /**
     * Equals method for the GSSNameSpi objects.
     * If either name denotes an anonymous principal, the call should
     * return false.
     *
     * @param name to be compared with
     * @returns true if they both refer to the same entity, else false
     * @exception GSSException with major codes of BAD_NAMETYPE,
     *    BAD_NAME, FAILURE
     */
    public boolean equals(GSSNameSpi name) throws GSSException;
  

    /**
     * Returns a flat name representation for this object. The name
     * format is defined in RFC 2078.
     *
     * @return the flat name representation for this object
     * @exception GSSException with major codes NAME_NOT_MN, BAD_NAME,
     *    BAD_NAME, FAILURE.   
     */
    public byte[] export() throws GSSException;


    /**
     * Get the mechanism type that this NameElement corresponds to.
     *
     * @return the Oid of the mechanism type
     */
    public Oid getMechanism();

    /**
     * Returns a string representation for this name. The printed
     * name type can be obtained by calling getStringNameType().
     *
     * @return string form of this name
     * @see #getStringNameType()
     * @overrides Object#toString
     */
    public String toString();
    

    /**
     * Returns the oid describing the format of the printable name.
     *
     * @return the Oid for the format of the printed name
     */
    public Oid getStringNameType();
  
    /**
     * Indicates if this name object represents an Anonymous name.
     */
    public boolean isAnonymousName();
}
