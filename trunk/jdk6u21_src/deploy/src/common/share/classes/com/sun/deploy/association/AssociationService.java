/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association;


import java.net.URL;
import java.util.Iterator;
import java.util.List;

import com.sun.deploy.association.utility.AppAssociationWriter;
import com.sun.deploy.association.utility.AppAssociationWriterFactory;
import com.sun.deploy.association.utility.AppAssociationReader;
import com.sun.deploy.association.utility.AppAssociationReaderFactory;
import com.sun.deploy.association.utility.AppUtility;


/**
 * This class provides relevant services for association.
 * <PRE>
 * 1. It provides the following operations: 
 *    1.1 Retrieve the association information for a specified file type.
 *    1.2 Register a specified association for specific user. The association information woulc 
 *        be available for this user (with OS support).
 *    1.3 Register a specified association system wide. The association information would 
 *        be available for every user (with OS support).
 *    1.4 Unregister a specified association for specific user. The association for this user 
 *        will be removed (with OS support).
 *    1.5 Unregister a specified association system wide. The association will not be available
 *        for every user (with OS support).
 * 
 * 2. On <B>Windows</B>:
 *    The Association class works as an abstraction for Windows Registry file type information, include:
 *      Description about this associatioin
 *      Mime type for this association
 *      Relevant file extension
 *      Relevant icon file name
 *      Relevant actions (operations)
 *
 * 3. On <B>Gnome desktop</B>:
 *    The Association class works as an abstraction for GnomeVFS file type information, include:
 *      Description about this associatioin
 *      Mime type for this association
 *      Relevant file extensions
 *      Relevant icon file name
 *      Relevant actions (operations)
 *      Corresponding registration file name
 *    
 * 4. The registeration/unregistration operations would report exceptions in the following cases:
 *    4.1 Try to register an already existed association
 *    4.2 Try to unregister a not existed associatioin
 *    4.3 Fail to register or unregister an association
 * </PRE>
 *
 * @version 1.0
 */
public class AssociationService {
    // A platform-dependent instance of AppAssociationReader.
    private AppAssociationReader appAssocReader;
    // A platform-dependent instance of AppAssociationWriter.
    private AppAssociationWriter appAssocWriter;
  
    public AssociationService() {
        appAssocReader = AppAssociationReaderFactory.newInstance();
        appAssocWriter = AppAssociationWriterFactory.newInstance();
    }
  
    /**
     * Returns the Association object containing the association info associated with the given 
     * mime type. If the given mime type is not registered in the system, null is returned.
     * <P>
     * <B>Notice:</B> A valid MIME type should be of the form class/type and contains no spaces. 
     * For example, "text/html".  
     *
     * @param mimeType a given mime type name.
     * @return the relevant association object or null if such mimeType not available.
     */
    public Association getMimeTypeAssociation(String mimeType) {
        if (mimeType == null) {
            throw new IllegalArgumentException("The specified mime type is null");
        }

        // Check whether the given mime type exists/is registered in the system.
        if (!appAssocReader.isMimeTypeExist(mimeType)) {
            return null;		
        }
        
        // Get the association associated with the mime type.
        Association assoc = new Association();
        List fileExtList = appAssocReader.getFileExtListByMimeType(mimeType);
        String iconFileName = appAssocReader.getIconFileNameByMimeType(mimeType);
        String description = appAssocReader.getDescriptionByMimeType(mimeType);
        List actionList = appAssocReader.getActionListByMimeType(mimeType);
      
        assoc.setMimeType(mimeType);

        if (fileExtList != null) {
            Iterator iter = fileExtList.iterator();

            if (iter != null) {
                while (iter.hasNext()) {
                    assoc.addFileExtension((String) iter.next());
                }
            }
        }
        
        if (iconFileName != null) {
            assoc.setIconFileName(iconFileName);
        }
        
        if (description != null) {
            assoc.setDescription(description);
        }
      
        if (actionList != null) {
            Iterator iter = actionList.iterator();

            if (iter != null) {
                while (iter.hasNext()) {
                    assoc.addAction((Action) iter.next());
                }
            }
        }
        
        return assoc;
    }        
  
