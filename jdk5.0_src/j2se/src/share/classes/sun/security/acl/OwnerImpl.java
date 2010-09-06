/*
 * @(#)OwnerImpl.java	1.13 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.security.acl;

import java.util.*;
import java.security.*;
import java.security.acl.*;

/**
 * Class implementing the Owner interface. The
 * initial owner principal is configured as 
 * part of the constructor.
 * @author 	Satish Dharmaraj
 */
public class OwnerImpl implements Owner {
    private Group ownerGroup;

    public OwnerImpl(Principal owner) {
	ownerGroup = new GroupImpl("AclOwners");
	ownerGroup.addMember(owner);
    }

    /**
     * Adds an owner. Owners can modify ACL contents and can disassociate 
     * ACLs from the objects they protect in the AclConfig interface.
     * The caller principal must be a part of the owners list of the ACL in 
     * order to invoke this method. The initial owner is configured
     * at ACL construction time. 
     * @param caller the principal who is invoking this method. 
     * @param owner The owner that should be added to the owners list.
     * @return true if success, false if already an owner.
     * @exception NotOwnerException if the caller principal is not on 
     * the owners list of the Acl.
     */
    public synchronized boolean addOwner(Principal caller, Principal owner)
      throws NotOwnerException
    {
	if (!isOwner(caller))
	    throw new NotOwnerException();

	ownerGroup.addMember(owner);
	return false;
    }

    /** 
     * Delete owner. If this is the last owner in the ACL, an exception is 
     * raised.
     * The caller principal must be a part of the owners list of the ACL in 
     * order to invoke this method. 
     * @param caller the principal who is invoking this method. 
     * @param owner The owner to be removed from the owners list.
     * @return true if the owner is removed, false if the owner is not part 
     * of the owners list.
     * @exception NotOwnerException if the caller principal is not on 
     * the owners list of the Acl.
     * @exception LastOwnerException if there is only one owner left in the group, then
     * deleteOwner would leave the ACL owner-less. This exception is raised in such a case.
     */
    public synchronized boolean deleteOwner(Principal caller, Principal owner) 
      throws NotOwnerException, LastOwnerException
    {
	if (!isOwner(caller))
	    throw new NotOwnerException();
	
	Enumeration e = ownerGroup.members();
	//
	// check if there is atleast 2 members left.
	//
	Object o = e.nextElement();
	if (e.hasMoreElements()) 
	    return ownerGroup.removeMember(owner);
	else
	    throw new LastOwnerException();
	    
    }   

    /**
     * returns if the given principal belongs to the owner list.
     * @param owner The owner to check if part of the owners list
     * @return true if the passed principal is in the owner list, false if not.
     */
    public synchronized boolean isOwner(Principal owner) {
	return ownerGroup.isMember(owner);
    }
}
