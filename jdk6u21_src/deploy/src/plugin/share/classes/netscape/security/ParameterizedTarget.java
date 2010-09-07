/*
 * @(#)ParameterizedTarget.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;

/**
 * This class is a subclass of UserTarget. It's meant to use the optional 
 * argument to checkScopePermission() which allows more specific information 
 * to be passed to the target (i.e., file names or network addresses). Once 
 * you register a Principal with the ParameterizedTarget, you can 
 * enablePrivilege() to build up an access control list (mapping any objects 
 * to Privileges), and checkScopePermission() allows you to query the access 
 * control list. 
 *
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public class ParameterizedTarget extends netscape.security.UserTarget
{
    /**
     * This constructor is only used by the Persistent store. 
     * Not guaranteed to be supported in future versions. 
     */
    protected ParameterizedTarget()
    {
	super();
    }


    /**
     * This constructor is only used by the Persistent store. 
     * Not guaranteed to be supported in future versions. 
     *
     * @param name The name of the target (by convention, target 
     *	      names use a "BouncyCaps" style, with the first word 
     *	      capitalized) 
     * @param prin The principal in which the name is defined 
     *	      (basically, principals define namespaces for targets) 
     * @param risk A word or short phrase describing how dangerous 
     *	      this target could make an applet (usual strings are 
     *	      "low", "medium", and "high") 
     * @param riskColor - A background color for the user dialog. 
     *	      Assume the text will be black and pick a light color, 
     *	      e.g., a pastel color. The format of the color is 
     *	      "#rrggbb". 
     * @param description A short sentence describing what this 
     *	      target permits for the user (i.e.: "Read-only access 
     *	      to the local filesystem") 
     * @param url A pointer to a Web page describing this target 
     *	      in as much detail as you want. 
     */
    public ParameterizedTarget(String name,
			       Principal prin,
			       String risk,
			       String riskColor,
			       String description,
			       String url)
    {
	super(name, prin, risk, riskColor, description, url);
    }


    /**
     * @return the string representation of the data object that is stored 
     *	       with the target 
     */
    public String getDetailedInfo(Object data)
    {
	return null;
    }

    /** 
     * An intelligent target will override this method to implement its 
     * own access control policies. This method is called indirectly when 
     * other code calls: PrivilegeManager.enablePrivilege(). 
     * 
     * @param prin The principal on the requesting class 
     * @param data Arbitrary data, as passed to enablePrivilege() strictly 
     *	      for putting up a dialog box. sigh 
     */
    public Privilege enablePrivilege(Principal prin,
                                     Object data)
    {
	PrivilegeManager privMgr = PrivilegeManager.getPrivilegeManager();
	privMgr.enablePrivilege(this, prin, data);

	return null;
    }


    /** 
     * An intelligent target will override this method to implement its 
     * own access control policies. 
     *
     * @param prin The principal on the requesting class 
     * @param data Arbitrary data, as passed to checkPrivilegeEnabled() 
     */
    public Privilege checkPrivilegeEnabled(Principal prinAry[],
	                                   Object data)
    {
	PrivilegeManager privMgr = PrivilegeManager.getPrivilegeManager();
	privMgr.checkPrivilegeEnabled(this, data);

	return null;
    }
}

