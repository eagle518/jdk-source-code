/*
 * @(#)ErrorMessage.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

/** This class represents Error message created
  *  by APIChanges or SignatureTest **/
public class ErrorMessage {
    /** messages for current error message. If for current error type this
     *  message is not exist, than ErrorFormatter.messages is used **/
    String messages[];
    /** name of the class where is error is found **/
    String className;
    /** failed definition **/
    String definition;
    /** the tail of the error message which will be added to the message
     *  in the plain format **/
    String tail;
    /** type of the error message **/
    int errorType;
    /** creates new error message 
     *  @param errorType type of the error message
     *  @param className name of the class where is error is found
     *  @param definition failed definition
     *  @param tail the tail of the error message which will be added to
     *  the message in the plain format **/
    public ErrorMessage(int errorType, String className, String definition,
                        String tail) {
        this.errorType = errorType;
        this.className = className;
        this.definition = (definition == null) ? "" : definition ;
        this.tail = (tail == null) ? "" : tail ;
        
    }

    /** set the new messages
     *  @param messages the new messages **/
    public void setMessages(String messages []) {
        this.messages = messages;
    }


    /** Compares this ErrorMessage with the given ErrorMessage for order.
     *  The ordering rules are following:
     *  <p> 1. compare errorType as integer
     *  <p> 2. if errorTypes are equal, than compare classNames alphabetically.
     *  <p> 3. if ClassNames are equals than compare definition <p>
     *  Returns a negative integer, zero, or a positive integer as this
     *  ErrorMessage is less than, equal to, or greater than the given
     *  ErrorMessage. **/
    public int compareTo(ErrorMessage ob) {
        if (ob.errorType == this.errorType) {
            if (this.className.equals(ob.className))
                return (getShortName(this.definition)).compareTo(
                    getShortName(ob.definition));
            else 
                return this.className.compareTo(ob.className);
        } else {
            return this.errorType - ob.errorType;
        }
    }

    /** Returns short name for missing or added member
     *  @param def definition of the given member **/
    public String getShortName(String def) {
        String retVal;
        int pos = def.lastIndexOf(" throws ");
        if (pos >= 0)
            retVal = def.substring(0, pos);
        else
            retVal = def;
        return retVal.substring(retVal.lastIndexOf(' ') + 1);
    }

    /** returns String representation of current ErrorMessage **/
    public String toString() {
        return messages[errorType] + className + 
            "\n    " + ErrorFormatter.toString(definition) + tail;
    }
}



