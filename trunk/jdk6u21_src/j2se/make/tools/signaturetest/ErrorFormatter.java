/*
 * @(#)ErrorFormatter.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.tests.api.SignatureTest;

import java.io.PrintWriter;
import java.util.Vector;
/** This is class for formatting error messages which are created by
 *  SignatureTest or APIChangesTest. This class returns messages in
 *  the plain format without sorting and grouping. The subclasses of
 *  this class can represents other formats. **/
public class ErrorFormatter implements SignatureConstants {
    /** field include messages for different type of errors **/
    String[]  messages = {
	"Required class not found in implementation: ", // Missing Classes
        "Definition required but not found in ",       
        // Missing nested Classes or class definitions
	"Definition required but not found in ",       
	// Missing Superclasses or Superinterfaces
	"Definition required but not found in ",        // Missing Fields
	"Definition required but not found in ",        // Missing Constructors
	"Definition required but not found in ",        // Missing Methods
	"Found class not permitted in implementation: ",// Added Classes
	"Definition found but not permitted in ",      
        // Added nested Classes or class definitions
	"Definition found but not permitted in ",     
	// Added Superclasses or Superinterfaces
	"Definition found but not permitted in ",       // Added Fields
	"Definition found but not permitted in ",       // Added Constructors
	"Definition found but not permitted in ",       // Added Methods
        "LinkageError does not allow to track definition in " // LinkageError
    };
    /** PrintWriter which prints error messages **/
    PrintWriter out;
    /** number of errors **/
    int size;
    
    /** Creates ErrorFormatter which PrintWriter
     *  @param out  PrintWriter which prints error messages **/
    public ErrorFormatter(PrintWriter out) {
        this.out = out;
        size = 0;
    }

    /** Creates ErrorFormatter which PrintWriter and new messages
     *  @param out  PrintWriter which prints error messages
     *  @param messages messages for errors **/
    public ErrorFormatter(PrintWriter out, String messages[]) {
        this.out = out;
        size = 0;
        this.messages = messages;
    }

    /** adds new error to the ErrorFormatter
     *  @param errorType type of the error message
     *  @param className name of the class where is error is found
     *  @param def short error message
     *  @param tail the tail of the error message which will be added to
     *  the message in the plain format **/
    public void addError(String errorType, String className, String def,
                         String tail) {
        out.println(createError(errorType, className, def, tail));
        size++;
    }

    /** set the new messages
     *  @param messages the new messages **/
    public void setMessages(String messages[]) {
        this.messages = messages;
    }

    /** create new error
     *  @param errorType type of the error message
     *  @param className name of the class where is error is found
     *  @param def short error message
     *  @param tail the tail of the error message which will be added to
     *  the message in the plain format **/        
    protected ErrorMessage createError(String errorType, String className,
                                       String def, String tail) {
        int errorDeg = 0;
	if (def != null) {
	    if (def.startsWith(INNER) ||
		def.startsWith(CLASS))
		errorDeg = 1;
	    if (def.startsWith(SUPER) || 
		def.startsWith(INTERFACE))
		errorDeg = 2;
	    if (def.startsWith(FIELD))
		errorDeg = 3;
	    if (def.startsWith(CONSTRUCTOR))
		errorDeg = 4;
	    if (def.startsWith(METHOD))
		errorDeg = 5;
	}
	if (!errorType.equals("Missing"))
	    errorDeg += 6;
        if (errorType.equals("LinkageError"))
            errorDeg = 12;
        ErrorMessage er = new ErrorMessage(errorDeg, className, def, tail);
        er.setMessages(messages);
        return er;
    }
    /** returns number of the error messages and prints errors **/
    public int printErrors() {
        return size;
    }
    
    /** returns definition with full name of definition type **/
    public static String toString(String label) {
        for (int i = 0; i < prefixes.length; i++) {
            if (label.startsWith(prefixes[i][0])) 
                return (prefixes[i][1] + 
                        label.substring(prefixes[i][0].length()));
        }
        return label;
    }
}

