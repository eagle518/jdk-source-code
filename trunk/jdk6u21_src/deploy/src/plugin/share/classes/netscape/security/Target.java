/*
 * @(#)Target.java	1.8 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;

/**
 * All access control decisions boil down to who is allowed to do what. 
 * The Principal class represents the who, and the Target class represents 
 * the what. In this system, a target may represent a primitive device 
 * ("the microphone") or it may be a macro for a number of other targets 
 * ("typical game privileges"). 
 *
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public class Target 
{
    // private members
    private String name = null;
    private Principal prin = null;
    private String risk = null;
    private String riskColor = null;
    private String description = null;
    private String url = null;
    private Target targetAry[] = null;
    		 
    /** 
     * This constructor is only used by the Persistent store. Not 
     * guaranteed to be supported in future versions. 
     */
    protected Target()
    {
	this(null, null, null, null, null, null, null);
    }

    /** 
     * Constructor can only be called by folks in the netscape.security 
     * package. Outsiders should call registerTarget() after creating 
     * the target and use the target returned by registerTarget. 
     *
     * @param name The name of the target (by convention, target names 
     *	      use a "BouncyCaps" style, with the first word capitalized) 
     * @param prin The principal in which the name is defined (basically,
     *	      principals define namespaces for targets) 
     */
    public Target(String name, Principal prin)
    {
	this(name, prin, null, null, null, null, null);
    }

    /**
     * Constructor can only be called by folks in the netscape.security 
     * package. Outsiders should call registerTarget() after creating 
     * the target and use the target returned by registerTarget.      
     *
     * @param name The name of the target (by convention, target names 
     *	      use a "BouncyCaps" style, with the first word capitalized) 
     */
    public Target(String name)
    {
	this(name, null, null, null, null, null, null);
    }


    /** 
     * Constructor can only be called by folks in the netscape.security 
     * package. Outsiders should call registerTarget() after creating 
     * the target and use the target returned by registerTarget. 
     *
     * @param name The name of the target (by convention, target names 
     *	      use a "BouncyCaps" style, with the first word capitalized) 
     * @param prin The principal in which the name is defined (basically,
     *	      principals define namespaces for targets) 
     * @param targetAry An array of targets for the macro. Each target 
     *	      in the array must have the same principal as the new target. 
     */
    public Target(String name,
 		  Principal prin,
		  Target targetAry[])
    {
	this(name, prin, null, null, null, null, targetAry);
    }

    /** 
     * Constructor can only be called by folks in the netscape.security 
     * package. Outsiders should call registerTarget() after creating 
     * the target and use the target returned by registerTarget. 
     *
     * @param name The name of the target (by convention, target names 
     *	      use a "BouncyCaps" style, with the first word capitalized) 
     * @param prin The principal in which the name is defined (basically,
     *	      principals define namespaces for targets) 
     * @param risk A word or short phrase describing how dangerous this 
     *	      target could make an applet (usual strings are "low", 
     *	      "medium", and "high") 
     * @param riskColor A background color for the user dialog. Assume 
     *	      the text will be black and pick a light color, e.g., a 
     *	      pastel color. The format of the color is "#rrggbb". 
     * @param description A short sentence describing what this target 
     *	      permits for the user (i.e.: "Read-only access to the local 
     *	      filesystem") 
     * @param url A pointer to a Web page describing this target in as 
     *	      much detail as you want. 
     */
    public Target(String name,
                  Principal prin,
                  String risk,
                  String riskColor,
                  String description,
                  String url)
    {
	this(name, prin, risk, riskColor, description, url, null);
    }


    /** 
     * Constructor can only be called by folks in the netscape.security 
     * package. Outsiders should call registerTarget() after creating 
     * the target and use the target returned by registerTarget. 
     *
     * @param name The name of the target (by convention, target names 
     *	      use a "BouncyCaps" style, with the first word capitalized) 
     * @param prin The principal in which the name is defined (basically,
     *	      principals define namespaces for targets) 
     * @param risk A word or short phrase describing how dangerous this 
     *	      target could make an applet (usual strings are "low", 
     *	      "medium", and "high") 
     * @param riskColor A background color for the user dialog. Assume 
     *	      the text will be black and pick a light color, e.g., a 
     *	      pastel color. The format of the color is "#rrggbb". 
     * @param description A short sentence describing what this target 
     *	      permits for the user (i.e.: "Read-only access to the local 
     *	      filesystem") 
     * @param url A pointer to a Web page describing this target in as 
     *	      much detail as you want. 
     * @param targetAry An array of targets for the macro. Each target 
     *	      in the array must have the same principal as the new target. 
     */
    public Target(String name,
                  Principal prin,
                  String risk,
                  String riskColor,
                  String description,
                  String url,
	          Target targetAry[])
    {
	this.name = name;
	this.prin = prin;
	this.risk = risk;
	this.riskColor = riskColor;
	this.description = description;
	this.url = url;
	this.targetAry = targetAry;
    }


    /**
     * If another instance of Target is registered with the same name as 
     * this one, the other is returned. Else, this one is added to the target 
     * registry and returned. 
     */
    public final Target registerTarget()
    {
	return this;
    }

    
    /** 
     * Search for a target, and if it's not already there, return null. 
     * findTarget() looks up the the name in the target registry. In this 
     * case, with the system principal is an implicit second argument. If 
     * the target exists, it is returned. Otherwise, null is returned. 
     *
     * @param name The name of the target (by convention, target names use 
     *	      a "BouncyCaps" style, with the first word capitalized) 
     */
    public static Target findTarget(String name)
    {
	return new Target(name);
    }

    /**
     * Search for a target, and if it's not already there, return null. 
     * findTarget() looks up the the name in the target registry. In this 
     * case, the second argument, a Principal, defines the namespace to 
     * search for the target. If the target exists, it is returned. 
     * Otherwise, null is returned. 
     *
     * @param name The name of the target (by convention, target names 
     *	      use a "BouncyCaps" style, with the first word capitalized) 
     * @param prin The principal in which the name is defined 
     */
    public static Target findTarget(String name, Principal prin)
    {
	return new Target(name, prin);
    }

    /** 
     * Search for a target, and if it's not already there, return null. 
     * Most people will not use this method. Given any target, registered 
     * or not, this searches the target registry for a target having the 
     * same name and principal. If the target exists, it is returned. 
     * Otherwise, null is returned. 
     * 
     * @param target Used to get the name and principal of the target 
     *	      for which you're actually searching 
     */
    public static Target findTarget(Target target)
    {
	return target;
    }

    /** 
     * An intelligent target will override this method to implement its 
     * own access control policies. By default, this method returns a 
     * blank privilege. 
     *
     * @param prinAry The principals on the requesting class. 
     * @param data When the resource wanting to protect itself calls 
     *	      PrivilegeManager.checkPrivilegeEnabled() and passes the 
     *	      optional data field, it comes out here. It's meaning is 
     *	      specific to each target. For example, a restricted filesystem 
     *	      target might use this to pass the file-name and then implement 
     *	      a policy about which subset of the filesystem is accessible. 
     * @return A Privilege object. In the normal case, a blank privilege is 
     *	       returned. This privilege is combined with other privileges to 
     *	       ultimately decide whether the access is granted or denied. For 
     *	       more information on how this works, see Privilege.add() 
     */
    public Privilege checkPrivilegeEnabled(Principal prinAry[],
					   Object data)
    {
	PrivilegeManager privMgr = PrivilegeManager.getPrivilegeManager();
	privMgr.checkPrivilegeEnabled(this, data);

	return null;
    }


    /** 
     * An intelligent target will override this method to implement its own 
     * access control policies. By default, this method returns a blank 
     * privilege. This version only takes one argument and is invoked when 
     * the corresponding PrivilegeManager.checkPrivilegeEnabled() method 
     * without the Object data argument is called. 
     *
     * @param prinAry The principals on the requesting class. 
     * @return A Privilege object. In the normal case, a blank privilege is 
     *	       returned. This privilege is combined with other privileges to 
     *	       ultimately decide whether the access is granted or denied. For 
     *	       more information on how this works, see Privilege.add() 
     */
    public Privilege checkPrivilegeEnabled(Principal prinAry[])
    {
	return checkPrivilegeEnabled(prinAry, null);
    }


    /** 
     * An intelligent target will override this method to implement its 
     * own access control policies. By default, this method returns a 
     * blank privilege. 
     *
     * @param p The principal on the requesting class. 
     * @parma data Any arbitrary data, as passed to 
     *	      PrivilegeManager.checkPrivilegeEnabled(). For more information, 
     *	      see checkPrivilegeEnabled(Principal[], Object), above. 
     */
    public Privilege checkPrivilegeEnabled(Principal p,
                                           Object data)
    {
	PrivilegeManager privMgr = PrivilegeManager.getPrivilegeManager();
	privMgr.checkPrivilegeEnabled(this, data);

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
    public Privilege enablePrivilege(Principal prin, Object data)
    {
	PrivilegeManager privMgr = PrivilegeManager.getPrivilegeManager();
	privMgr.enablePrivilege(this, prin, data);

	return null;
    }


    /**
     * @return A short string describing the "danger" level of this applet 
     *	       (i.e.: "low", "medium", or "high") 
     */
    public String getRisk()
    {
	return risk;
    }

    /**
     * @return A background color for the user dialog. These generally 
     *	       assume the text will be black and are thus light colors, 
     *	       e.g., pastels. The format of the color is "#rrggbb". 
     */
    public String getRiskColor()
    {
	return riskColor;
    }

    /** 
     * @return A short sentence describing what this target permits for 
     *	       the user (i.e.: "Read-only access to the local filesystem") 
     */
    public String getDescription()
    {
	return description;
    }

    public static Target getTargetFromDescription(String desc)
    {
	return null;
    }

    /**
     * @return A pointer to a Web page describing this target in detail. 
     */
    public String getHelpUrl()
    {
	return url;
    }

    /** 
     * @return the string representation of the data object that is stored 
     *	       with the target 
     */
    public String getDetailedInfo(Object data)
    {
	return null;
    }
}

