/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association;


import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.sun.deploy.association.utility.AppUtility;


/**
 * This class abstracts the cross-platform registration information for one file type. 
 * <P>
 * The associated information includes description, mime type, file extensions, icon file name, 
 * and the corresponding action list. 
 *
 * @version 1.0
 */
public class Association {
  
    /**
     * The name of the generated or removed mime files on Gnome while registering/unregistering
     * an association. 
     * <P> On Gnome, one association is stored in plain text files: *.mime, *.keys and *.applications.
     * With this field, the files to be generated or removed would be <name>.mime, <name>.keys and
     * <name>.applications. 
     * <P>
     * <B>Notice: </B>While registering/unregistering an association on Gnome, this field must be 
     * specified. But on Windows, it's unnecessary and never used.
     */
    private String name;

    /**
     * Description of the association.
     */
    private String description;
  
    /**
     * Mime type of the association.
     */
    private String mimeType;
  
    /**
     * File extension list of the association.
     * <P>
     * On <B>Windows</B>
     * Only the HEADER file extension in the file extension list will be effective
     * during registration
     * 
     */
    private List fileExtensionList;
  
    /**
     * Icon file name of the association.
     */
    private String iconFileName;
  
    /**
     * Action list of the association.
     */
    private List actionList;
    
    /**
     * Hashcode for this association 
     */
    private int hashcode;

    public Association() {
    }
  
    /**
     * Returns the value of the name field.
     * <P>
     * The name field is the name of the generated or removed mime files on Gnome 
     * while registering/unregistering an association. 
     * <P> 
     * On Gnome, one association is stored in plain text files: *.mime, *.keys and *.applications.
     * With this field, the files to be generated or removed would be 
     * <name>.mime, <name>.keys and <name>.applications. 
     * <P>
     * <B>Notice: </B>While registering/unregistering an association on Gnome, this field must be 
     * specified. But on Windows, it's unnecessary and never used.
     *
     * @return String
     */
    public String getName() {
        return name;
    }
  
    /**
     * Sets the value of the name field.
     * <P>
     * The name field is the name of the generated or removed mime files on Gnome 
     * while registering/unregistering an association. 
     * <P> 
     * On Gnome, one association is stored in plain text files: *.mime, *.keys and *.applications.
     * With this field, the files to be generated or removed would be 
     * <name>.mime, <name>.keys and <name>.applications.
     * <P>
     * <B>Notice: </B>While registering/unregistering an association on Gnome, this field must be 
     * specified. But on Windows, it's unnecessary and never used.
     * 
     * @param name a given name value.
     */
    public void setName(String name) {
    	if (name == null) {
            throw new IllegalArgumentException("The given mime file name is null.");
    	}
    	
        this.name = name;
    }

    /**
     * Returns the value of the description field.
     *
     * @return the description of this association.
     */
    public String getDescription() {
        return description;
    }
  
    /**
     * Sets the value of the description field.
     * 
     * @param description a given description value.
     */
    public void setDescription(String description) {
        if (description == null) {
            throw new IllegalArgumentException("The given description is null.");
        }

        this.description = description;
    }
  
    /**
     * Returns the value of the mime type field.
     *
     * @return the mime type.
     */
    public String getMimeType() {
        return mimeType;
    }
  
    /**
     * Sets the value of the mime type field.
     * <P>
     * <B>Notice:</B> A valid MIME type should be of the form class/type and contains no spaces. 
     * For example, "text/html".  
     * 
     * @param mimeType a given mime type.
     */
    public void setMimeType(String mimeType) {
        if (mimeType == null) {
            throw new IllegalArgumentException("The given MIME type is null.");
        }
        
        this.mimeType = mimeType;
    }
  
    /**
     * Adds one file extension to the file extension list. If the given file extension already exists in the file extension
     * list, it will return false, and no changes are made to the file extension list.
     * <P>
     * The specified file extension may have a leading '.' character or not.
     * If it has no leading '.' character, the '.' will be added automatically.
     * <P>
     * On <B>Windows</B>
     * Only the HEADER file extension will be effective during registration, i.e. the 
     * first added file extension will be the only one writing into registration table. 
     * 
     * @param fileExt a given file extension.
     * @return true if the file extension list is changed as a result of the call.
     */
    public boolean addFileExtension(String fileExt) {
        if (fileExt == null) {
            throw new IllegalArgumentException("The given file extension is null.");
        }
   	
        // Add the leading '.' character to the given file extension if not exists.    
        fileExt = AppUtility.addDotToFileExtension(fileExt);
        
        if (fileExtensionList == null) {
            fileExtensionList = new ArrayList();
        }
        
        return fileExtensionList.add(fileExt);
    }
  
