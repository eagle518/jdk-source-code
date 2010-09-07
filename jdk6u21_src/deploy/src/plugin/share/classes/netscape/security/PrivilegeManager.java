/*
 * @(#)PrivilegeManager.java	1.15 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;

import com.sun.deploy.resources.ResourceManager;

/**
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public class PrivilegeManager
{
    public final static int PROPER_SUBSET = 0x0001;
    public final static int EQUAL = 0x0002;
    public final static int NO_SUBSET = 0x0003;
    public final static int SIGNED_APPLET_DBNAME = 0x0004;
    public final static int TEMP_FILENAME = 0x0005;

    /**
     * Create a PrivilegeManager. Normal users of the security system will 
     * never need to do this. 
     */
    protected PrivilegeManager()
    {
    }

    /**
     * Checks whether the current principal has been granted privileges for 
     * the specified target, usually as a result of a call to 
     * enablePrivilege(). 
     *
     * @param target Target whose access is being checked
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target 
     */
    public void checkPrivilegeEnabled(Target target) 
		throws ForbiddenTargetException
    {
	printErrorMessage();
    }

    /**
     * Checks whether the current principal has been granted privileges for 
     * the specified target, usually as a result of a call to 
     * enablePrivilege(). 
     *
     * @param target Target whose access is being checked
     * @param data passed to the target, allowing the target to make more 
     *		   detailed access control decisions. 
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public void checkPrivilegeEnabled(Target target, Object data) 
		throws ForbiddenTargetException
    {
	printErrorMessage();
    }


    /**
     * This call enables privileges to the given target until the calling 
     * method exits. As a side effect, if your principal does not have 
     * privileges for the target, the user may be consulted to grant these 
     * privileges. 
     *
     * @param targetStr the name of the Target whose access is being checked.
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public static void enablePrivilege(String targetStr)
		       throws ForbiddenTargetException		
    {
	printErrorMessage();
    }


    /** 
     * This call enables privileges to the given target until the calling 
     * method exits. As a side effect, if your principal does not have 
     * privileges for the target, the user may be consulted to grant these 
     * privileges. 
     *
     * @param target Target whose access is being checked 
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public void enablePrivilege(Target target)
	        throws ForbiddenTargetException		
    {
	printErrorMessage();
    }

    /**
     * This call enables privileges to the given target until the calling 
     * method exits. See above for details. 
     *
     * @param target Target whose access is being checked 
     * @param prefPrin You may optionally specify a principal on whose 
     *	      behalf you're enabling the privilege. If you have more then one 
     *	      principal (i.e., you have more than one digital signature), this 
     *	      lets you specify which principal may be shown to the user. It is 
     *	      an error to specify a preferred principal which has not signed 
     *	      your code. However, if you have Impersonator privileges, this 
     *	      lets you act on behalf of any principal. 
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public void enablePrivilege(Target target, Principal prefPrin)
	        throws ForbiddenTargetException		
    {
	printErrorMessage();
    }


    /**
     * This call enables privileges to the given target until the calling 
     * method exits. See above for details. 
     *
     * @param target Target whose access is being checked 
     * @param prefPrin You may optionally specify a principal on whose 
     *	      behalf you're enabling the privilege. If you have more then one 
     *	      principal (i.e., you have more than one digital signature), this 
     *	      lets you specify which principal may be shown to the user. It is 
     *	      an error to specify a preferred principal which has not signed 
     *	      your code. However, if you have Impersonator privileges, this 
     *	      lets you act on behalf of any principal
     * @param data Passed to the target, allowing the target to make more 
     *	      detailed access control decisions
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public void enablePrivilege(Target target, Principal prefPrin, Object data)
	        throws ForbiddenTargetException		
    {
	printErrorMessage();
    }


    /**    
     * This undoes the action of a previous enablePrivilege() or 
     * disablePrivilege(). The stack annotation of the caller, for the given 
     * target, is returned to its original state. 
     *
     * @param target Target whose access is being modified
     */
    public void revertPrivilege(Target target)
    {
	printErrorMessage();
    }


    /**    
     * This undoes the action of a previous enablePrivilege() or 
     * disablePrivilege(). The stack annotation of the caller, for the given 
     * target, is returned to its original state. 
     *
     * @param targetStr a target String whose access is being modified 
     */
    public static void revertPrivilege(String targetStr)
    {
	printErrorMessage();
    }

    /**
     * This call places an annotation on the caller stack frame forbidding 
     * access to the given target. The purpose of this is to shadow a 
     * positive annotation earlier in the call stack. This might be done 
     * prior to executing a callback into untrusted code. Unlike 
     * enablePrivilege(), this will not throw an exception. 
     *
     * @param target Target whose access is being modified
     */
    public void disablePrivilege(Target target)
    {
	printErrorMessage();
    }

    /**
     * This call places an annotation on the caller stack frame forbidding 
     * access to the given target. The purpose of this is to shadow a 
     * positive annotation earlier in the call stack. This might be done 
     * prior to executing a callback into untrusted code. Unlike 
     * enablePrivilege(), this will not throw an exception. 
     *
     * @param targetStr a target String whose access is being modified 
     */
    public static void disablePrivilege(String targetStr)
    {
	printErrorMessage();
    }


    /**
     * Check the current principal's privilege for this target. 
     *
     * @param targetStr a Target name whose access is being checked 
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public static void checkPrivilegeGranted(String targetStr)
	        throws ForbiddenTargetException		
    {
	printErrorMessage();
    }


    /**
     * Check the current principal's privilege for this target. 
     *
     * @param target Target whose access is being checked 
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public void checkPrivilegeGranted(Target target)
	        throws ForbiddenTargetException		
    {
	printErrorMessage();
    }

    /**
     * Check the current principal's privilege for this target. 
     *
     * @param target Target whose access is being checked 
     * @param data Passed to the target, allowing the target to 
     *	      make more detailed access control decisions
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public void checkPrivilegeGranted(Target target, Object data)
	        throws ForbiddenTargetException		
    {
	printErrorMessage();
    }

    /**
     * Check the current principal's privilege for this target. 
     *
     * @param target Target whose access is being checked 
     * @param prin Principal whose privileges you wish to query 
     * @param data Passed to the target, allowing the target to 
     *	      make more detailed access control decisions
     * @exception ForbiddenTargetException thrown if access is denied to 
     *		  the Target
     */
    public void checkPrivilegeGranted(Target target, Principal prin, Object data)
	        throws ForbiddenTargetException		
    {
	printErrorMessage();
    }


    /**
     * A method may inquire if its caller has a given principal. Since 
     * this check may need to be wrapped deep inside some other checks, 
     * an optional second argument says how many stack frames to skip. 
     *
     * @param prin The principal you're asking about - does the method 
     *	      which called you belong to a class owned by this principal? 
     * @param callerDepth Specifies which stack frame to interrogate 
     *	      for privileges. An argument of zero refers to the stack 
     *	      frame which invoked this method directly. Positive arguments 
     *	      increase the number of skipped stack frames. If you want to 
     *	      interrogate your caller, use a callerDepth of one. 
     * @return true iff the invoking class has the given principal
     */
    public boolean isCalledByPrincipal(Principal prin, int callerDepth)
    {
	printErrorMessage();
	return false;
    }

    /**
     * A method may inquire if its caller has a given principal. This 
     * callerDepth argument is implicitly 1. See above for details.
     *
     * @param prin The principal you're asking about - does the method 
     *	      which called you belong to a class owned by this principal? 
     * @return true iff the invoking class has the given principal
     */
    public boolean isCalledByPrincipal(Principal prin)
    {
	printErrorMessage();
	return isCalledByPrincipal(prin, 1);
    }

    /**
     * Get the principal used to sign system classes, such as the Java 
     * class libraries. 
     */
    public static Principal getSystemPrincipal()
    {
	printErrorMessage();
	return null;
    }

    /**
     * Gets the system Privilege Manager. 
     */ 
    public static PrivilegeManager getPrivilegeManager()
    {
	printErrorMessage();
	return new PrivilegeManager();
    }

    /**
     * Get an array of principals for the class which called 
     * getMyPrincipals(). 
     * 
     * @return an array of principals for the given stack frame - 
     *	       cryptographic principals (e.g.: certificates) will be 
     *	       first in the array, then codebase principals will follow. 
     */
    public static Principal[] getMyPrincipals()
    {
	printErrorMessage();
	return null;
    }

    /**
     * This method returns the principals associated with a given class. 
     * This may be useful to verify that classes you're about to call 
     * were signed by somebody you trust. 
     * 
     * @param cl Class for which we want to find the Principals
     * @return an array of principals for the given stack frame - 
     *	       cryptographic principals (e.g.: certificates) will be 
     *	       first in the array, then codebase principals will follow. 
     */
    public Principal[] getClassPrincipals(Class cl)
    {
	printErrorMessage();
	return null;
    }

    /**
     * A method may inquire if a given class was signed by a given 
     * principal. 
     * 
     * @return true iff the invoking class has the given principal. 
     */
    public boolean hasPrincipal(Class cl, Principal prin)
    {
	printErrorMessage();
	return false;
    }

    /**
     * Compares two arrays of Principals. 
     *
     * @return either PROPER_SUBSET or EQUAL or NO_SUBSET. This 
     *	       method returns either PROPER_SUBSET, EQUAL or 
     *	       NO_SUBSET, if the principal array pointed to by p1 
     *	       is a subset, is equal to, or is not a subset of the 
     *	       principal array pointed to by p2 respectively. 
     */
    public int comparePrincipalArray(Principal p1[], Principal p2[])
    {
	printErrorMessage();
	return NO_SUBSET;
    }

    
    /**
     * This method gets the list of principals at the caller depth 
     * and matches it with the principals for the given Class cl. 
     *
     * @param cl The class you're asking about
     * @param callerDepth Specifies which stack frame to integrrogate
     *	      for priviledge. An argument of zero refers to the stack 
     *	      frame which invoked this method directly. Positive 
     *	      arguments increase the number of skipped stack frames. 
     *	      If you want to interrogate your caller, use a callerDepth 
     *	      of one
     * @return true iff the given class has all the same principals as 
     *	       the class at the given stack depth
     */
    public boolean checkMatchPrincipal(Class cl, int callerDepth)
    {
	printErrorMessage();
	return false;
    }


    /**
     * This method gets the list of principals at the caller depth 
     * and matches it with the principals for the given Class cl. 
     *
     * @param prin The Principal you're comparing to the given stack frame 
     * @param callerDepth Specifies which stack frame to integrrogate
     *	      for priviledge. An argument of zero refers to the stack 
     *	      frame which invoked this method directly. Positive 
     *	      arguments increase the number of skipped stack frames. 
     *	      If you want to interrogate your caller, use a callerDepth 
     *	      of one
     * @return true iff the given class has all the same principals as 
     *	       the class at the given stack depth
     */
    public boolean checkMatchPrincipal(Principal prin, int callerDepth)
    {
	printErrorMessage();
	return false;
    }

    /**
     * Get an array of principals for the current class and match it with 
     * the principals for the given Class cl (equivalent to calling the 
     * above method with callerDepth of one). 
     *
     * @param cl The Class you're asking about 
     * @return true iff the given class has all your principals
     */
    public boolean checkMatchPrincipal(Class cl)
    {
	printErrorMessage();
	return false;
    }

    /**
     * Instruct your ClassLoader that all further classes loaded must 
     * have the same principals as you do. 
     */
    public boolean checkMatchPrincipalAlways()
    {
	printErrorMessage();
	return false;
    }

    /**
     * This method returns the principals associated with a given stack 
     * frame. This may be useful to make sure you're being called by 
     * somebody you trust. 
     * 
     * @param callerDepth Specifies which stack frame to interrogate for 
     *	      privileges. An argument of zero refers to the stack frame 
     *	      which invoked this method directly. Positive arguments 
     *	      increase the number of skipped stack frames. If you want 
     *	      to interrogate your caller, use a callerDepth of one. 
     * @return an array of principals for the given stack frame - 
     *	       cryptographic principals (e.g., certificates) will be first 
     *	       in the array, then codebase principals will follow. 
     */
    public Principal[] getClassPrincipalsFromStack(int callerDepth)
    {
	printErrorMessage();
	return null;
    }

    /*
     * This method returns the privilege table associated with your caller.
     */
    public PrivilegeTable getPrivilegeTableFromStack()
    {
	printErrorMessage();
	return null;
    }

    private static void printErrorMessage()
    {
	System.err.println(ResourceManager.getMessage("liveconnect.wrong.securitymodel"));
    }
}
