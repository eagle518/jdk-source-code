/*
 * @(#)UserTarget.java	1.7 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;

/**
 * This class is a subclass of Target intended to present the user with a 
 * dialog box when an applet wants access to itself. All the GUI stuff 
 * happens here. 
 *
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public class UserTarget extends netscape.security.Target
{
    /**
     * This constructor is only used by the Persistent store. 
     * Not guaranteed to be supported in future versions. 
     */
    public UserTarget()
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
    public UserTarget(String name,
		      Principal prin,
		      String risk,
		      String riskColor,
		      String description,
		      String url)
    {
	super(name, prin, risk, riskColor, description, url);
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
     * @param targetAry - An array of targets for the macro. Each 
     *	      target in the array must have the same principal as 
     *	      the new target. 
     */
    public UserTarget(String name,
                      Principal prin,
                      String risk,
                      String riskColor,
                      String description,
                      String url,
                      Target targetAry[])
    {
	super(name, prin, risk, riskColor, description, url, targetAry);
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
}   