    /**
     * Removes the given file extension from the file extension list. If the file extension is not contained 
     * in the file extension list, it will return false, no changes are made to the file extension list.
     * <P>
     * The specified file extension may have a leading '.' character or not.
     * If it has no leading '.' character, the '.' will be added automatically.
     * 
     * @param fileExt a given file extension.
     * @return true if the given file extension is contained the file extension list and removed.
     */
    public boolean removeFileExtension(String fileExt) {
        if (fileExt == null) {
            throw new IllegalArgumentException("The given file extension is null.");
        }
    	
        // Add the leading '.' character to the given file extension if not exists.
        fileExt = AppUtility.addDotToFileExtension(fileExt);        
        if (fileExtensionList != null) {
            return fileExtensionList.remove(fileExt);
        }
        
        return false;
    }
  
    /**
     * Returns the file extension list
     * 
     * @return file extension list of the association
     */
    public List getFileExtList() {
        // Make defensive copy
        if (fileExtensionList == null) {
            return null;
        } else {
            List retList = new ArrayList();
            
            Iterator iter = fileExtensionList.iterator();
            while (iter.hasNext()) {
                retList.add(iter.next());
            }
            
            return retList;
        }            
    }
  
    /**
     * Returns the value of the iconFileName field..
     *
     * @return icon file name for this association
     */
    public String getIconFileName() {
        return iconFileName;
    }
  
    /**
     * Sets the value of the iconFileName field.
     * <P>
     * <B>Notice:</B> On <B>Windows</B>
     * <P>
     * An association with icon file name set would be valid for registration only if the
     * file extension list is not empty.
     * 
     * @param fileName a given icon file name.
     */
    public void setIconFileName(String fileName) {
        if (fileName == null) {
            throw new IllegalArgumentException("The given icon file name is null.");
        }

        this.iconFileName = fileName;
    }
  
    /**
     * Adds a given action to the action list. If the given action already exists in the action list, 
     * it will return false, and no changes are made to the action list.
     * <P>
     * <B>Notice:</B> A valid action should not have null verb or command field
     * <P>
     * On <B>Windows</B><P>
     * An association with none-empty action list would be valid for registration
     * when the file extension list is not empty.
     * 
     * @param action a given action.
     * @return true if the action list is changed as a result of the call.
     */
    public boolean addAction(Action action) {
        if (action == null) {
            throw new IllegalArgumentException("The given action is null.");
        }
        
        // Check the specified action object has no null verb and command field.
        if (action.getVerb() == null) { 
            throw new IllegalArgumentException("the given action object has null verb field.");
        } else if (action.getCommand() == null) {
            throw new IllegalArgumentException("the given action object has null command field.");
        }
    
        if (actionList == null) {
            actionList = new ArrayList();
        } 
        
        return actionList.add(new Action(action.getVerb(), action.getCommand(),
                        action.getDescription()));
    }
  
    /**
     * Removes a given action from the action list. If the action is not contained in the action list,
     * it will return false, and no changes are made to the action list.
     * <P>
     * <B>Notice:</B> A valid action should not have null verb or command field
     * 
     * @param action a given action.
     * @return true if the given action is contained the action list and removed. 
     */
    public boolean removeAction(Action action) {
        if (action == null) {
            throw new IllegalArgumentException("The given action is null.");
        }

        // Check the specified action object has no null verb and command field.
        if ((action.getVerb() == null) || (action.getCommand() == null)) {
            throw new IllegalArgumentException("the given action object has null verb field or command field.");
        }
        if (actionList != null) {
            return actionList.remove(action);
        }
        
        return false;
    }
  
    /**
     * Returns the action list
     * 
     * @return action list of the association
     */
    public List getActionList() {
        // Make defensive copy
        if (actionList == null) {
            return null;
        } else {
            List retList = new ArrayList();
            
            Iterator iter = actionList.iterator();
            while (iter.hasNext()) {
                retList.add(iter.next());
            }
            
            return retList;
        }            
    }

    /**
     * Returns the action, whose verb field is the same with the
     * specified verb. If such action does not exist, returns null.
     *
     * @param verb The specified verb
     * @return the action with the specified verb.
     */
    public Action getActionByVerb(String verb) {
        Iterator iter;
    
        if (actionList != null) {
            iter = actionList.iterator();
            if (iter != null) {
                while (iter.hasNext()) {
                    Action temAction = (Action) iter.next();
                    String temVerb = temAction.getVerb();

                    if (temVerb.equalsIgnoreCase(verb)) {
                        return temAction;
                    }
                }
            }
        }

        return null;
    }
  
