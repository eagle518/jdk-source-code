/*
 * @(#)Status.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package javasoft.sqe.javatest;

import java.io.PrintWriter;

/**
 * A class to embody the result of a test: a status-code and a related message.
 *
 * @author Jonathan J Gibbons
 */

public class Status
{
    /**
     * Create a Status that represents the successful outcome of a test.
     */
    public static Status passed(String reason) {
	return new Status(PASSED, reason);
    }

    /**
     * Create a Status that represents the unsuccessful outcome of a test.
     */
    public static Status failed(String reason) {
	return new Status(FAILED, reason);
    }

    /**
     * Create a Status that represents that the test completed, but that further
     * analysis of the output of the test against reference files is required.
     */
    public static Status checkFile(String reason) {
	return new Status(CHECK_FILE, reason);
    }

    /**
     * Create a Status that represents that the test completed, but that further
     * analysis of the output of the test against reference files is required.
     */
    public static Status error(String reason) {
	return new Status(ERROR, reason);
    }

    /**
     * Create a Status that represents that the test was not run because under
     * the conditions given it was not applicable.  This method is retained
     * for backwards compatability only; the resultant object is of FAILED
     * type.
     * @deprecated
     */
    public static Status notApplicable(String reason) {
	return new Status(FAILED, "Not Applicable: " + reason);
    }

    /**
     * Create a Status that represents that the test has not yet been run
     */
    static Status notRun(String reason) {
	return new Status(NOT_RUN, reason);
    }

    /**
     * Return true if the type code of the status is PASSED.
     * @see #passed
     * @see #getType
     * @see #PASSED
     */
    public boolean isPassed() {
	return (type == PASSED);
    }

    /**
     * Return true if the type code of the status is FAILED.
     * @see #failed
     * @see #getType
     * @see #FAILED
     */
    public boolean isFailed() {
	return (type == FAILED);
    }

    /**
     * Return true if the type code of the status is CHECK_FILE.
     * @see #checkFile
     * @see #getType
     * @see #CHECK_FILE
     */
    public boolean isCheckFile() {
	return (type == CHECK_FILE);
    }

    /**
     * Return true if the type code of the status is ERROR.
     * @see #error
     * @see #getType
     * @see #ERROR
     */
    public boolean isError() {
	return (type == ERROR);
    }

    /**
     * A return code indicating that the test was executed and was successful.
     * @see #passed
     * @see #getType
     */
    public static final int PASSED = 0;

    /**
     * A return code indicating that the test was executed but the test
     * reported that it failed.
     * @see #failed
     * @see #getType
     */
    public static final int FAILED = 1;

    /**
     * A return code indicating that the test was executed but did not itself 
     * determine whether it succeeded or failed;  instead, the output the 
     * test generated must be compared against a known correct version 
     * (a `golden file') to determine if it passed or not.
     * @see #checkFile
     * @see #getType
     */
    public static final int CHECK_FILE = 2;

    /**
     * A return code indicating that the test was not run because some error
     * occurred before the test could even be attempted. This is generally
     * a more serious error than FAILED.
     * @see #getType
     */
    public static final int ERROR = 3;

    /**
     * A return code indicating that the test has not yet been run in this context.  
     * (More specifically, no status file has been recorded for this test in the 
     * current work directory.)  This is for the internal use of the harness only.
     * @see #getType
     */
    public static final int NOT_RUN = 4;

    /**
     * Get a type code indicating the type of Status message this is.
     * @see #PASSED
     * @see #FAILED
     * @see #CHECK_FILE
     * @see #ERROR
     */
    public int getType() {
	return type;
    }

    /**
     * Get the message given when the status was created.
     */
    public String getReason() {
	return reason;
    }

    /**
     * Return a new Status object with a possibly augmented reason field
     */
    public Status augment(String aux) {
	if (aux == null || aux.length() == 0)
	    return this;
	else 
	    return new Status(type, (reason + " [" + aux + "]"));
    }

    /**
     * Return a new Status object with a possibly augmented reason field
     */
    public Status augment(Status aux) {
	return (aux == null ? this : augment(aux.reason));
    }

    /**
     * Parse a string-form of a Status.
     * @see #exit
     */
    public static Status parse(String s) {
	for (int i = 0; i < texts.length; i++)
	    if (s.startsWith(texts[i]))
		return new Status(i, s.substring(texts[i].length()).trim());
	return null;
    }

    /**
     * Standard routine.
     */
    public String toString() {
	if (reason == null || reason.length() == 0)
	    return texts[type];	
	else
	    return texts[type] + " " + reason;
    }

    /**
     * Convenience exit() function for the main() of tests to exit in such a 
     * way that the status passes up across process boundaries without losing
     * information (ie exit codes don't give the associated text of the status
     * and return codes when exceptions are thrown could cause unintended 
     * results). <p>
     *
     * An identifying marker is written to the error stream, which the script
     * running the test watches for as the last output before returning, 
     * followed by the type and reason
     *
     * The method does not return.  It calls System.exit with a value
     * dependent on the type.
     */
    public void exit() {
	PrintWriter strm = new PrintWriter(System.err);
	strm.print(EXIT_PREFIX);
	strm.print(texts[type]);
	strm.println(reason);
	strm.flush();
	System.exit(exitCodes[type]);
    }


    //-----internal routines------------------------------------------------------

    public Status(int type, String reason) { 
	this.type = type; 
	this.reason = reason; 
    }

    //----------Data members---------------------------------------------------------

    private int type;
    private String reason;

    public static final String EXIT_PREFIX = "STATUS:";

    private static String[] texts = {  
	// correspond to PASSED, FAILED, CHECK_FILE, ERROR, NOT_RUN
	"Passed.", 
        "Failed.", 
	"Completed--check results.",
	"Error.",
	"Not run."
    };

    /**
     * Exit codes used by Status.exit corresponding to
     * PASSED, FAILED, CHECK_FILE, ERROR, NOT_RUN.
     * The only values that should normally be returned from a test
     * are the first three; the other two values are provided
     * for completeness.
     * <small> Note: The assignment is historical and cannot easily be changed. </small>
     */
    public static final int[] exitCodes = { 95, 97, 96, 98, 99 };
}
