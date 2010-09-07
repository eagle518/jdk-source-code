/*
 * @(#)Privilege.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package netscape.security;

/**
 * For every Principal in the system, you can ask whether it's allowed
 * to access a Target (using various methods of PrivilegeManager). The 
 * answer to that question is a Privilege. 
 *
 * A Privilege tells you two things: whether the access in question is 
 * allowed, and if it is, how long it will be allowed for. These are 
 * called the permission and duration of the privilege. 
 *
 * a permission may be allowed, forbidden, or blank 
 * a duration may be scope, session, or forever 
 *
 * Note that you never contruct your own Privilege. Instead, you use the 
 * findPrivilege method. There exists exactly one instance of each possible 
 * Privilege. This means that you can safely compare privileges with ==, 
 * which is faster than using the equals() 
 *  
 * This class acts as a stub to provide backward compatibility for Netscape 
 * 4.x VM.
 */
public final class Privilege
{
    /** 
     * number of possible permissions (allowed, forbidden, or blank) 
     */
    public final static int N_PERMISSIONS = 0x000F;

    /**
     * forbidden privilege
     */
    public final static int FORBIDDEN = 0x0000;

    /**
     * allowed privilege
     */
    public final static int ALLOWED = 0x0001;

    /** 
     * blank privilege (access is neither explicitly allowed nor forbidden)
     */ 
    public final static int BLANK = 0x0002;

    /** 
     * number of possible durations (scope, session, or forever) 
     */ 
    public final static int N_DURATIONS = 0x00F0;

    /**
     * scope duration (until current procedure returns) 
     */
    public final static int SCOPE = 0x0010;

    /**
     * session duration (until browser exits) 
     */
    public final static int SESSION = 0x0020;

    /**
     * permanent duration (until certificate expires) 
     */
    public final static int FOREVER = 0x0040;

    // Private members     
    private int permission = FORBIDDEN;
    private int duration = SCOPE;

    /** 
     * Used for persistent object storage. Not guranteed to be 
     * supported in the future. 
     */
    Privilege(int permission, int duration)
    {
	this.permission = permission;
	this.duration = duration;
    }

    /** 
     * findPrivilege() should be used instead of the Privilege 
     * constructor. Because the number of possible privileges is 
     * reasonably small, this function returns a handle to a pre-allocated 
     * Privilege, which accelerates comparison tests (you can safely use 
     * == to compare Privilege objects). 
     * 
     * @param permission One of: ALLOWED, FORBIDDEN, or BLANK 
     * @param duration One of: SCOPE, SESSION, or FOREVER 
     * @return the Privilege corresponding to the arguments 
     */
    public static Privilege findPrivilege(int permission,
					  int duration)
    {
	return new Privilege(permission, duration);
    }

    /**
     * add() method takes two permissions and returns a new permission. 
     * Permission addition follows these rules: 
     * ALLOWED   + ALLOWED   = ALLOWED        1 + 1 = 1
     * ALLOWED   + BLANK     = ALLOWED        1 + 2 = 1
     * BLANK     + BLANK     = BLANK          2 + 2 = 2
     * ALLOWED   + FORBIDDEN = FORBIDDEN      1 + 0 = 0
     * BLANK     + FORBIDDEN = FORBIDDEN      2 + 0 = 0
     * FORBIDDEN + FORBIDDEN = FORBIDDEN      0 + 0 = 0
     *
     * @return the permission that results from the above rules 
     */
    public static int add(int p1, int p2)
    {
	if (p1 == FORBIDDEN || p2 == FORBIDDEN)
	    return FORBIDDEN;

	if (p1 == BLANK)
	    return p2;

	if (p2 == BLANK)
	    return p1;

	if (p1 == ALLOWED || p2 == ALLOWED)
	    return ALLOWED;

	return BLANK;
    }

    /** 
     * There are two versions of add(). One adds two Privilege objects. 
     * The other just adds permissions (as returned by getPermission()). 
     */
    public static Privilege add(Privilege p1, Privilege p2)
    {
	int perm = add(p1.getPermission(), p2.getPermission());

	return new Privilege(perm, p1.getDuration());
    }

    /** 
     * Compares the permission of this Privilege to the permission of
     * the argument Privilege. The duration is ignored in the comparison. 
     *
     * @return true if the permissions are the same 
     */
    public boolean samePermission(Privilege p)
    {
	return samePermission(p.getPermission());
    }

    /** 
     * Compares the permission of this object to the permission argument. 
     *
     * @return true if the permissions are the same 
     */
    public boolean samePermission(int perm)
    {
	return (this.permission == perm);
    }

    /** 
     * Compares the duration of this Privilege to the duration of the 
     * argument Privilege. The permission is ignored in the comparison. 
     *
     * @return true if the durations are the same 
     */
    public boolean sameDuration(Privilege p)
    {
	return sameDuration(p.getDuration());
    }

    /**
     * Compares the duration of this Privilege to the duration argument. 
     *
     * @return true if the durations are the same 
     */
    public boolean sameDuration(int duration)
    {
	return (this.duration == duration);
    }

    /**
     * @return true if the permission of this privilege allowed 
     */
    public boolean isAllowed()
    {
	return (permission == ALLOWED);
    }


    /**
     * @return true if the permission of this privilege forbidden 
     */
    public boolean isForbidden()
    {
	return (permission == FORBIDDEN);
    }


    /**
     * @return true if the permission of this privilege blank 
     */
    public boolean isBlank()
    {
	return (permission == BLANK);
    }

    /**
     * @return the permission of this privilege (allowed, forbidden, 
     * or blank) 
     */
    public int getPermission()
    {
	return permission;
    }

    /** 
     * @return the duration of this privilege (scope, session, or forever) 
     */
    public int getDuration()
    {
	return duration;
    }

    public boolean equals(Object priv)
    {
	if (priv instanceof Privilege)
	{
	    Privilege p = (Privilege) priv;

	    return ((permission == p.getPermission()) 
		    && (duration == p.getDuration()));
	}
	else
	{
	    return false;
	}
    }
}
