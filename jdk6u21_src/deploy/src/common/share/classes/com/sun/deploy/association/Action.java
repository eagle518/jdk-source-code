/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.association;


/**
 * This class represents a key/value mapping pair. The key could be a verb applied to 
 * a file by launching an associated application. It could also be an information
 * key stores the value.
 * <P>
 * The action could be added to an <code>Association</code>, and then registered to 
 * the system registry(on Windows) or MIME database(on Gnome desktop)
 * <P>
 * Only Action objects with both verb field and command field filled can be
 * added or removed from Association objects.
 *
 * @see Association
 * @version 1.0
 */
public class Action {
  
    /**
     * Description of this action.
     * <P>
     * <B>Notice: </B>This field is not required to create a valid Action object.
     * It's used only on Windows, and on Gnome for Linux and Solaris, this field 
     * is not used.
     */
    private String description;
  
    /**
     * Name of the verb field.
     */
    private String verb;
  
    /**
     * Command field associated with the given verb.
     */
    private String command;
    
    /**
     * Hash code for this action
     */
    private int hashcode = 0;
  
    /**
     * Constructor of an Action object. Which may be used as part of an association to
     * be registered/unregistered into/from the Gnome MIME database.
     * <PRE>
     * On <B>Windows</B>
     *   The verb field could be like: open, view, print, etc. Or any
     *   user defined verbs.
     * On <B>Gnome desktop</B>
     *   *Only* open verb could be used, and actions with other verbs will be ignored.
     *   Since other user defined verbs couldn't be used in file viewers, such as Nautilus,
     *   to register those verbs in the MIME type database is meanless.
     * </PRE> 
     *
     * @param verb a given verb value.
     * @param command a given command value.
     */
    public Action(String verb, String command) {
        this.verb = verb;
        this.command = command;
    }

    /**
     * Constructor of an Action object.
     * 
     * @param verb a given verb value.
     * @param command a given command value.
     * @param desc a given description value
     */
    public Action(String verb, String command, String desc) {
        this.verb = verb;
        this.command = command;
        this.description = desc;
    }
  
    /**
     * Returns the value of the description field.
     * 
     * @return the value of the desctiption field.
     */
    public String getDescription() {
        return description;
    }
  
    /**
     * Sets the description field.
     * <P>
     * <B>Notice: </B>This field is not required to create a valid Action object.
     * It's used only on Windows, and not used on Gnome for both Linux and Solaris.
     * 
     * @param description a given description value.
     */
    public void setDescription(String description) {
        this.description = description;
    }
  
    /**
     * Returns the value of the verb field.
     * 
     * @return the value of the verb field.
     */
    public String getVerb() {
        return verb;
    }
  
    /**
     * Sets the verb field.
     * 
     * @param verb a given verb value.
     */
    public void setVerb(String verb) {
        this.verb = verb;
    }
  
    /**
     * Returns the value of the command field.
     *
     * @return the value of the command field.
     */
    public String getCommand() {
        return command;
    }
  
    /**
     * Sets the command field.
     * 
     * @param command a given command value.
     */
    public void setCommand(String command) {
        this.command = command;
    }
  
    /**
     * Overrides equals() method
     * 
     * @param otherObj other object for the comparision
     * @return true if two action objects contain same values
     */
    public boolean equals(Object otherObj) {
        if (otherObj instanceof Action) {
            Action otherAction = (Action) otherObj;
            String otherDescription = otherAction.getDescription();
            String otherVerb = otherAction.getVerb();
            String otherCommand = otherAction.getCommand();

            if ((description == null
                            ? otherDescription == null
                            : description.equals(otherDescription))
                    && (verb == null
                            ? otherVerb == null
                            : verb.equals(otherVerb))
                    && (command == null
                            ? otherCommand == null
                            : command.equals(otherCommand))) {
                return true;
            }
        }
        return false; 
    }
    
    /**
     * Returns the hash code of this action
     * 
     * @return hash code of the action
     */
    public int hashCode() {
    	if (hashcode != 0) {
    		int result = 17;
    		if (this.description != null) {
    			result = 37 * result + this.description.hashCode();	
    		}
    		if (this.verb != null) {
    			result = 37 * result + this.verb.hashCode();
    		}
    		if (this.command != null) {
    			result = 37 * result + this.command.hashCode();
    		}
    		hashcode = result;
    	}
    	return hashcode;
    }
  
    /**
     * Overrides toString() method.
     *
     * @return String
     */
    public String toString() {
        String crlfString = "\r\n";
        String content = "";
        String tabString = "\t";
    
        content = content.concat(tabString);
        content = content.concat("Description: ");
        if (this.description != null) {
			content = content.concat(description);
        }
        content = content.concat(crlfString);
            
		content = content.concat(tabString);
        content = content.concat("Verb: ");
        if (this.verb != null) {
			content = content.concat(verb);
        }
        content = content.concat(crlfString);

		content = content.concat(tabString);
        content = content.concat("Command: ");
        if (this.command != null) {
			content = content.concat(command);
        }
        content = content.concat(crlfString);
    
        return content;  
    }
}
