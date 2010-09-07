/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association.utility;


import com.sun.deploy.association.Association;
import com.sun.deploy.association.AssociationAlreadyRegisteredException;
import com.sun.deploy.association.AssociationNotRegisteredException;
import com.sun.deploy.association.RegisterFailedException;


/**
 * Containing funtions to modify the association information
 * 
 * @version 1.0
 */
public interface AppAssociationWriter {

    /** 
     * Constants for the registration/unregistration level.
     */
    public final static int USER_LEVEL = AppConstants.USER_LEVEL;
    public final static int SYSTEM_LEVEL = AppConstants.SYSTEM_LEVEL;
    public final static int DEFAULT_LEVEL = AppConstants.DEFAULT_LEVEL;

    /**
     * Checks whether the given assocation is valid for registration according to 
     * platform-specific logic.
     * 
     * @param assoc a given Association object.
     * @throws IllegalArgumentException if the given association is not valid for registration. 
     */
    public void checkAssociationValidForRegistration(Association assoc) throws IllegalArgumentException;  

    /**
     * Checks whether the given assocation is valid for unregistration according to 
     * platform-specific logic.
     * 
     * @param assoc a given Association object.
     * @throws IllegalArgumentException if the given association is not valid for unregistration.
     */
    public void checkAssociationValidForUnregistration(Association assoc) throws IllegalArgumentException;
    
    /**
     * Checks whether the given assocation exists in the system
     * 
     * @param assoc a given Association object.
     * @param level a given MIME database level.
     * @return true if the given Association already exists in the specified MIME database.
     */
    public boolean isAssociationExist(Association assoc, int level);

    /**
     * Registers the given association within specified level. 
     * 
     * @param assoc a given Association object.
     * @param level a given registration level
     * @throws AssociationAlreadyRegisteredException if the given association has
     * been registered in the system.
     * @throws RegisterFailedException if the given association fails to be registered.
     */
    public void registerAssociation(Association assoc, int level) 
            throws AssociationAlreadyRegisteredException, RegisterFailedException;

    /**
     * Unregisters the given association in specified level.
     * 
     * @param association a given Association object.
     * @param level a given registration level
     * @throws AssociationNotRegisteredException if the given association has not been 
     * registered before.
     * @throws RegisterFailedException if the given association fails to be unregistered.   
     */
    public void unregisterAssociation(Association assoc, int level) 
            throws AssociationNotRegisteredException, RegisterFailedException;
}