    /**
     * Returns the Association object associated with the given file extension.
     * The file extension list in the returned association will contain only this given file extension.  
     * If the given file extension is not registered in the system, null is returned.
     *
     * @param fileExt a given file extension name.
     * @return the relevant association object, or null if such file extension not available.
     */
    public Association getFileExtensionAssociation(String fileExt) {
        if (fileExt == null) {
            throw new IllegalArgumentException("The specified file extension is null");
        }

        // Add the leading '.' character to the given file extension if not exists.    
        fileExt = AppUtility.addDotToFileExtension(fileExt);

        // Check whether the given file extension exists/is registered in the system.
        if (!appAssocReader.isFileExtExist(fileExt)) {
            return null;
        }
        
        // Get the association associated with the file extension.
        Association assoc = new Association();
        String mimeType = appAssocReader.getMimeTypeByFileExt(fileExt);        
        String iconFileName = appAssocReader.getIconFileNameByFileExt(fileExt);
        String description = appAssocReader.getDescriptionByFileExt(fileExt);
        List actionList = appAssocReader.getActionListByFileExt(fileExt);
      
        // Do not retrieve other file extensions.
        assoc.addFileExtension(fileExt);
        
        if (iconFileName != null) {
            assoc.setIconFileName(iconFileName);
        }
        
        if (mimeType != null) {
            assoc.setMimeType(mimeType);
        }
        
        if (description != null) {
            assoc.setDescription(description);
        }
      
        if (actionList != null) {
            Iterator iter = actionList.iterator();

            if (iter != null) {
                while (iter.hasNext()) {
                    assoc.addAction((Action) iter.next());
                }
            }
        }
        
        return assoc;
    }        
  
    /**
     * Returns the relevant association for the given url
     *     
     * @param url given url.
     * @return association for this URL.
     */
    public Association getAssociationByContent(URL url) {
        if (url == null) {
            throw new IllegalArgumentException("The specified URL is null");
        }
        
        Association assoc = null;
        String mimeType = appAssocReader.getMimeTypeByURL(url);

        if (mimeType != null) {
            // Get association by mime type.
            assoc = getMimeTypeAssociation(mimeType);
        }
        
        if (assoc == null) {
            // Get association by file extension.
            String fileExt = AppUtility.getFileExtensionByURL(url);

            if (fileExt != null) {
                assoc = getFileExtensionAssociation(fileExt);
            }
        }
            
        return assoc;
    }        
  
    /**
     * Registers the given association in user specific level.
     * <PRE>
     * On <B>Windows</B>:
     *     1. The file extension list and mime type can't all be null.
     *     2. If any of the fileds: description, iconFile, actionList is not null, 
     *        then fileExtensionList should not be empty.
     * 
     *     For <B>Windows NT, Windows Me/98/95</B>: the registration is always system wide, since all users 
     *         share the same association information.
     *     For <B>Windows 2000 and Windows XP</B>: the registration is only applied to this specific user.
     * 
     *     <B>Notice:</B>Only the file extension at the header of the file extension list will be registered on Windows platform.
     *
     * On <B>Gnome Desktop</B>:
     *     Both the name and mimeType fields need to be specified to perform this operation.
     * 
     *     Three MIME files(<name>.mime, <name>.keys and <name>.applications) would be created if they don't exist
     *     in the user specific MIME database. And the association info will be write to these MIME files.
     *
     *     The user needs writing permission to the user specific MIME database. Or else, 
     *     a RegisterFailerException is thrown. 
     * </PRE>
     * <P>
     * If the given association is invalid for the operation, an IllegalArgumentException is thrown.
     * 
     * @param assoc a given Association object.
     * @throws AssociationAlreadyRegisteredException if the given association already exists in the system.
     * @throws RegisterFailedException if the given association fails to be registered in the system.
     */
    public void registerUserAssociation(Association assoc) 
            throws AssociationAlreadyRegisteredException, RegisterFailedException {
        if (assoc == null) {
            throw new IllegalArgumentException("The specified association is null");
        }

        // Check whether the specified association is valid for registration.
        try {
            appAssocWriter.checkAssociationValidForRegistration(assoc); 
        } catch (IllegalArgumentException e) {
            throw e;
        }
        
        // Check whether the specified association already exists.
        if (appAssocWriter.isAssociationExist(assoc, AppAssociationWriter.USER_LEVEL)) {
            throw new AssociationAlreadyRegisteredException("Assocation already exists!");  
        }            

        // Perform registration.                
        appAssocWriter.registerAssociation(assoc, AppAssociationWriter.USER_LEVEL);
    }                

    /**
     * Unregisters the given association in user specific level.
     * <PRE>
     * On <B>Windows</B>:
     *     Either the mimeType or fileExtensionList field needs to be specified to perform this operation.
     * 
     *     For <B>Windows NT, Windows Me/98/95</B>: the unregistration is always system wide, since all users 
     *         share the same association information.
     *     For <B>Windows 2000 and Windows XP</B>: the unregistration is only applied to this specific user.
     *
     * On <B>Gnome Desktop</B>:
     *     Only the name field needs to be specified to perform this operation.
     * 
     *     Three MIME files(<name>.mime, <name>.keys and <name>.applications) would be removed, and all the association 
     *     in these MIME files is deleted from the user specific MIME database.
     *
     *     The user needs writing permission to the user specific MIME database. Or else, 
     *     a RegisterFailerException is thrown. 
     * </PRE>
     * <P>
     * If the given association is invalid for the operation, an IllegalArgumentException is thrown.
     * 
     * @param association a given Association object.
     * @throws AssociationNotRegisteredException if the given association doesn't exist in the system.
     * @throws RegisterFailedException if the given association fails to be registered in the system.   
     */
    public void unregisterUserAssociation(Association assoc) 
            throws AssociationNotRegisteredException, RegisterFailedException {
        if (assoc == null) {
            throw new IllegalArgumentException("The specified association is null");
        }

        // Check whether the specified association is valid for unregistration.
        try {
            appAssocWriter.checkAssociationValidForUnregistration(assoc);
        } catch (IllegalArgumentException e) {
            throw e;
        }
        
        // Check whether the specified association not exists.
        if (!appAssocWriter.isAssociationExist(assoc, AppAssociationWriter.USER_LEVEL)) {
            throw new AssociationNotRegisteredException("Assocation not exists!");  
        }            

        // Perform unregistration.
        appAssocWriter.unregisterAssociation(assoc, AppAssociationWriter.USER_LEVEL);
    }