    /**
     * Overrides equals() method.
     * <P>
     * On <B>Windows</B>:<P>
     *    RegisterSystemAssociation and RegisterUserAssociation will only save the header
     *    file extension in the file extension list. 
     *    The retrieved association (with only one file extension) may not be equal to the one
     *    registerd (with multiple file extensions in the file extension list) 
     *  
     * @param object reference for other object
     * @return true if the specified Assoction object equals to this object.
     */
    public boolean equals(Object other) {
        if (!(other instanceof Association)) {
            return false;
        }
        Association otherAssoc = (Association) other;
    
        /*
         * Compare if the basic part of the association (description, iconfile, mimetype)
         * equals
         */
        boolean isBasicEquals, isActionListEquals, isFileExtListEquals;
        String otherDesc = otherAssoc.getDescription();
        String otherIconFileName = otherAssoc.getIconFileName();
        String otherMimeType = otherAssoc.getMimeType();

        isBasicEquals = ((description == null
                        ? otherDesc == null
                        : description.equals(otherDesc))
                && (iconFileName == null
                        ? otherIconFileName == null
                        : iconFileName.equals(otherIconFileName))
                && (mimeType == null
                        ? otherMimeType == null
                        : mimeType.equals(otherMimeType)));
                     
        if (!isBasicEquals) {
            return false; 
        }
        
        //Compare if the file extension list equals
        List otherFileExtList = otherAssoc.getFileExtList();
        isFileExtListEquals = false;
        //fileExtlistEqulas when
        //1. both file extension lists are null
        //2. neither file extension lists is null and they have same elements
        if ((fileExtensionList == null) && (otherFileExtList == null)) {
            isFileExtListEquals = true;
        } else if ((fileExtensionList != null) && (otherFileExtList != null)) {
            if ((fileExtensionList.containsAll(otherFileExtList)) &&
                (otherFileExtList.containsAll(fileExtensionList))) {
                isFileExtListEquals = true;
            }
        }
        if (!isFileExtListEquals) {
            return false;
        }

        //Compare if the action list equals
        List otherActionList = otherAssoc.getActionList();
        isActionListEquals = false;
        //action list Equlas when
        //1. both action lists are null
        //2. neither action lists is null and they have same elements
        if ((actionList == null) && (otherActionList != null)) {
            isActionListEquals = true;
        } else if ((actionList != null) && (otherActionList != null)) {
            if ((actionList.containsAll(otherActionList)) &&
                (otherActionList.containsAll(actionList))) {
                isActionListEquals = true;
            }
        }
        
        return isActionListEquals;
    }
    
	/**
	 * Returns the hashcode of this association object
	 * 
	 * @return hashcode for this association object
	 */
	public int hashCode() {
		if (hashcode != 0) {
			int result = 17;
			if (this.name != null) {
				result = result * 37 + this.name.hashCode();
			}
			if (this.description != null) {
				result = result * 37 + this.description.hashCode();
			}
			if (this.mimeType != null) {
				result = result * 37 + this.mimeType.hashCode();
			}
			if (this.iconFileName != null) {
				result = result * 37 + this.iconFileName.hashCode();
			}
			if (this.fileExtensionList != null) {
				result = result * 37 + this.fileExtensionList.hashCode();
			}
			if (this.actionList != null) {
				result = result * 37 + this.actionList.hashCode();
			}
			hashcode = result;
		}
		return hashcode;
	}
    
  
    /**
     * Overrides toString() method.
     * <PRE>
     * The output of this object as a string would be like:
     *
     *     MIME File Name:  
     *     Description:  
     *     MIME Type:  
     *     Icon File:  
     *     File Extension:  
     *     Action List:  
     *         Description:    
     *         Verb: 
     *         Command: 
     * </PRE>
     * @return String representation of this association
     */
    public String toString() {
        String crlfString = "\r\n";
        String content = "";
        Iterator temIter;

        content = content.concat("MIME File Name: ");
        if (this.name != null) {
            content = content.concat(name);
        }
        content = content.concat(crlfString);

        content = content.concat("Description: ");
        if (this.description != null) {
			content = content.concat(description);
        }
        content = content.concat(crlfString);
    
        content = content.concat("MIME Type: ");
        if (this.mimeType != null) {
            content = content.concat(mimeType);
        }
        content = content.concat(crlfString);

        
        content = content.concat("Icon File: ");
        if (this.iconFileName != null) {
            content = content.concat(iconFileName);
        }
        content = content.concat(crlfString);
    
        content = content.concat("File Extension: ");
        if (fileExtensionList != null) {
            temIter = fileExtensionList.iterator();
            if (temIter != null) {
                while (temIter.hasNext()) {
                    content = content.concat((String) temIter.next());
                    if (temIter.hasNext()) {
                        content = content.concat(" ");
                    }
                }
            }
        }
        content = content.concat(crlfString);
    
        content = content.concat("Action List: ");
        if (actionList != null) {
            temIter = actionList.iterator();
            if (temIter != null) {
                content = content.concat(crlfString);
                while (temIter.hasNext()) {
                    Action temAction = (Action) temIter.next();
                    content = content.concat(temAction.toString());
                }
            }
        }
        content = content.concat(crlfString);
    
        return content;
    }
}