    /**
     * Registers the given association in system level. 
     * <PRE>
     * On <B>Windows</B>:
     *     1. The file extension list and mime type can't all be null.
     *     2. If any of the fileds: description, iconFile, actionList is not null, 
     *        then fileExtensionList should not be empty.
     * 
     *     For <B>Windows XP</B>: the user needs the administrator permission to access the system association
     *         information in the Registry.
     *
     *     On <B>Gnome Desktop</B>:
     *     Both the name and mimeType fields need to be specified to perform this operation.
     * 
     *     Three MIME files(<name>.mime, <name>.keys and <name>.applications) woulc be created if they don't exist
     *     in the user specific MIME database. And the association info will be write to these MIME files.
     *
     *     The user needs writing permission to the user specific MIME database. Or else, 
     *     a RegisterFailerException is thrown. 
     * 
     *     <B>Notice:</B>Only the file extension at the header of the file extension list will be registered on Windows platform.
     * </PRE>
     * <P>
     * If the given association is invalid for the operation, an IllegalArgumentException is thrown.
     * 
     * @param assoc a given Association object.
     * @throws AssociationAlreadyRegisteredException if the given association already exists in the system.
     * @throws RegisterFailedException if the given association fails to be registered in the system.
     */
    public void registerSystemAssociation(Association assoc) 
            throws AssociationAlreadyRegisteredException, RegisterFailedException {
        if (assoc == null) {
            throw new IllegalArgumentException("The specified association is null");
        }

        // Check whether the specified association is valid for registration.
        try {
            appAssocWriter.checkAssociationValidForRegistration(assoc);
    	} catch (IllegalArgumentException e) {
            throw e;
        }
        
        // Check whether the specified association already exists.
        if (appAssocWriter.isAssociationExist(assoc, AppAssociationWriter.SYSTEM_LEVEL)) {
            throw new AssociationAlreadyRegisteredException("Assocation already exists!");  
        }            

        // Perform registration.
        appAssocWriter.registerAssociation(assoc, AppAssociationWriter.SYSTEM_LEVEL);
    }

    /**
     * Unregisters the given association in system level.
     * <PRE>
     * On <B>Windows</B>:
     *     Either the mimeType or fileExtensionList field should be specified to perform this operation.
     *
     *     For <B>Windows XP</B>: the user needs the administrator permission to access the system association
     *         information in the Registry.
     *
     * On <B>Gnome Desktop</B>:
     *     Only the name field needs to be specified to perform this operation.
     * 
     *     Three MIME files(<name>.mime, <name>.keys and <name>.applications) would be removed, and all the association 
     *     in these MIME files is deleted from the user specific MIME database.
     *
     *     The user needs writing permission to the user specific MIME database. Or else, 
     *     a RegisterFailerException is thrown. 
     * </PRE>
     * <P>
     * If the given association is invalid for the operation, an IllegalArgumentException is thrown.
     * 
     * @param association a given Association object.
     * @throws AssociationNotRegisteredException if the given association doesn't exist in the system.
     * @throws RegisterFailedException if the given association fails to be registered in the system.   
     */
    public void unregisterSystemAssociation(Association assoc) 
            throws AssociationNotRegisteredException, RegisterFailedException {
        if (assoc == null) {
            throw new IllegalArgumentException("The specified association is null");
        }

        // Check whether the specified association is valid for unregistration.
        try {
            appAssocWriter.checkAssociationValidForUnregistration(assoc); 
        } catch (IllegalArgumentException e) {
        	throw e;
        }
        
        // Check whether the specified association not exists.
        if (!appAssocWriter.isAssociationExist(assoc, AppAssociationWriter.SYSTEM_LEVEL)) {
            throw new AssociationNotRegisteredException("Assocation not existed!");  
        }            

        appAssocWriter.unregisterAssociation(assoc, AppAssociationWriter.SYSTEM_LEVEL);
    }

    public boolean hasAssociation(Association assoc) {
        return appAssocWriter.isAssociationExist(assoc,
                                          AppAssociationWriter.SYSTEM_LEVEL) ||
               appAssocWriter.isAssociationExist(assoc,
                                          AppAssociationWriter.USER_LEVEL);
    }
}
